#include "Document.h"
#include "LinkDestination.h"
#include "XojPage.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include <string.h>
#include <Stacktrace.h>

Document::Document(DocumentHandler * handler) {
	XOJ_INIT_TYPE(Document);

	this->handler = handler;
	this->contentsModel = NULL;
	this->preview = NULL;
	this->attachPdf = false;
	this->createBackupOnSave = false;

	this->documentLock = g_mutex_new();
}

Document::~Document() {
	XOJ_CHECK_TYPE(Document);

	clearDocument(true);

	if (this->contentsModel) {
		g_object_unref(this->contentsModel);
		this->contentsModel = NULL;
	}

	g_mutex_free(this->documentLock);
	this->documentLock = NULL;

	XOJ_RELEASE_TYPE(Document);
}

void Document::lock() {
	XOJ_CHECK_TYPE(Document);

	g_mutex_lock(this->documentLock);

	//	if(tryLock()) {
	//		fprintf(stderr, "Locked by\n");
	//		Stacktrace::printStracktrace();
	//		fprintf(stderr, "\n\n\n\n");
	//	} else {
	//		g_mutex_lock(this->documentLock);
	//	}
}

void Document::unlock() {
	XOJ_CHECK_TYPE(Document);
	g_mutex_unlock(this->documentLock);

	//	fprintf(stderr, "Unlocked by\n");
	//	Stacktrace::printStracktrace();
	//	fprintf(stderr, "\n\n\n\n");
}

bool Document::tryLock() {
	XOJ_CHECK_TYPE(Document);

	return g_mutex_trylock(this->documentLock);
}

void Document::clearDocument(bool destroy) {
	XOJ_CHECK_TYPE(Document);

	if (this->preview) {
		cairo_surface_destroy(this->preview);
		this->preview = NULL;
	}

	if (!destroy) {
		// look aufheben
		bool lastLock = tryLock();
		unlock();
		this->handler->fireDocumentChanged(DOCUMENT_CHANGE_CLEARED);
		if (!lastLock) { // document was locked before
			lock();
		}
	}

	this->pages.clear();

	this->filename = NULL;
	this->pdfFilename = NULL;
}

/**
 * Returns the pageCount, this call don't need to be synchronized (if it's not critical, you may get wrong data)
 */
int Document::getPageCount() {
	XOJ_CHECK_TYPE(Document);

	return this->pages.size();
}

int Document::getPdfPageCount() {
	XOJ_CHECK_TYPE(Document);

	return pdfDocument.getPageCount();
}

void Document::setFilename(String filename) {
	XOJ_CHECK_TYPE(Document);

	this->filename = filename;
}

String Document::getFilename() {
	XOJ_CHECK_TYPE(Document);

	return filename;
}

String Document::getPdfFilename() {
	XOJ_CHECK_TYPE(Document);

	return pdfFilename;
}

cairo_surface_t * Document::getPreview() {
	XOJ_CHECK_TYPE(Document);

	return this->preview;
}

void Document::setPreview(cairo_surface_t * preview) {
	XOJ_CHECK_TYPE(Document);

	if (this->preview) {
		cairo_surface_destroy(this->preview);
	}
	if (preview) {
		this->preview = cairo_surface_reference(preview);
	} else {
		this->preview = NULL;
	}

}

String Document::getEvMetadataFilename() {
	XOJ_CHECK_TYPE(Document);

	String uri = "file://";
	if (!this->filename.isEmpty()) {
		uri += this->filename;
		return uri;
	}
	if (!this->pdfFilename.isEmpty()) {
		uri += this->pdfFilename;
		return uri;
	}
	return NULL;
}

bool Document::isPdfDocumentLoaded() {
	XOJ_CHECK_TYPE(Document);

	return pdfDocument.isLoaded();
}

bool Document::isAttachPdf() {
	XOJ_CHECK_TYPE(Document);

	return this->attachPdf;
}

int Document::findPdfPage(int pdfPage) {
	XOJ_CHECK_TYPE(Document);

	int count = getPageCount();
	for (int i = 0; i < count; i++) {
		PageRef p = this->pages[i];
		if (p.getBackgroundType() == BACKGROUND_TYPE_PDF) {
			if (p.getPdfPageNr() == pdfPage) {
				return i;
			}
		}
	}
	return -1;
}

void Document::buildTreeContentsModel(GtkTreeIter * parent, XojPopplerIter * iter) {
	XOJ_CHECK_TYPE(Document);

	do {
		GtkTreeIter treeIter = { 0 };

		XojPopplerAction * action = iter->getAction();
		XojLinkDest * link = action->getDestination();

		if (action->getTitle().isEmpty()) {
			g_object_unref(link);
			delete action;
			continue;
		}

		link->dest->setExpand(iter->isOpen());

		gtk_tree_store_append(GTK_TREE_STORE(contentsModel), &treeIter, parent);
		char *titleMarkup = g_markup_escape_text(action->getTitle().c_str(), -1);

		gtk_tree_store_set(GTK_TREE_STORE(contentsModel), &treeIter, DOCUMENT_LINKS_COLUMN_NAME, titleMarkup, DOCUMENT_LINKS_COLUMN_LINK, link,
				DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, "", -1);

		g_free(titleMarkup);
		g_object_unref(link);

		XojPopplerIter * child = iter->getChildIter();
		if (child) {
			buildTreeContentsModel(&treeIter, child);
			delete child;
		}

		delete action;

	} while (iter->next());
}

void Document::buildContentsModel() {
	XOJ_CHECK_TYPE(Document);

	if (this->contentsModel) {
		g_object_unref(this->contentsModel);
		this->contentsModel = NULL;
	}

	XojPopplerIter * iter = pdfDocument.getContentsIter();
	if (iter == NULL) {
		// No Bookmarks
		return;
	}

	this->contentsModel = (GtkTreeModel *) gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_BOOLEAN, G_TYPE_STRING);
	buildTreeContentsModel(NULL, iter);
	delete iter;
}

GtkTreeModel * Document::getContentsModel() {
	XOJ_CHECK_TYPE(Document);

	return this->contentsModel;
}

bool Document::fillPageLabels(GtkTreeModel * treeModel, GtkTreePath * path, GtkTreeIter * iter, Document * doc) {
	XojLinkDest * link = NULL;

	gtk_tree_model_get(treeModel, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

	if (link == NULL) {
		return false;
	}

	int page = doc->findPdfPage(link->dest->getPdfPage());

	gchar * pageLabel = NULL;
	if (page != -1) {
		pageLabel = g_strdup_printf("%i", page + 1);
	}
	gtk_tree_store_set(GTK_TREE_STORE(treeModel), iter, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, pageLabel, -1);
	g_free(pageLabel);

	g_object_unref(link);
	return false;
}

void Document::updateIndexPageNumbers() {
	XOJ_CHECK_TYPE(Document);

	if (this->contentsModel != NULL) {
		gtk_tree_model_foreach(this->contentsModel, (GtkTreeModelForeachFunc) fillPageLabels, this);
	}
}

bool Document::readPdf(String filename, bool initPages, bool attachToDocument) {
	XOJ_CHECK_TYPE(Document);

	GError * popplerError = NULL;

	lock();

	if (!pdfDocument.load(filename.c_str(), password.c_str(), &popplerError)) {
		char * txt = g_strdup_printf("Document not loaded! (%s), %s", filename.c_str(), popplerError->message);
		lastError = txt;
		g_free(txt);
		g_error_free(popplerError);
		return false;
	}

	printf("attachToDocument: %i\n", attachToDocument);

	this->pdfFilename = filename;
	this->attachPdf = attachToDocument;

	lastError = NULL;

	if (initPages) {
		this->pages.clear();
	}

	if (initPages) {
		for (int i = 0; i < pdfDocument.getPageCount(); i++) {
			XojPopplerPage * page = pdfDocument.getPage(i);
			PageRef p = new XojPage(page->getWidth(), page->getHeight());
			p.setBackgroundPdfPageNr(i);
			addPage(p);
		}
	}

	buildContentsModel();
	updateIndexPageNumbers();

	unlock();

	this->handler->fireDocumentChanged(DOCUMENT_CHANGE_PDF_BOOKMARKS);

	return true;
}

void Document::setPageSize(PageRef p, double width, double height) {
	XOJ_CHECK_TYPE(Document);

	p.setSize(width, height);

	int id = indexOf(p);
	if (id >= 0 && id < getPageCount()) {
		this->handler->firePageSizeChanged(id);
	}
}

String Document::getLastErrorMsg() {
	XOJ_CHECK_TYPE(Document);

	return lastError;
}

void Document::deletePage(int pNr) {
	XOJ_CHECK_TYPE(Document);

	std::vector<PageRef>::iterator it;

	it = this->pages.begin() + pNr;
	this->pages.erase(it);

	updateIndexPageNumbers();
}

void Document::insertPage(PageRef p, int position) {
	XOJ_CHECK_TYPE(Document);

	this->pages.insert(this->pages.begin() + position, p);

	updateIndexPageNumbers();
}

void Document::addPage(PageRef p) {
	XOJ_CHECK_TYPE(Document);

	this->pages.push_back(p);

	updateIndexPageNumbers();
}

int Document::indexOf(PageRef page) {
	XOJ_CHECK_TYPE(Document);

	for (int i = 0; i < this->pages.size(); i++) {
		PageRef pg = this->pages[i];
		if (pg == page) {
			return i;
		}
	}

	return -1;
}

PageRef Document::getPage(int page) {
	XOJ_CHECK_TYPE(Document);

	if (getPageCount() <= page) {
		return NULL;
	}
	if (page < 0) {
		return NULL;
	}

	return this->pages[page];
}

XojPopplerPage * Document::getPdfPage(int page) {
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument.getPage(page);
}

XojPopplerDocument & Document::getPdfDocument() {
	XOJ_CHECK_TYPE(Document);

	return this->pdfDocument;
}

void Document::operator=(Document & doc) {
	XOJ_CHECK_TYPE(Document);

	clearDocument();

	this->pdfDocument = doc.pdfDocument;

	this->password = doc.password;
	this->createBackupOnSave = doc.createBackupOnSave;
	this->pdfFilename = doc.pdfFilename;
	this->filename = doc.filename;

	for (int i = 0; i < doc.pages.size(); i++) {
		PageRef p = doc.pages[i];
		addPage(p);
	}

	buildContentsModel();
	updateIndexPageNumbers();

	bool lastLock = tryLock();
	unlock();
	this->handler->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	if (!lastLock) { // document was locked before
		lock();
	}
}

void Document::setCreateBackupOnSave(bool backup) {
	XOJ_CHECK_TYPE(Document);

	this->createBackupOnSave = backup;
}

bool Document::shouldCreateBackupOnSave() {
	XOJ_CHECK_TYPE(Document);

	return this->createBackupOnSave;
}

