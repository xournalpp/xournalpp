#include "Document.h"

#include "pdf/base/XojPdfAction.h"

#include "LinkDestination.h"
#include "XojPage.h"

#include <config.h>
#include <i18n.h>
#include <Stacktrace.h>
#include <Util.h>

#include <boost/locale/format.hpp>

#include <string.h>
#include <iostream>
using std::cout;
using std::endl;

Document::Document(DocumentHandler* handler)
{
	XOJ_INIT_TYPE(Document);

	this->handler = handler;
	this->contentsModel = NULL;
	this->preview = NULL;
	this->attachPdf = false;
	this->createBackupOnSave = false;

	g_mutex_init(&this->documentLock);
}

Document::~Document()
{
	XOJ_CHECK_TYPE(Document);

	clearDocument(true);

	if (this->contentsModel)
	{
		g_object_unref(this->contentsModel);
		this->contentsModel = NULL;
	}

	XOJ_RELEASE_TYPE(Document);
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

void Document::setFilename(path filename)
{
	XOJ_CHECK_TYPE(Document);

	this->filename = filename;
}

path Document::getFilename()
{
	XOJ_CHECK_TYPE(Document);

	return filename;
}

path Document::getPdfFilename()
{
	XOJ_CHECK_TYPE(Document);

	return pdfFilename;
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

path Document::getEvMetadataFilename()
{
	XOJ_CHECK_TYPE(Document);

	if (!this->filename.empty())
	{
		return this->filename;
	}
	if (!this->pdfFilename.empty())
	{
		return this->pdfFilename;
	}
	return path("");
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

	for (size_t i = 0; i < getPageCount(); i++)
	{
		PageRef p = this->pages[i];
		if (p->getBackgroundType().isPdfPage())
		{
			if (p->getPdfPageNr() == pdfPage)
			{
				return i;
			}
		}
	}
	return -1;
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

void Document::buildContentsModel()
{
	XOJ_CHECK_TYPE(Document);

	if (this->contentsModel)
	{
		g_object_unref(this->contentsModel);
		this->contentsModel = NULL;
	}

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

bool Document::readPdf(path filename, bool initPages, bool attachToDocument)
{
	XOJ_CHECK_TYPE(Document);

	GError* popplerError = NULL;

	lock();

	if (!pdfDocument.load(filename.c_str(), password.c_str(), &popplerError))
	{
		lastError = FS(_F("Document not loaded! ({1}), {2}") % filename % popplerError->message);
		g_error_free(popplerError);
		return false;
	}

	cout << bl::format("attachToDocument: {1}") % attachToDocument << endl;

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
			XojPdfPage* page = pdfDocument.getPage(i);
			PageRef p = new XojPage(page->getWidth(), page->getHeight());
			p->setBackgroundPdfPageNr(i);
			addPage(p);
		}
	}

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

	size_t id = indexOf(p);
	if (id != size_t_npos && id < getPageCount())
	{
		this->handler->firePageSizeChanged(id);
	}
}

string Document::getLastErrorMsg()
{
	XOJ_CHECK_TYPE(Document);

	return lastError;
}

void Document::deletePage(size_t pNr)
{
	XOJ_CHECK_TYPE(Document);

	std::vector<PageRef>::iterator it;

	it = this->pages.begin() + pNr;
	this->pages.erase(it);

	updateIndexPageNumbers();
}

void Document::insertPage(PageRef p, size_t position)
{
	XOJ_CHECK_TYPE(Document);

	this->pages.insert(this->pages.begin() + position, p);

	updateIndexPageNumbers();
}

void Document::addPage(PageRef p)
{
	XOJ_CHECK_TYPE(Document);

	this->pages.push_back(p);

	updateIndexPageNumbers();
}

size_t Document::indexOf(PageRef page)
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

	return size_t_npos;
}

PageRef Document::getPage(size_t page)
{
	XOJ_CHECK_TYPE(Document);

	if (getPageCount() <= page)
	{
		return NULL;
	}
	if (page == size_t_npos)
	{
		return NULL;
	}

	return this->pages[page];
}

XojPdfPage* Document::getPdfPage(size_t page)
{
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument.getPage(page);
}

XojPdfDocument& Document::getPdfDocument()
{
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument;
}

void Document::operator=(Document& doc)
{
	XOJ_CHECK_TYPE(Document);

	clearDocument();

	// Copy PDF Document
	this->pdfDocument = doc.pdfDocument;

	this->password = doc.password;
	this->createBackupOnSave = doc.createBackupOnSave;
	this->pdfFilename = doc.pdfFilename;
	this->filename = doc.filename;

	for (unsigned int i = 0; i < doc.pages.size(); i++)
	{
		PageRef p = doc.pages[i];
		addPage(p);
	}

	buildContentsModel();
	updateIndexPageNumbers();

	bool lastLock = tryLock();
	unlock();
	this->handler->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	if (!lastLock) // document was locked before
	{
		lock();
	}
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
