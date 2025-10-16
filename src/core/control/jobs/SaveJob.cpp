#include "SaveJob.h"
#include "util/OutputStream.h"

#include <memory>  // for __shared_ptr_access

#include <cairo.h>  // for cairo_create, cairo_destroy
#include <glib.h>   // for g_warning, g_error
#include <random>
#include <fstream>

#include "control/Control.h"              // for Control
#include "control/jobs/BlockingJob.h"     // for BlockingJob
#include "control/xojfile/SaveHandler.h"  // for SaveHandler
#include "model/Document.h"               // for Document
#include "model/PageRef.h"                // for PageRef
#include "model/PageType.h"               // for PageType
#include "model/XojPage.h"                // for XojPage
#include "pdf/base/XojPdfPage.h"          // for XojPdfPageSPtr, XojPdfPage
#include "util/PathUtil.h"                // for clearExtensions, safeRename...
#include "util/XojMsgBox.h"               // for XojMsgBox
#include "util/i18n.h"                    // for FS, _, _F
#include "view/DocumentView.h"            // for DocumentView
#include <pugixml.hpp> // Da aggiungere anche per Unix
#include "util/OutputStream.h"           // for GzInputStream
#include "filesystem.h"  // for path, filesystem_error, remove
#include "util/StringUtils.h"

SaveJob::SaveJob(Control* control, std::function<void(bool)> callback):
        BlockingJob(control, _("Save")), callback(std::move(callback)) {}

SaveJob::~SaveJob() = default;

void SaveJob::run() {
    save();

    if (this->control->getWindow()) {
        callAfterRun();
    }
}

void SaveJob::afterRun() {
    if (!this->lastError.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->lastError);
        callback(false);
    } else {
        this->control->resetSavedStatus();
        callback(true);
    }
}

void SaveJob::updatePreview(Control* control) {
    const int previewSize = 128;

    Document* doc = control->getDocument();

    doc->lock();

    if (doc->getPageCount() > 0) {
        PageRef page = doc->getPage(0);

        double width = page->getWidth();
        double height = page->getHeight();

        double zoom = 1;

        if (width < height) {
            zoom = previewSize / height;
        } else {
            zoom = previewSize / width;
        }
        width *= zoom;
        height *= zoom;

        cairo_surface_t* crBuffer =
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil_cast<int>(width), ceil_cast<int>(height));

        cairo_t* cr = cairo_create(crBuffer);
        cairo_scale(cr, zoom, zoom);

        xoj::view::BackgroundFlags flags = xoj::view::BACKGROUND_SHOW_ALL;

        // We don't have access to a PdfCache on which DocumentView relies for PDF backgrounds.
        // We thus print the PDF background by hand.
        if (page->getBackgroundType().isPdfPage()) {
            auto pgNo = page->getPdfPageNr();
            XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
            if (popplerPage) {
                popplerPage->render(cr);
            }
            flags.showPDF = xoj::view::HIDE_PDF_BACKGROUND;  // Already printed (if any)
        } else {
            flags.forceBackgroundColor = xoj::view::FORCE_AT_LEAST_BACKGROUND_COLOR;
        }

        DocumentView view;
        view.drawPage(page, cr, true /* don't render erasable */, flags);
        cairo_destroy(cr);
        doc->setPreview(crBuffer);
        cairo_surface_destroy(crBuffer);
    } else {
        doc->setPreview(nullptr);
    }

    doc->unlock();
}
// Nel tuo file che contiene extractXmlFromXopp
#include <iostream> // Per il logging degli errori

std::string extractXmlFromXopp(const fs::path& filepath, fs::path tempDir) {
    GzInputStream in(filepath);

    // **Controllo 1: Errore durante l'apertura del file**
    // GzInputStream imposta un errore nel costruttore se gzopen fallisce.
    if (!in.getLastError().empty()) {
        std::cerr << "Error opening compressed file " << filepath.string() 
                  << ": " << in.getLastError() << std::endl;
        return ""; // Restituisce stringa vuota in caso di errore
    }

    fs::create_directories(tempDir);
    std::string xml = in.readAll();

    // **Controllo 2: Errore durante la lettura del file**
    // Controlla se readAll() ha impostato un errore.
    if (!in.getLastError().empty()) {
        std::cerr << "Error reading compressed file " << filepath.string() 
                  << ": " << in.getLastError() << std::endl;
        return ""; // Restituisce stringa vuota in caso di errore
    }
    
    if (xml.empty()) {
        std::cerr << "Warning: Extracted XML from " << filepath.string() 
                  << " is empty. The file might be valid but contain no data." << std::endl;
    }

    return xml;
}

fs::path createTempDir() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 9999);
    int code = dist(gen);

    fs::path tempDir = fs::temp_directory_path() / ("Xournal-" + std::to_string(code));
    
    return tempDir;
}

void saveFinalFile(const std::string& modifiedXMLStr, const std::string& originalXMLStr,
                   const fs::path& target, auto& lastError)
{
    pugi::xml_document modifiedDoc, originalDoc;

    if (!modifiedDoc.load_string(modifiedXMLStr.c_str())) {
        lastError = FS(_F("Error saving file, could not parse modified XML"));
        return;
    }

    if (!originalDoc.load_string(originalXMLStr.c_str())) {
        // Se il file originale non esiste o è corrotto, salviamo semplicemente il file con le modifiche.
        // Questo gestisce il caso di "Salva con nome" o di un file nuovo.
        GzOutputStream out(target);
        out.write(modifiedXMLStr.c_str(), modifiedXMLStr.length());
        out.close();

        g_warning("Pare ci sia un problema qui");

        return;
    }

    // Usa il documento originale come base per il risultato.
    pugi::xml_document resultDoc;
    resultDoc.append_copy(originalDoc.document_element());
    pugi::xml_node resultRoot = resultDoc.child("xournal");

    // 1. Crea una mappa di UID -> xml_node per tutte le pagine modificate.
    
    std::map<std::string, pugi::xml_node> modifiedPagesMap;
    for (pugi::xml_node modifiedPage : modifiedDoc.child("xournal").children("page")) {
        const char* uid = modifiedPage.attribute("uid").as_string();
        if (uid && *uid) { // Assicurati che l'UID non sia nullo o vuoto
            modifiedPagesMap[uid] = modifiedPage;
        }
    }

    pugi::xml_node currentPage = resultRoot.child("page");
    while (currentPage) {
        // Salva il nodo successivo PRIMA di un'eventuale rimozione
        pugi::xml_node nextNode = currentPage.next_sibling("page");

        const char* uid = currentPage.attribute("uid").as_string();
        if (uid && *uid) {
            auto it = modifiedPagesMap.find(uid);
            if (it != modifiedPagesMap.end()) {
                // Trovata una versione modificata di questa pagina, sostituiscila.
                resultRoot.insert_copy_before(it->second, currentPage);
                resultRoot.remove_child(currentPage); // Ora è sicuro rimuovere il nodo corrente

                // Rimuovi dalla mappa per contrassegnarla come elaborata.
                modifiedPagesMap.erase(it);
            }
        }
        // Passa al nodo successivo
        currentPage = nextNode;
    }

    // 3. Aggiungi alla fine le pagine rimanenti dalla mappa (pagine nuove o senza un UID corrispondente).
    for (const auto& pair : modifiedPagesMap) {
        resultRoot.append_copy(pair.second);
    }
    

    // 4. Salva il documento unito.
    std::stringstream ss;
    resultDoc.save(ss, "  "); // Usa un'indentazione per la leggibilità
    std::string mergedXml = ss.str();

    GzOutputStream out(target);
    out.write(mergedXml.c_str(), mergedXml.length());
    out.close();
}

auto SaveJob::save() -> bool {
    updatePreview(control);
    Document* doc = this->control->getDocument();
    SaveHandler h;

    long totalTime = 0;

    doc->lock();
    fs::path target = doc->getFilepath();

    g_warning("Saving to: %s", target.u8string().c_str());

    Util::safeReplaceExtension(target, "xopp");

    h.prepareSave(doc, target);
    doc->unlock();
    
    auto const createBackup = doc->shouldCreateBackupOnSave();

    if ( !StringUtils::isXoppLegacy )
    {
        fs::path tempDir = createTempDir();

        std::string originalXMLStr = extractXmlFromXopp(target, tempDir);
        
        if (createBackup) {
            try {
                // Note: The backup must be created for the target as this is the filepath
                // which will be written to. Do not use the `filepath` variable!
                Util::safeRenameFile(target, fs::path{target} += "~");
            } catch (const fs::filesystem_error& fe) {
                g_warning("Could not create backup! Failed with %s", fe.what());
                this->lastError = FS(_F("Save file error, can't backup: {1}") % std::string(fe.what()));
                if (!control->getWindow()) {
                    g_error("%s", this->lastError.c_str());
                }
                return false;
            }
        }

        //std::ofstream outFile(tempDir / "original.xml");
        //outFile << originalXMLStr;

        fs::path xoppFileModifiedOnlyPages = tempDir / "backup.xopp";

        /*
            Save modified pages only in xopp format
        */

        doc->lock();
        h.saveTo(xoppFileModifiedOnlyPages, this->control);
        doc->setFilepath(target);
        doc->unlock();

        /*
            Extract XML from xoppFileModifiedOnlyPages
        */

        std::string modifiedXMLStr = extractXmlFromXopp(xoppFileModifiedOnlyPages, tempDir);

        saveFinalFile(modifiedXMLStr, originalXMLStr, target, this->lastError);

    }
    else
    {
        
        if (createBackup) {
            try {
                // Note: The backup must be created for the target as this is the filepath
                // which will be written to. Do not use the `filepath` variable!
                Util::safeRenameFile(target, fs::path{target} += "~");
            } catch (const fs::filesystem_error& fe) {
                g_warning("Could not create backup! Failed with %s", fe.what());
                this->lastError = FS(_F("Save file error, can't backup: {1}") % std::string(fe.what()));
                if (!control->getWindow()) {
                    g_error("%s", this->lastError.c_str());
                }
                return false;
            }
        }

        doc->lock();
        h.saveTo(target, this->control);
        doc->setFilepath(target);
        doc->unlock();
    }

    
    if (!h.getErrorMessage().empty()) {
        this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());
        if (!control->getWindow()) {
            g_error("%s", this->lastError.c_str());
        }
        return false;
    } else if (createBackup) {
        try {
            // If a backup was created it can be removed now since no error occured during the save
            fs::remove(fs::path{target} += "~");
        } catch (const fs::filesystem_error& fe) {
            g_warning("Could not delete backup! Failed with %s", fe.what());
        }
    } else {
        doc->setCreateBackupOnSave(true);
    }

    return true;
}
