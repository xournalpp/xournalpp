/*
 * Xournal++
 *
 * The document
 *
 * All methods are unlocked, you need to lock the document before you change something and unlock after.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>        // for size_t
#include <memory>         // for unique_ptr
#include <mutex>          // for mutex
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

#include <cairo.h>    // for cairo_surface_t
#include <glib.h>     // for gpointer, gsize
#include <gtk/gtk.h>  // for GtkTreeModel, GtkTreeIter, GtkT...

#include "pdf/base/XojPdfDocument.h"  // for XojPdfDocument
#include "pdf/base/XojPdfPage.h"      // for XojPdfPageSPtr
#include "util/raii/GObjectSPtr.h"    // for GObjectSptr

#include "PageRef.h"     // for PageRef
#include "filesystem.h"  // for path

class DocumentHandler;
class XojPdfBookmarkIterator;

class Document {
public:
    Document(DocumentHandler* handler);
    virtual ~Document();

public:
    enum DocumentType { XOPP, XOJ, PDF };

    bool readPdf(const fs::path& filename, bool initPages, bool attachToDocument, gpointer data = nullptr,
                 gsize length = 0);

    size_t getPageCount() const;
    size_t getPdfPageCount() const;
    XojPdfPageSPtr getPdfPage(size_t page) const;
    const XojPdfDocument& getPdfDocument() const;

    void insertPage(const PageRef& p, size_t position);
    void addPage(const PageRef& p);
    template <class InputIter>
    void addPages(InputIter first, InputIter last);
    PageRef getPage(size_t page) const;
    void deletePage(size_t pNr);

    static void setPageSize(PageRef p, double width, double height);
    static double getPageWidth(PageRef p);
    static double getPageHeight(PageRef p);

    size_t indexOf(const PageRef& page);

    /**
     * @return The last error message to show to the user
     */
    std::string getLastErrorMsg() const;

    size_t findPdfPage(size_t pdfPage) const;

    Document& operator=(const Document& doc);

    void setFilepath(fs::path filepath);
    fs::path getFilepath() const;
    fs::path getPdfFilepath() const;
    fs::path createSaveFoldername(const fs::path& lastSavePath) const;
    fs::path createSaveFilename(DocumentType type, const std::string& defaultSaveName,
                                const std::string& defaultPfdName = "") const;

    fs::path getEvMetadataFilename() const;

    GtkTreeModel* getContentsModel() const;

    void setCreateBackupOnSave(bool backup);
    bool shouldCreateBackupOnSave() const;

    void clearDocument(bool destroy = false);

    bool isAttachPdf() const;

    cairo_surface_t* getPreview() const;
    void setPreview(cairo_surface_t* preview);

    void lock();
    void unlock();
    bool tryLock();

private:
    void buildContentsModel();
    void freeTreeContentModel();
    static bool freeTreeContentEntry(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc);

    void buildTreeContentsModel(GtkTreeIter* parent, XojPdfBookmarkIterator* iter);
    void updateIndexPageNumbers();
    static bool fillPageLabels(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc);

private:
    DocumentHandler* handler = nullptr;

    XojPdfDocument pdfDocument;

    fs::path filepath;
    fs::path pdfFilepath;
    bool attachPdf = false;

    /**
     *  Password: not handled yet
     */
    std::string password;

    std::string lastError;

    /**
     * The pages in the document
     */
    std::vector<PageRef> pages;

    /**
     * Index from pdf page number to document page number
     */
    using PageIndex = std::unordered_map<size_t, size_t>;

    /**
     * The cached page index
     */
    std::unique_ptr<PageIndex> pageIndex;

    /**
     * Creates an index from pdf page number to document page number
     *
     * Clears the index first in case it is already exists.
     */
    void indexPdfPages();

    /**
     * The bookmark contents model
     */
    xoj::util::GObjectSPtr<GtkTreeModel> contentsModel;

    /**
     *  create a backup before save
     */
    bool createBackupOnSave = false;

    /**
     * The preview for the file
     */
    cairo_surface_t* preview = nullptr;

    /**
     * The lock of the document
     */
    std::mutex documentLock;
};

template <class InputIter>
void Document::addPages(InputIter first, InputIter last) {
    this->pages.insert(this->pages.end(), first, last);
    this->pageIndex.reset();
    updateIndexPageNumbers();
}
