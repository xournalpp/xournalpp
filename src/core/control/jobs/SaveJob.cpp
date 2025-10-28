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
#include "util/StringUtils.h"
#include <unordered_map> 
#include <string_view>   

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
        //lastError = FS(_F("Cannot parse XML from file"));
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

struct GzXmlWriter : public pugi::xml_writer {
    GzOutputStream& stream;

    GzXmlWriter(GzOutputStream& s) : stream(s) {}

    virtual void write(const void* data, size_t size) {
        stream.write(static_cast<const char*>(data), size);
    }
};

/**
 * Helper function to build a map of XML page nodes keyed by their UID.
 * This allows for O(1) average-case lookup of pages.
 * @param xournalRoot The <xournal> root node.
 * @return A map where key is UID (string_view) and value is the page node.
 */
std::unordered_map<std::string_view, pugi::xml_node> buildPageMap(pugi::xml_node xournalRoot) {
    std::unordered_map<std::string_view, pugi::xml_node> pageMap;
    if (!xournalRoot) {
        return pageMap;
    }
    for (pugi::xml_node page : xournalRoot.children("page")) {
        pageMap[page.attribute("uid").value()] = page;
    }
    return pageMap;
}

/**
 * Helper function to copy metadata (root attributes, title, preview)
 * from a source XML root node to a destination root node.
 * @param sourceRoot The <xournal> node to copy from.
 * @param destRoot The <xournal> node to copy to.
 */
void copyMetadata(pugi::xml_node sourceRoot, pugi::xml_node destRoot) {
    if (!sourceRoot || !destRoot) {
        return;
    }

    // Copy all attributes from <xournal> tag
    for (pugi::xml_attribute attr : sourceRoot.attributes()) {
        destRoot.append_attribute(attr.name()) = attr.value();
    }

    // Copy <title> and <preview> nodes if they exist
    pugi::xml_node titleNode = sourceRoot.child("title");
    if (titleNode) {
        destRoot.append_copy(titleNode);
    }
    pugi::xml_node previewNode = sourceRoot.child("preview");
    if (previewNode) {
        destRoot.append_copy(previewNode);
    }
}

/**
 * Merges the modified XML (containing only changed pages) with the original XML
 * to create the final, complete .xopp file.
 *
 * This function ensures that the final page order matches the current order in the Document object.
 * It iterates through the Document's pages, picking the "modified" version of a page if it exists,
 * otherwise falling back to the "original" version.
 *
 * @param control The main application control object.
 * @param modifiedXMLStr A string containing the XML for *only* the modified pages.
 * @param originalXMLStr A string containing the XML for the *original* file.
 * @param target The final file path to save to.
 * @param lastError A reference to store any error message.
 */
void SaveJob::saveFinalFile(Control* control, const std::string& modifiedXMLStr, const std::string& originalXMLStr, const fs::path& target, auto& lastError) {
    pugi::xml_document modifiedDoc, originalDoc;

    // 1. Parse the modified XML string
    if (!modifiedDoc.load_buffer(modifiedXMLStr.data(), modifiedXMLStr.length())) {
        lastError = FS(_F("Error saving file, could not parse modified XML"));
        return;
    }

    // 2. Try to parse the original XML string
    bool originalIsValid = originalDoc.load_buffer(originalXMLStr.data(), originalXMLStr.length());

    // 3. If the original XML is invalid (e.g., new file), just save the modified XML directly.
    if (!originalIsValid) {
        GzOutputStream out(target);
        out.write(modifiedXMLStr.c_str(), modifiedXMLStr.length());
        out.close();
        return;
    }

    // 4. Both XMLs are valid; proceed with the merge.
    pugi::xml_document resultDoc;
    pugi::xml_node sourceRoot = modifiedDoc.child("xournal");
    pugi::xml_node resultRoot = resultDoc.append_child("xournal");

    // 5. Copy metadata (attributes, title, preview) from the modified doc
    copyMetadata(sourceRoot, resultRoot);

    // 6. Get the definitive page order from the main Document object
    Document* doc = control->getDocument();
    std::vector<PageRef> pages = doc->getPages();

    // 7. Set up progress reporting
    ProgressListener* listener = control;
    auto pagesCount = pages.size();
    listener->setMaximumState(pagesCount);
    g_warning("%lu pages to process", static_cast<unsigned long>(pagesCount));

    // 8. Build lookup maps for fast page access (O(1) average)
    auto modifiedPageMap = buildPageMap(modifiedDoc.child("xournal"));
    auto originalPageMap = buildPageMap(originalDoc.child("xournal"));

    // 9. Main merge loop: Iterate through pages in the *correct* order
    int progressCounter = 1;
    for (const auto& pageRef : pages) {
        std::string_view uid = pageRef.get()->getUID();

        auto it_mod = modifiedPageMap.find(uid);
        if (it_mod != modifiedPageMap.end()) {
            // Page was modified: use the version from modifiedXMLStr
            resultRoot.append_copy(it_mod->second);
        } else {
            auto it_orig = originalPageMap.find(uid);
            if (it_orig != originalPageMap.end()) {
                // Page was *not* modified: use the version from originalXMLStr
                resultRoot.append_copy(it_orig->second);
            } else {
                // This should ideally not happen if the Document is in sync
                g_warning("Could not find page with UID %s in either modified or original document.", std::string(uid).c_str());
            }
        }

        listener->setCurrentState(progressCounter++);
    }

    // 10. Save the final merged XML document
    GzOutputStream out(target);
    GzXmlWriter writer(out);
    
    // Using 3 spaces for indentation
    resultDoc.save(writer, "   "); 
    out.close();
}

/**
 * Attempts to create a backup of the target file by renaming it with a "~" suffix.
 * @param target The file to back up.
 * @return true on success, false on failure (and sets lastError).
 */
auto SaveJob::createBackup(const fs::path& target) -> bool {
    try {
        Util::safeRenameFile(target, fs::path{target} += "~");
        return true;
    } catch (const fs::filesystem_error& fe) {
        g_warning("Could not create backup! Failed with %s", fe.what());
        this->lastError = FS(_F("Save file error, can't backup: {1}") % std::string(fe.what()));
        if (!control->getWindow()) {
            g_error("%s", this->lastError.c_str());
        }
        return false;
    }
}

/**
 * Extracts the original XML, detects if it's a "legacy" format, and stores the XML.
 * @param target The path to the original .xopp file.
 * @param tempDir A temporary directory for extraction.
 * @param originalXMLStr [out] The extracted XML content.
 * @return true on success, false on failure (and sets lastError).
 */
auto SaveJob::detectLegacyFormat(const fs::path& target, const fs::path& tempDir, std::string& originalXMLStr) -> bool {
    originalXMLStr = extractXmlFromXopp(target, tempDir, this->lastError);

    pugi::xml_document xmlDoc;
    if (!xmlDoc.load_string(originalXMLStr.c_str())) {
        // If the file doesn't exist or is empty, originalXMLStr might be empty.
        // This is OK (e.g., new file), but we can't parse it.
        // We'll treat it as non-legacy by default.
        if (originalXMLStr.empty()) {
            return true;
        }

        this->lastError = FS(_F("Could not parse original XML file"));
        return false;
    }

    return true;
}

/**
 * Handles saving the document in the "modern" format.
 * This involves saving only modified pages to a temporary file,
 * then merging them with the original file's pages using saveFinalFile.
 * @param h The SaveHandler prepared for saving.
 * @param target The final destination path.
 * @param originalXMLStr The XML content of the original file.
 * @param tempDir The temporary directory for intermediate files.
 * @param totalTime [in/out] Reference to the total time counter for metrics.
 */
auto SaveJob::saveModern(SaveHandler& h, const fs::path& target, const std::string& originalXMLStr, const fs::path& tempDir, long& totalTime) -> void {
    auto start_time = std::chrono::steady_clock::now();
    fs::path xoppFileModifiedOnlyPages = tempDir / "backup.xopp";

    // 1. Save *only* modified pages to a temporary .xopp file
    Document* doc = this->control->getDocument();
    doc->lock();
    h.saveTo(xoppFileModifiedOnlyPages, this->control);
    doc->unlock();

    auto end_time = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    totalTime += millis;
    g_warning("Saving modified pages took: %ld ms", millis);

    start_time = std::chrono::steady_clock::now();

    // 2. Extract the XML from the temporary file
    std::string modifiedXMLStr = extractXmlFromXopp(xoppFileModifiedOnlyPages, tempDir, this->lastError);

    // 3. Merge the modified XML and original XML into the final target file
    this->saveFinalFile(this->control, modifiedXMLStr, originalXMLStr, target, this->lastError);

    end_time = std::chrono::steady_clock::now();
    millis = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    totalTime += millis;
    g_warning("saveFinalFile (merge) duration: %ld ms", millis);

    // 4. Clean up the temporary directory
    uintmax_t removeTempDirectory = fs::remove_all(tempDir);
    g_warning("Removed temporary directory %s with %llu files.", tempDir.u8string().c_str(),
              static_cast<unsigned long long>(removeTempDirectory));
}

/**
 * Handles saving the document in the "legacy" format (saving the entire document at once).
 * @param h The SaveHandler prepared for saving.
 * @param target The final destination path.
 * @param totalTime [in/out] Reference to the total time counter for metrics.
 */
auto SaveJob::saveLegacy(SaveHandler& h, const fs::path& target, long& totalTime) -> void {
    auto start_time = std::chrono::steady_clock::now();

    Document* doc = this->control->getDocument();
    doc->lock();
    h.saveTo(target, this->control);
    doc->unlock();

    auto end_time = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    totalTime += millis;
    g_warning("saveTo (legacy) duration: %ld ms", millis);
}

/**
 * Main function to save the document.
 * Orchestrates preview updates, backup creation, legacy detection,
 * and the appropriate save strategy (legacy vs. modern).
 * @return true if the save was successful, false otherwise.
 */
auto SaveJob::save() -> bool {
    long totalTime = 0;
    auto start_time = std::chrono::steady_clock::now();

    // 1. Initial setup: update preview, get doc, prepare save handler
    updatePreview(control);
    Document* doc = this->control->getDocument();
    SaveHandler h;

    doc->lock();
    fs::path target = doc->getFilepath();
    Util::safeReplaceExtension(target, "xopp");
    h.prepareSave(doc, target);
    doc->unlock();

    auto const createBackup = doc->shouldCreateBackupOnSave();
    fs::path tempDir = createTempDir();

    // 2. Detect format (legacy or modern) and get original XML
    std::string originalXMLStr;
    if (!this->detectLegacyFormat(target, tempDir, originalXMLStr)) {
        // Error already set in detectLegacyFormat
        fs::remove_all(tempDir); // Clean up temp dir on failure
        return false;
    }

    auto end_time = std::chrono::steady_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    totalTime += millis;
    g_warning("prepareToSave and legacy detection take: %ld ms", millis);

    // 3. Create a backup (e.g., rename main.xopp to main.xopp~)
    if (createBackup) {
        if (!this->createBackup(target)) {
            fs::remove_all(tempDir); // Clean up temp dir on failure
            return false; // Error already set in createBackup
        }
    }
    
    if ( doc->getFileVersion() < 5 ) {
        this->saveLegacy(h, target, totalTime);
    } else {
        this->saveModern(h, target, originalXMLStr, tempDir, totalTime);
    }

    // 5. Finalize: update doc path and report total time
    doc->setFilepath(target);
    g_warning("Save took: %ld ms", totalTime);

    // 6. Final error checking and backup cleanup
    if (!h.getErrorMessage().empty()) {
        this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());
        if (!control->getWindow()) {
            g_error("%s", this->lastError.c_str());
        }
        // Note: We DON'T remove the backup file if saving failed, allowing user recovery
        return false;

    } else if (createBackup) {
        // Save was successful, remove the backup file.
        try {
            fs::remove(fs::path{target} += "~");
        } catch (const fs::filesystem_error& fe) {
            g_warning("Could not delete backup! Failed with %s", fe.what());
        }
    } else {
        // Backup was not created this time (e.g., first save),
        // so ensure it's enabled for *next* time.
        doc->setCreateBackupOnSave(true);
    }

    return true;
}