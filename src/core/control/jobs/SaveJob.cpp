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
#include <pugixml.hpp>                    // for xml_document, xml_node, ...             
#include "util/OutputStream.h"            // for GzInputStream
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

std::string extractXmlFromXopp(const fs::path& filepath, fs::path tempDir, auto& lastError) {
    GzInputStream in(filepath);

    if (!in.getLastError().empty()) {
        lastError = FS(_F("Error opening compressed file"));
        return "";
    }

    fs::create_directories(tempDir);
    std::string xml = in.readAll();

    if (xml.empty()) {
        lastError = FS(_F("Cannot parse XML from file"));
        return "";
    }

    return xml;
}

fs::path createTempDir() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 999999);

    while (true) {
        int code = dist(gen);
        fs::path tempDir = fs::temp_directory_path() / ("xournal-" + std::to_string(code));
        
        std::error_code ec;

        if (fs::create_directory(tempDir, ec)) {
            return tempDir;
        }

        if (ec && ec != std::errc::file_exists) {
            throw fs::filesystem_error("Impossibile creare la directory temporanea", tempDir, ec);
        }

    }
}

void saveFinalFile(Control* control, const std::string& modifiedXMLStr, const std::string& originalXMLStr, const fs::path& target, auto& lastError) {
    pugi::xml_document modifiedDoc, originalDoc;

    if (!modifiedDoc.load_string(modifiedXMLStr.c_str())) {
        lastError = FS(_F("Error saving file, could not parse modified XML"));
        return;
    }

    bool originalIsValid = originalDoc.load_string(originalXMLStr.c_str());

    if (!originalIsValid) {
        GzOutputStream out(target);
        out.write(modifiedXMLStr.c_str(), modifiedXMLStr.length());
        out.close();
        return;
    }

    pugi::xml_document resultDoc;
    pugi::xml_node sourceRoot = modifiedDoc.child("xournal");
    pugi::xml_node resultRoot = resultDoc.append_child("xournal");

    for (pugi::xml_attribute attr : sourceRoot.attributes()) {
        resultRoot.append_attribute(attr.name()) = attr.value();
    }

    pugi::xml_node titleNode = sourceRoot.child("title");

    if (titleNode) {
        resultRoot.append_copy(titleNode);
    }
    pugi::xml_node previewNode = sourceRoot.child("preview");

    if (previewNode) {
        resultRoot.append_copy(previewNode);
    }

    ProgressListener* listener = control;

    int progressCounter = 1;

    auto pagesCount = control->getDocument()->getPages().size();

    listener->setMaximumState(  pagesCount );

    Document* doc = control->getDocument();
    std::vector<PageRef> pages = doc->getPages();

    pugi::xml_node modifiedXournalNode = modifiedDoc.child("xournal");
    pugi::xml_node originalXournalNode = originalDoc.child("xournal");

    for (const auto& pageRef : pages) {
        const char* uid = pageRef.get()->getUID().c_str();

        pugi::xml_node pageNode = modifiedXournalNode.find_child_by_attribute("page", "uid", uid);

        if (pageNode) {

            resultRoot.append_copy(pageNode);

        } else {
            
            pugi::xml_node originalPageNode = originalXournalNode.find_child_by_attribute("page", "uid", uid);

            if (originalPageNode) {
                resultRoot.append_copy(originalPageNode);
            } else {
                g_warning("Could not find page with UID %s in either modified or original document.", uid);
            }

        }

        listener->setCurrentState(progressCounter++);
    }

    std::stringstream ss;
    resultDoc.save(ss, "  ");
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

    Util::safeReplaceExtension(target, "xopp");

    h.prepareSave(doc, target);
    doc->unlock();
    
    auto const createBackup = doc->shouldCreateBackupOnSave();

    fs::path tempDir = createTempDir();

    std::string originalXMLStr = extractXmlFromXopp(target, tempDir, this->lastError);
        
    pugi::xml_document xmlDoc;
    if (!xmlDoc.load_string(originalXMLStr.c_str())) {
        this->lastError = FS(_F("Could not parse original XML file"));
        return false;
    }

    pugi::xml_node sourceRoot = xmlDoc.child("xournal");

    if (sourceRoot) {
        pugi::xml_attribute legacyAttr = sourceRoot.attribute("isLegacy");

        if ( !legacyAttr )
        {
            StringUtils::isLegacy = true;
        }
        else
        {
            StringUtils::isLegacy = false;
        }

    } else {
        StringUtils::isLegacy = false;
    }

    if ( !StringUtils::isLegacy )
    {
        if (createBackup) {
            try {
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

        fs::path xoppFileModifiedOnlyPages = tempDir / "backup.xopp";

        doc->lock();
        h.saveTo(xoppFileModifiedOnlyPages, this->control);
        doc->setFilepath(target);
        doc->unlock();

        std::string modifiedXMLStr = extractXmlFromXopp(xoppFileModifiedOnlyPages, tempDir, this->lastError);

        saveFinalFile(this->control, modifiedXMLStr, originalXMLStr, target, this->lastError);

        uintmax_t removeTempDirectory = fs::remove_all(tempDir);
        g_warning("Removed temporary directory %s with %llu files.", tempDir.u8string().c_str(),
                  static_cast<unsigned long long>(removeTempDirectory));
    
    }
    else
    {
        
        if (createBackup) {
            try {
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

    std::chrono::duration<double> elapsed = end - start;
    g_warning("Merging XML and saving took %.3f seconds", elapsed.count());

    return true;
}
