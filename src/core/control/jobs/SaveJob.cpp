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
#include "control/xml/pugixml.hpp" // Da aggiungere anche per Unix
#include "util/OutputStream.h"           // for GzInputStream
#include "filesystem.h"  // for path, filesystem_error, remove


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

std::string extractXmlFromXopp(const fs::path& filepath, fs::path tempDir) {
    
    GzInputStream in(filepath);

    fs::create_directories(tempDir);
    std::string xml = in.readAll();

    // Scrivi l'XML in un file dentro la cartella
    //fs::path xmlFile = tempDir / fileName;
    //std::ofstream out(xmlFile);
    //out << xml;
    //out.close();

    return xml;
}

bool compressXmlToXopp(const fs::path& xmlFile, const fs::path& xoppFile) {
    // 1. Leggi il contenuto del file XML con buffer più grande
    std::ifstream xmlInput(xmlFile, std::ios::binary);
    if (!xmlInput.is_open()) {
        g_error("Cannot open XML file: %s", xmlFile.string().c_str());
        return false;
    }
    
    // Ottimizzazione: usa un buffer più grande per ridurre le operazioni I/O
    constexpr size_t BUFFER_SIZE = 65536; // 64KB
    xmlInput.rdbuf()->pubsetbuf(nullptr, BUFFER_SIZE);
    
    // Ottimizzazione: pre-alloca la memoria se conosci la dimensione
    xmlInput.seekg(0, std::ios::end);
    size_t fileSize = xmlInput.tellg();
    xmlInput.seekg(0, std::ios::beg);
    
    std::string xmlContent;
    xmlContent.reserve(fileSize); // Pre-alloca per evitare riallocazioni
    
    xmlContent.assign(std::istreambuf_iterator<char>(xmlInput),
                      std::istreambuf_iterator<char>());
    xmlInput.close();
    
    // 2. Crea il file .xopp compresso
    GzOutputStream xoppOutput(xoppFile);
    
    if (!xoppOutput.getLastError().empty()) {
        g_error("Error creating xopp file: %s", xoppOutput.getLastError().c_str());
        return false;
    }
    
    // 3. Scrivi tutto in una volta (invece di scritture multiple)
    xoppOutput.write(xmlContent.c_str(), xmlContent.length());
    
    // 4. Chiudi il file
    xoppOutput.close();
    
    if (!xoppOutput.getLastError().empty()) {
        g_error("Error writing xopp file: %s", xoppOutput.getLastError().c_str());
        return false;
    }
    
    g_message("Successfully created xopp file: %s", xoppFile.string().c_str());
    return true;
}

auto SaveJob::save() -> bool {
    updatePreview(control);
    Document* doc = this->control->getDocument();
    SaveHandler h;

    long totalTime = 0;

    auto startTotal = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();

    doc->lock();
    fs::path target = doc->getFilepath();
    Util::safeReplaceExtension(target, "xopp");

    h.prepareSave(doc, target);
    doc->unlock();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    totalTime += duration.count();

    g_message("Prepare to save %lld ms", duration.count());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 9999);
    int code = dist(gen);

    start = std::chrono::high_resolution_clock::now();

    fs::path tempDir = fs::temp_directory_path() / ("Xournal-" + std::to_string(code));

    std::string originalDocument = extractXmlFromXopp(target, tempDir);

    auto const createBackup = doc->shouldCreateBackupOnSave();

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

    fs::path xoppModifiedPages = tempDir / "backup.xopp";

    /*
        Save modified pages only in xopp format
    */

    doc->lock();
    h.saveTo(xoppModifiedPages, this->control);
    doc->setFilepath(xoppModifiedPages);
    doc->unlock();

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    totalTime += duration.count();
    g_message("Salvare solo le pagine modificate %lld ms", duration.count());

    start = std::chrono::high_resolution_clock::now();

    /*
        Extract XML from xoppModifiedPages
    */

    std::string newDocument = extractXmlFromXopp(xoppModifiedPages, tempDir);

    pugi::xml_document doc1, doc2;

    if (!doc1.load_string(newDocument.c_str())) {
        std::cerr << "Errore caricamento documento1.xml\n";
        return -1;
    }
    if (!doc2.load_string(originalDocument.c_str())) {
        std::cerr << "Errore caricamento documento2.xml\n";
        return -1;
    }
    
    pugi::xml_document resultDoc;
    pugi::xml_node root = resultDoc.append_copy(doc2.document_element());

    int pageIndex = 0;
    for (pugi::xml_node page : root.children("page")) {
        if (std::find(UndoRedoHandler::pagesChanged.begin(), UndoRedoHandler::pagesChanged.end(),  pageIndex) != UndoRedoHandler::pagesChanged.end()) {
            pugi::xpath_node pageDoc1 = doc1.select_node(("/xournal/page[" + std::to_string(pageIndex + 1) + "]").c_str());
            if (pageDoc1) {
                root.insert_copy_before(pageDoc1.node(), page);
                root.remove_child(page);
            }
        }
        pageIndex++;
    }

    std::stringstream ss;
    resultDoc.save(ss, "  ");
    std::string mergedXml = ss.str();
    
    GzOutputStream out(target);
    out.write(mergedXml.c_str(), mergedXml.length());
    out.close();

    //fs::path mergedXml = tempDir / "merged.xml";

    // Salva il documento RISULTANTE (resultDoc, non doc2!)
    //resultDoc.save_file(mergedXml.c_str(), "");

    // Comprimi il merged XML nel target finale
    /*if (!compressXmlToXopp(mergedXml, target)) {
        this->lastError = "Failed to compress merged XML to xopp";
        return false;
    }*/

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    totalTime += duration.count();
    g_message("Salvare il file ultimo %lld ms", duration.count());

    g_message("Total save time %lld ms", totalTime);
    
    //compressXmlToXopp(a, target);

    /*
    auto pagesToWrite = UndoRedoHandler::pagesChanged;
   
    for (int idx : pagesToWrite) {
        std::string xpathExpr = "/xournal/page[" + std::to_string(idx + 1) + "]";

        pugi::xpath_node pageDoc1 = doc1.select_node(xpathExpr.c_str()); // original
        pugi::xpath_node pageDoc2 = doc2.select_node(xpathExpr.c_str()); // modified

        if (pageDoc1 && pageDoc2) {
            pugi::xml_node oldPage = pageDoc2.node();
            pugi::xml_node parent = oldPage.parent();

            // Inserisco copia della pagina da doc1 al posto di quella di doc2
            parent.insert_copy_before(pageDoc1.node(), oldPage);
            parent.remove_child(oldPage);
        } else {
            std::cerr << "Pagina " << idx << " non trovata\n";
        }
    }

    fs::path a = target.parent_path() / "documento.xml";

    // Salvo il risultato
    doc2.save_file(a.c_str(), "  ");

    compressXmlToXopp(a, target);
    */

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
