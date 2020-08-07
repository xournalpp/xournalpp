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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pdf/base/XojPdfBookmarkIterator.h"
#include "pdf/base/XojPdfDocument.h"
#include "pdf/base/XojPdfPage.h"

#include "DocumentHandler.h"
#include "LinkDestination.h"
#include "PageRef.h"
#include "XournalType.h"
#include "filesystem.h"

class Document {
public:
    Document(DocumentHandler* handler);
    virtual ~Document();

public:
    enum DocumentType { XOPP, XOJ, PDF };

    bool readPdf(const fs::path& filename, bool initPages, bool attachToDocument, gpointer data = nullptr,
                 gsize length = 0);

    size_t getPageCount();
    size_t getPdfPageCount();
    XojPdfPageSPtr getPdfPage(size_t page);
    XojPdfDocument& getPdfDocument();

    void insertPage(const PageRef& p, size_t position);
    void addPage(const PageRef& p);
    template <class InputIter>
    void addPages(InputIter first, InputIter last);
    PageRef getPage(size_t page);
    void deletePage(size_t pNr);

    static void setPageSize(PageRef p, double width, double height);
    static double getPageWidth(PageRef p);
    static double getPageHeight(PageRef p);

    size_t indexOf(const PageRef& page);

    /**
     * @return The last error message to show to the user
     */
    string getLastErrorMsg();

    bool isPdfDocumentLoaded();
    size_t findPdfPage(size_t pdfPage);

    Document& operator=(const Document& doc);

    void setFilepath(fs::path filepath);
    fs::path getFilepath();
    fs::path getPdfFilepath();
    fs::path createSaveFolder(fs::path lastSavePath);
    fs::path createSaveFilename(DocumentType type, const string& defaultSaveName);

    fs::path getEvMetadataFilename();

    GtkTreeModel* getContentsModel();

    void setCreateBackupOnSave(bool backup);
    bool shouldCreateBackupOnSave() const;

    void clearDocument(bool destroy = false);

    bool isAttachPdf() const;

    cairo_surface_t* getPreview();
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
    string password;

    string lastError;

    /**
     * The pages in the document
     */
    vector<PageRef> pages;

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
    GtkTreeModel* contentsModel = nullptr;

    /**
     *  create a backup before save, because the original file was an older fileversion
     */
    bool createBackupOnSave = false;

    /**
     * The preview for the file
     */
    cairo_surface_t* preview = nullptr;

    /**
     * The lock of the document
     */
    GMutex documentLock{};
};

template <class InputIter>
void Document::addPages(InputIter first, InputIter last) {
    this->pages.insert(this->pages.end(), first, last);
    this->pageIndex.reset();
    updateIndexPageNumbers();
}
