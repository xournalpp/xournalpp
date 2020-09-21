#include "Document.h"

#include "pdf/base/XojPdfAction.h"

#include "LinkDestination.h"
#include "XojPage.h"

#include <config.h>
#include <i18n.h>
#include <Stacktrace.h>
#include <Util.h>
#include <util/cpp14memory.h>

Document::Document(DocumentHandler* handler)
 : handler(handler)
{
	XOJ_INIT_TYPE(Document);
	g_mutex_init(&this->documentLock);
}

Document::~Document()
{
	XOJ_CHECK_TYPE(Document);

	clearDocument(true);
	freeTreeContentModel();

	XOJ_RELEASE_TYPE(Document);
}

void Document::freeTreeContentModel()
{
	XOJ_CHECK_TYPE(Document);

	if (this->contentsModel)
	{
		gtk_tree_model_foreach(this->contentsModel, (GtkTreeModelForeachFunc) freeTreeContentEntry, this);

		g_object_unref(this->contentsModel);
		this->contentsModel = NULL;
	}
}

bool Document::freeTreeContentEntry(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc)
{
	XojLinkDest* link = NULL;
	gtk_tree_model_get(treeModel, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

	if (link == NULL)
	{
		return false;
	}

	// The dispose function of XojLinkDest is not called, this workaround fixes the Memory Leak
	delete link->dest;
	link->dest = NULL;

	return false;
}

void Document::lock()
{
	XOJ_CHECK_TYPE(Document);

	g_mutex_lock(&this->documentLock);

	//	if(tryLock()) {
	//		fprintf(stderr, "Locked by\n");
	//		Stacktrace::printStracktrace();
	//		fprintf(stderr, "\n\n\n\n");
	//	} else {
	//		g_mutex_lock(&this->documentLock);
	//	}
}

void Document::unlock()
{
	XOJ_CHECK_TYPE(Document);
	g_mutex_unlock(&this->documentLock);

	//	fprintf(stderr, "Unlocked by\n");
	//	Stacktrace::printStracktrace();
	//	fprintf(stderr, "\n\n\n\n");
}

bool Document::tryLock()
{
	XOJ_CHECK_TYPE(Document);

	return g_mutex_trylock(&this->documentLock);
}

void Document::clearDocument(bool destroy)
{
	XOJ_CHECK_TYPE(Document);

	if (this->preview)
	{
		cairo_surface_destroy(this->preview);
		this->preview = NULL;
	}

	if (!destroy)
	{
		// release lock
		bool lastLock = tryLock();
		unlock();
		this->handler->fireDocumentChanged(DOCUMENT_CHANGE_CLEARED);
		if (!lastLock) // document was locked before
		{
			lock();
		}
	}

	this->pages.clear();
    this->pageIndex.reset();
	freeTreeContentModel();

	this->filename = "";
	this->pdfFilename = "";
}

/**
 * Returns the pageCount, this call don't need to be synchronized (if it's not critical, you may get wrong data)
 */
size_t Document::getPageCount()
{
	XOJ_CHECK_TYPE(Document);

	return this->pages.size();
}

size_t Document::getPdfPageCount()
{
	XOJ_CHECK_TYPE(Document);

	return pdfDocument.getPageCount();
}

void Document::setFilename(Path filename)
{
	XOJ_CHECK_TYPE(Document);

	this->filename = filename;
}

Path Document::getFilename()
{
	XOJ_CHECK_TYPE(Document);

	return filename;
}

Path Document::getPdfFilename()
{
	XOJ_CHECK_TYPE(Document);

	return pdfFilename;
}

Path Document::createSaveFolder(Path lastSavePath)
{
	if (!filename.isEmpty())
	{
		return filename.getParentPath();
	}
	else if (!pdfFilename.isEmpty())
	{
		return pdfFilename.getParentPath();
	}
	else
	{
		return lastSavePath;
	}
}

Path Document::createSaveFilename(DocumentType type, string defaultSaveName)
{
	if (!filename.isEmpty())
	{
		// This can be any extension
		Path p = filename.getFilename();
		p.clearExtensions();
		return p;
	}
	else if (!pdfFilename.isEmpty())
	{
		Path p = pdfFilename.getFilename();
        std::string ext = this->attachPdf ? ".pdf" : "";
        p.clearExtensions(ext);
		return p;
	}
	else
	{
		time_t curtime = time(NULL);
		char stime[128];
		strftime(stime, sizeof(stime), defaultSaveName.c_str(), localtime(&curtime));

		// Remove the extension, file format is handled by the filter combo box
		Path p = stime;
		p.clearExtensions();
		return p;
	}
}


cairo_surface_t* Document::getPreview()
{
	XOJ_CHECK_TYPE(Document);

	return this->preview;
}

void Document::setPreview(cairo_surface_t* preview)
{
	XOJ_CHECK_TYPE(Document);

	if (this->preview)
	{
		cairo_surface_destroy(this->preview);
	}
	if (preview)
	{
		this->preview = cairo_surface_reference(preview);
	}
	else
	{
		this->preview = NULL;
	}
}

Path Document::getEvMetadataFilename()
{
	XOJ_CHECK_TYPE(Document);

	if (!this->filename.isEmpty())
	{
		return this->filename;
	}
	if (!this->pdfFilename.isEmpty())
	{
		return this->pdfFilename;
	}
	return Path("");
}

bool Document::isPdfDocumentLoaded()
{
	XOJ_CHECK_TYPE(Document);

	return pdfDocument.isLoaded();
}

bool Document::isAttachPdf()
{
	XOJ_CHECK_TYPE(Document);

	return this->attachPdf;
}

size_t Document::findPdfPage(size_t pdfPage)
{
	XOJ_CHECK_TYPE(Document);

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

void Document::buildTreeContentsModel(GtkTreeIter* parent, XojPdfBookmarkIterator* iter)
{
	XOJ_CHECK_TYPE(Document);

	do
	{
		GtkTreeIter treeIter = { 0 };

		XojPdfAction* action = iter->getAction();
		XojLinkDest* link = action->getDestination();

		if (action->getTitle().empty())
		{
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
		if (child)
		{
			buildTreeContentsModel(&treeIter, child);
			delete child;
		}

		delete action;

	}
	while (iter->next());
}

void Document::indexPdfPages() {
    auto index = mem::make_unique<PageIndex>();
    for (size_t i = 0; i < this->pages.size(); ++i) {
        const auto& p = this->pages[i];
        if (p->getBackgroundType().isPdfPage()) {
            index->emplace(p->getPdfPageNr(), i);
        }
    }
    this->pageIndex.swap(index);
}


void Document::buildContentsModel()
{
	XOJ_CHECK_TYPE(Document);

	freeTreeContentModel();

	XojPdfBookmarkIterator* iter = pdfDocument.getContentsIter();
	if (iter == NULL)
	{
		// No Bookmarks
		return;
	}

	this->contentsModel = (GtkTreeModel*) gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_BOOLEAN, G_TYPE_STRING);
	buildTreeContentsModel(NULL, iter);
	delete iter;
}

GtkTreeModel* Document::getContentsModel()
{
	XOJ_CHECK_TYPE(Document);

	return this->contentsModel;
}

bool Document::fillPageLabels(GtkTreeModel* treeModel, GtkTreePath* path, GtkTreeIter* iter, Document* doc)
{
	XojLinkDest* link = NULL;
	gtk_tree_model_get(treeModel, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

	if (link == NULL)
	{
		return false;
	}

	int page = doc->findPdfPage(link->dest->getPdfPage());

	gchar* pageLabel = NULL;
	if (page != -1)
	{
		pageLabel = g_strdup_printf("%i", page + 1);
	}
	gtk_tree_store_set(GTK_TREE_STORE(treeModel), iter, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, pageLabel, -1);
	g_free(pageLabel);

	g_object_unref(link);
	return false;
}

void Document::updateIndexPageNumbers()
{
	XOJ_CHECK_TYPE(Document);

	if (this->contentsModel != NULL)
	{
		gtk_tree_model_foreach(this->contentsModel, (GtkTreeModelForeachFunc) fillPageLabels, this);
	}
}

bool Document::readPdf(Path filename, bool initPages, bool attachToDocument, gpointer data, gsize length)
{
	XOJ_CHECK_TYPE(Document);

	GError* popplerError = NULL;

	lock();

	if (data != nullptr)
	{
		if (!pdfDocument.load(data, length, password, &popplerError))
		{
			lastError = FS(_F("Document not loaded! ({1}), {2}") % filename.str() % popplerError->message);
			g_error_free(popplerError);
			unlock();

			return false;
		}
	} else
	{
		if (!pdfDocument.load(filename.c_str(), password, &popplerError))
		{
			lastError = FS(_F("Document not loaded! ({1}), {2}") % filename.str() % popplerError->message);
			g_error_free(popplerError);
			unlock();

			return false;
		}
	}


	this->pdfFilename = filename;
	this->attachPdf = attachToDocument;

	lastError = "";

	if (initPages)
	{
		this->pages.clear();
	}

	if (initPages)
	{
		for (size_t i = 0; i < pdfDocument.getPageCount(); i++)
		{
			XojPdfPageSPtr page = pdfDocument.getPage(i);
			PageRef p = new XojPage(page->getWidth(), page->getHeight());
			p->setBackgroundPdfPageNr(i);
            this->pages.emplace_back(p);
        }
	}

    indexPdfPages();
	buildContentsModel();
	updateIndexPageNumbers();

	unlock();

	this->handler->fireDocumentChanged(DOCUMENT_CHANGE_PDF_BOOKMARKS);

	return true;
}

void Document::setPageSize(PageRef p, double width, double height)
{
	XOJ_CHECK_TYPE(Document);

	p->setSize(width, height);
}

double Document::getPageWidth(PageRef p)
{
	return p->getWidth();
}

double Document::getPageHeight(PageRef p)
{
	return p->getHeight();
}

/**
 * @return The last error message to show to the user
 */
string Document::getLastErrorMsg()
{
	XOJ_CHECK_TYPE(Document);

	return lastError;
}

void Document::deletePage(size_t pNr)
{
	XOJ_CHECK_TYPE(Document);

	vector<PageRef>::iterator it = this->pages.begin() + pNr;
	this->pages.erase(it);

    this->pageIndex.reset();
	updateIndexPageNumbers();
}

void Document::insertPage(const PageRef& p, size_t position)
{
	XOJ_CHECK_TYPE(Document);

	this->pages.insert(this->pages.begin() + position, p);

    this->pageIndex.reset();
    updateIndexPageNumbers();
}

void Document::addPage(const PageRef& p)
{
	XOJ_CHECK_TYPE(Document);

	this->pages.push_back(p);

    this->pageIndex.reset();
    updateIndexPageNumbers();
}

size_t Document::indexOf(const PageRef& page)
{
	XOJ_CHECK_TYPE(Document);

	for (size_t i = 0; i < this->pages.size(); i++)
	{
		PageRef pg = this->pages[i];
		if (pg == page)
		{
			return i;
		}
	}

	return npos;
}

PageRef Document::getPage(size_t page)
{
	XOJ_CHECK_TYPE(Document);

	if (getPageCount() <= page)
	{
		return NULL;
	}
	if (page == npos)
	{
		return NULL;
	}

	return this->pages[page];
}

XojPdfPageSPtr Document::getPdfPage(size_t page)
{
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument.getPage(page);
}

XojPdfDocument& Document::getPdfDocument()
{
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument;
}

Document& Document::operator=(const Document& doc)
{
	XOJ_CHECK_TYPE(Document);

	clearDocument();

	// Copy PDF Document
	this->pdfDocument = doc.pdfDocument;

	this->password = doc.password;
	this->createBackupOnSave = doc.createBackupOnSave;
	this->pdfFilename = doc.pdfFilename;
	this->filename = doc.filename;
    this->pages = doc.pages;

    indexPdfPages();
	buildContentsModel();
	updateIndexPageNumbers();

	bool lastLock = tryLock();
	unlock();
	this->handler->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	if (!lastLock) // document was locked before
	{
		lock();
	}
	return *this;
}

void Document::setCreateBackupOnSave(bool backup)
{
	XOJ_CHECK_TYPE(Document);

	this->createBackupOnSave = backup;
}

bool Document::shouldCreateBackupOnSave()
{
	XOJ_CHECK_TYPE(Document);

	return this->createBackupOnSave;
}
