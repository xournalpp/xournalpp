#include "Document.h"

#include <algorithm>
#include <utility>

#include <config.h>

#include "pdf/base/XojPdfAction.h"

#include "LinkDestination.h"
#include "PathUtil.h"
#include "Stacktrace.h"
#include "Util.h"
#include "XojPage.h"
#include "filesystem.h"
#include "i18n.h"

Document::Document(DocumentHandler* handler): handler(handler) { g_mutex_init(&this->documentLock); }

Document::~Document() {
    clearDocument(true);
    freeTreeContentModel();
}

void Document::freeTreeContentModel() {
    if (this->contentsModel) {
        gtk_tree_model_foreach(this->contentsModel, reinterpret_cast<GtkTreeModelForeachFunc>(freeTreeContentEntry),
                               this);

        g_object_unref(this->contentsModel);
        this->contentsModel = nullptr;
    }
}

auto Document::freeTreeContentEntry(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc)
        -> bool {
    XojLinkDest* link = nullptr;
    gtk_tree_model_get(treeModel, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

    if (link == nullptr) {
        return false;
    }

    // The dispose function of XojLinkDest is not called, this workaround fixes the Memory Leak
    delete link->dest;
    link->dest = nullptr;

    return false;
}

void Document::lock() {
    g_mutex_lock(&this->documentLock);

    //	if(tryLock()) {
    //		fprintf(stderr, "Locked by\n");
    //		Stacktrace::printStracktrace();
    //		fprintf(stderr, "\n\n\n\n");
    //	} else {
    //		g_mutex_lock(&this->documentLock);
    //	}
}

void Document::unlock() {
    g_mutex_unlock(&this->documentLock);

    //	fprintf(stderr, "Unlocked by\n");
    //	Stacktrace::printStracktrace();
    //	fprintf(stderr, "\n\n\n\n");
}

auto Document::tryLock() -> bool { return g_mutex_trylock(&this->documentLock); }

void Document::clearDocument(bool destroy) {
    if (this->preview) {
        cairo_surface_destroy(this->preview);
        this->preview = nullptr;
    }

    if (!destroy) {
        // release lock
        bool lastLock = tryLock();
        unlock();
        this->handler->fireDocumentChanged(DOCUMENT_CHANGE_CLEARED);
        if (!lastLock)  // document was locked before
        {
            lock();
        }
    }

    this->pages.clear();
    this->pageIndex.reset();
    freeTreeContentModel();

    this->filepath = fs::path{};
    this->pdfFilepath = fs::path{};
}

/**
 * Returns the pageCount, this call don't need to be synchronized (if it's not critical, you may get wrong data)
 */
auto Document::getPageCount() -> size_t { return this->pages.size(); }

auto Document::getPdfPageCount() -> size_t { return pdfDocument.getPageCount(); }

void Document::setFilepath(fs::path filepath) { this->filepath = std::move(filepath); }

auto Document::getFilepath() -> fs::path { return filepath; }

auto Document::getPdfFilepath() -> fs::path { return pdfFilepath; }

auto Document::createSaveFolder(fs::path lastSavePath) -> fs::path {
    if (!filepath.empty()) {
        return filepath.parent_path();
    }
    if (!pdfFilepath.empty()) {
        return pdfFilepath.parent_path();
    }


    return lastSavePath;
}

auto Document::createSaveFilename(DocumentType type, const string& defaultSaveName) -> fs::path {
    if (!filepath.empty()) {
        // This can be any extension
        fs::path p = filepath.filename();
        Util::clearExtensions(p);
        return p;
    }
    if (!pdfFilepath.empty()) {
        fs::path p = pdfFilepath.filename();
        std::string ext = this->attachPdf ? ".pdf" : "";
        Util::clearExtensions(p, ext);
        return p;
    }


    time_t curtime = time(nullptr);
    char stime[128];
    strftime(stime, sizeof(stime), defaultSaveName.c_str(), localtime(&curtime));

    // Remove the extension, file format is handled by the filter combo box
    fs::path p = stime;
    Util::clearExtensions(p);
    return p;
}


auto Document::getPreview() -> cairo_surface_t* { return this->preview; }

void Document::setPreview(cairo_surface_t* preview) {
    if (this->preview) {
        cairo_surface_destroy(this->preview);
    }
    if (preview) {
        this->preview = cairo_surface_reference(preview);
    } else {
        this->preview = nullptr;
    }
}

auto Document::getEvMetadataFilename() -> fs::path {
    if (!this->filepath.empty()) {
        return this->filepath;
    }
    if (!this->pdfFilepath.empty()) {
        return this->pdfFilepath;
    }
    return fs::path{};
}

auto Document::isPdfDocumentLoaded() -> bool { return pdfDocument.isLoaded(); }

auto Document::isAttachPdf() const -> bool { return this->attachPdf; }

auto Document::findPdfPage(size_t pdfPage) -> size_t {
    // Create a page index if not already indexed.
    if (!this->pageIndex)
        indexPdfPages();
    auto pos = this->pageIndex->find(pdfPage);
    if (pos == this->pageIndex->end()) {
        return -1;
    } else {
        return pos->second;
    }
}

void Document::buildTreeContentsModel(GtkTreeIter* parent, XojPdfBookmarkIterator* iter) {
    do {
        GtkTreeIter treeIter = {0};

        XojPdfAction* action = iter->getAction();
        XojLinkDest* link = action->getDestination();

        if (action->getTitle().empty()) {
            g_object_unref(link);
            delete action;
            continue;
        }

        link->dest->setExpand(iter->isOpen());

        gtk_tree_store_append(GTK_TREE_STORE(contentsModel), &treeIter, parent);
        char* titleMarkup = g_markup_escape_text(action->getTitle().c_str(), -1);

        gtk_tree_store_set(GTK_TREE_STORE(contentsModel), &treeIter, DOCUMENT_LINKS_COLUMN_NAME, titleMarkup,
                           DOCUMENT_LINKS_COLUMN_LINK, link, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, "", -1);

        g_free(titleMarkup);
        g_object_unref(link);

        XojPdfBookmarkIterator* child = iter->getChildIter();
        if (child) {
            buildTreeContentsModel(&treeIter, child);
            delete child;
        }

        delete action;

    } while (iter->next());
}

void Document::indexPdfPages() {
    auto index = std::make_unique<PageIndex>();
    for (size_t i = 0; i < this->pages.size(); ++i) {
        const auto& p = this->pages[i];
        if (p->getBackgroundType().isPdfPage()) {
            index->emplace(p->getPdfPageNr(), i);
        }
    }
    this->pageIndex.swap(index);
}


void Document::buildContentsModel() {
    freeTreeContentModel();

    XojPdfBookmarkIterator* iter = pdfDocument.getContentsIter();
    if (iter == nullptr) {
        // No Bookmarks
        return;
    }

    this->contentsModel = reinterpret_cast<GtkTreeModel*>(
            gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_BOOLEAN, G_TYPE_STRING));
    buildTreeContentsModel(nullptr, iter);
    delete iter;
}

auto Document::getContentsModel() -> GtkTreeModel* { return this->contentsModel; }

auto Document::fillPageLabels(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc) -> bool {
    XojLinkDest* link = nullptr;
    gtk_tree_model_get(treeModel, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

    if (link == nullptr) {
        return false;
    }

    int page = doc->findPdfPage(link->dest->getPdfPage());

    gchar* pageLabel = nullptr;
    if (page != -1) {
        pageLabel = g_strdup_printf("%i", page + 1);
    }
    gtk_tree_store_set(GTK_TREE_STORE(treeModel), iter, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, pageLabel, -1);
    g_free(pageLabel);

    g_object_unref(link);
    return false;
}

void Document::updateIndexPageNumbers() {
    if (this->contentsModel != nullptr) {
        gtk_tree_model_foreach(this->contentsModel, reinterpret_cast<GtkTreeModelForeachFunc>(fillPageLabels), this);
    }
}

auto Document::readPdf(const fs::path& filename, bool initPages, bool attachToDocument, gpointer data, gsize length)
        -> bool {
    GError* popplerError = nullptr;

    lock();

    if (data != nullptr) {
        if (!pdfDocument.load(data, length, password, &popplerError)) {
            lastError = FS(_F("Document not loaded! ({1}), {2}") % filename.string() % popplerError->message);
            g_error_free(popplerError);
            unlock();

            return false;
        }
    } else {
        if (!pdfDocument.load(filename, password, &popplerError)) {
            if (popplerError) {
                lastError = FS(_F("Document not loaded! ({1}), {2}") % filename.string() % popplerError->message);
                g_error_free(popplerError);
            } else {
                lastError = FS(_F("Document not loaded! ({1}), {2}") % filename.string() % "");
            }
            unlock();
            return false;
        }
    }

    this->pdfFilepath = filename;
    this->attachPdf = attachToDocument;
    lastError = "";

    if (initPages) {
        this->pages.clear();
    }

    if (initPages) {
        for (size_t i = 0; i < pdfDocument.getPageCount(); i++) {
            XojPdfPageSPtr page = pdfDocument.getPage(i);
            auto p = std::make_shared<XojPage>(page->getWidth(), page->getHeight());
            p->setBackgroundPdfPageNr(i);
            this->pages.emplace_back(std::move(p));
        }
    }

    indexPdfPages();
    buildContentsModel();
    updateIndexPageNumbers();

    unlock();

    this->handler->fireDocumentChanged(DOCUMENT_CHANGE_PDF_BOOKMARKS);

    return true;
}

void Document::setPageSize(PageRef p, double width, double height) { p->setSize(width, height); }

auto Document::getPageWidth(PageRef p) -> double { return p->getWidth(); }

auto Document::getPageHeight(PageRef p) -> double { return p->getHeight(); }

/**
 * @return The last error message to show to the user
 */
auto Document::getLastErrorMsg() -> string { return lastError; }

void Document::deletePage(size_t pNr) {
    auto it = this->pages.begin() + pNr;
    this->pages.erase(it);

    // Reset the page index
    this->pageIndex.reset();
    updateIndexPageNumbers();
}

void Document::insertPage(const PageRef& p, size_t position) {
    this->pages.insert(this->pages.begin() + position, p);

    // Reset the page index
    this->pageIndex.reset();
    updateIndexPageNumbers();
}

void Document::addPage(const PageRef& p) {
    this->pages.push_back(p);

    // Reset the page index
    this->pageIndex.reset();
    updateIndexPageNumbers();
}

auto Document::indexOf(const PageRef& page) -> size_t {
    for (size_t i = 0; i < this->pages.size(); i++) {
        PageRef pg = this->pages[i];
        if (pg == page) {
            return i;
        }
    }

    return npos;
}

auto Document::getPage(size_t page) -> PageRef {
    if (getPageCount() <= page) {
        return nullptr;
    }
    if (page == npos) {
        return nullptr;
    }

    return this->pages[page];
}

auto Document::getPdfPage(size_t page) -> XojPdfPageSPtr { return this->pdfDocument.getPage(page); }

auto Document::getPdfDocument() -> XojPdfDocument& { return this->pdfDocument; }

auto Document::operator=(const Document& doc) -> Document& {
    clearDocument();

    // Copy PDF Document
    this->pdfDocument = doc.pdfDocument;

    this->password = doc.password;
    this->createBackupOnSave = doc.createBackupOnSave;
    this->pdfFilepath = doc.pdfFilepath;
    this->filepath = doc.filepath;
    this->pages = doc.pages;

    indexPdfPages();
    buildContentsModel();
    updateIndexPageNumbers();

    bool lastLock = tryLock();
    unlock();
    this->handler->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
    if (!lastLock)  // document was locked before
    {
        lock();
    }
    return *this;
}

void Document::setCreateBackupOnSave(bool backup) { this->createBackupOnSave = backup; }

auto Document::shouldCreateBackupOnSave() const -> bool { return this->createBackupOnSave; }
