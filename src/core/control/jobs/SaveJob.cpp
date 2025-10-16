#include "SaveJob.h"
#include "util/OutputStream.h"

#include <memory>  // for __shared_ptr_access

#include <cairo.h>  // for cairo_create, cairo_destroy
#include <glib.h>   // for g_warning, g_error
#include <random>
#include <fstream>
#include <vector>
#include <chrono>

#include "control/Control.h"              // for Control
#include "control/jobs/BlockingJob.h"     // for BlockingJob
#include "control/jobs/ProgressListener.h"  // for ProgressListener
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

void saveFinalFile(Control* control, const std::string& modifiedXMLStr, const std::string& originalXMLStr, const fs::path& target, auto& lastError) {
    pugi::xml_document modifiedDoc, originalDoc;

    // Carica il documento modificato e originale

    if (!modifiedDoc.load_string(modifiedXMLStr.c_str())) {
        lastError = FS(_F("Error saving file, could not parse modified XML"));
        return;
    }

    bool originalIsValid = originalDoc.load_string(originalXMLStr.c_str());

    if (!originalIsValid) {
        // Se il file originale non esiste o è corrotto, salviamo semplicemente il file con le modifiche.
        // Questo gestisce il caso di "Salva con nome" o di un file nuovo.
        GzOutputStream out(target);
        out.write(modifiedXMLStr.c_str(), modifiedXMLStr.length());
        out.close();
        return;
    }

    // 1. Crea la base del documento finale con l'header corretto.
    pugi::xml_document resultDoc;
    pugi::xml_node sourceRoot = modifiedDoc.child("xournal");
    pugi::xml_node resultRoot = resultDoc.append_child("xournal");

    // Copia tutti gli attributi (creator, fileversion, etc.) dal sorgente
    for (pugi::xml_attribute attr : sourceRoot.attributes()) {
        resultRoot.append_attribute(attr.name()) = attr.value();
    }

    // Aggiungi solo i tag <title> e <preview> all'header
    pugi::xml_node titleNode = sourceRoot.child("title");
    if (titleNode) {
        resultRoot.append_copy(titleNode);
    }
    pugi::xml_node previewNode = sourceRoot.child("preview");
    if (previewNode) {
        resultRoot.append_copy(previewNode);
    }

    // 2. Ottieni l'ordine corretto delle pagine dal documento in memoria

    ProgressListener* listener = control;

    int progressCounter = 1;

    auto pagesCount = control->getDocument()->getPages().size();

    listener->setMaximumState(  pagesCount );

    g_warning("Total pages to process: %zu", pagesCount);

    Document* doc = control->getDocument();
    std::vector<PageRef> pages = doc->getPages();

    pugi::xml_node modifiedXournalNode = modifiedDoc.child("xournal");
    pugi::xml_node originalXournalNode = originalDoc.child("xournal");

    // 3. Itera sull'ordine corretto e aggiungi le pagine al documento finale
    for (const auto& pageRef : pages) {
        const char* uid = pageRef.get()->getUID().c_str();

        // Cerca la pagina prima nel documento delle modifiche
        pugi::xml_node pageNode = modifiedXournalNode.find_child_by_attribute("page", "uid", uid);

        if (pageNode) {
            // **CORREZIONE**: Aggiungi al nodo <xournal> (resultRoot), non al documento
            resultRoot.append_copy(pageNode);
        } else {
            // Se non la troviamo, cerchiamo quella originale (pagina spostata ma non modificata)
            pugi::xml_node originalPageNode = originalXournalNode.find_child_by_attribute("page", "uid", uid);
            if (originalPageNode) {
                // **CORREZIONE**: Aggiungi al nodo <xournal> (resultRoot), non al documento
                resultRoot.append_copy(originalPageNode);
            } else {
                g_warning("Could not find page with UID %s in either modified or original document.", uid);
            }
        }

        listener->setCurrentState(progressCounter++);
    }

    // 4. Salva il documento finale unito
    std::stringstream ss;
    resultDoc.save(ss, "  "); // Usa un'indentazione per la leggibilità
    std::string mergedXml = ss.str();

    GzOutputStream out(target);
    out.write(mergedXml.c_str(), mergedXml.length());
    out.close();

}

auto SaveJob::save() -> bool {

    auto start = std::chrono::steady_clock::now();

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

    fs::path tempDir = createTempDir();

    std::string originalXMLStr = extractXmlFromXopp(target, tempDir);
        
    // Carica il documento originale
    pugi::xml_document xmlDoc;
    if (!xmlDoc.load_string(originalXMLStr.c_str())) {
        g_warning("XML originale non valido, impossibile controllare il flag 'isLegacy'.");
        // Continua comunque, la funzione saveFinalFile gestirà l'XML vuoto
    }

    // Ottieni il nodo radice dal documento che hai appena caricato
    pugi::xml_node sourceRoot = xmlDoc.child("xournal");

    if (sourceRoot) {
        // 1. Chiedi l'attributo direttamente per nome
        pugi::xml_attribute legacyAttr = sourceRoot.attribute("isLegacy");

        // L'attributo non esiste
        if ( !legacyAttr )
        {
            StringUtils::isLegacy = true;
        }
        else
        {
            StringUtils::isLegacy = false;
        }

    } else {
        // Il file potrebbe essere nuovo o corrotto, quindi non è legacy
        StringUtils::isLegacy = false;
    }

    g_warning("isLegacy: %s", StringUtils::isLegacy ? "true" : "false");

    // Se le pagine non hanno uid quindi isLegacy non esiste come attributo
    // Allora dobbiamo ricorrere al vecchio tipo di salvataggio, sennò il nuovo
    if ( !StringUtils::isLegacy )
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

        saveFinalFile(this->control, modifiedXMLStr, originalXMLStr, target, this->lastError);

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

    auto end = std::chrono::steady_clock::now(); // tempo finale

    // differenza in secondi (double)
    std::chrono::duration<double> elapsed = end - start;
    g_warning("Merging XML and saving took %.3f seconds", elapsed.count());

    return true;
}
