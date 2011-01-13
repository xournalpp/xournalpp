#include "Document.h"

#include <poppler-document.h>
#include <poppler-page.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "LinkDestination.h"
#include "../gettext.h"
#include "../pdf/popplerGlibExt/PopplerGlibExtension.h"

#include <string.h>

Document::Document(DocumentHandler * handler) {
	this->pdfPageCount = 0;
	this->pdfDocument = NULL;
	this->pdfPages = NULL;
	this->handler = handler;
	this->contentsModel = NULL;
	this->pages = NULL;
	this->pageCount = 0;
	this->pagesArrayLen = 0;
	this->preview = NULL;
}

Document::~Document() {
	clearDocument(true);

	if (contentsModel) {
		g_object_unref(contentsModel);
		contentsModel = NULL;
	}
}

void Document::clearDocument(bool destroy) {
	int count = pdfPageCount;
	pdfPageCount = 0;

	if (this->preview) {
		cairo_surface_destroy(this->preview);
		this->preview = NULL;
	}

	if (!destroy) {
		handler->fireDocumentChanged(DOCUMENT_CHANGE_CLEARED);
	}

	for (int i = 0; i < this->pageCount; i++) {
		this->pages[i]->unreference();
	}
	g_free(pages);
	this->pages = NULL;
	this->pageCount = 0;
	this->pagesArrayLen = 0;

	for (int i = 0; i < count; i++) {
		if (pdfPages[i]) {
			g_object_unref(pdfPages[i]);
			pdfPages[i] = NULL;
		}
	}
	delete[] pdfPages;
	pdfPages = NULL;

	if (pdfDocument) {
		g_object_unref(pdfDocument);
		pdfDocument = NULL;
	}
}

int Document::getPageCount() {
	return this->pageCount;
}

int Document::getPdfPageCount() {
	return this->pdfPageCount;
}

void Document::setFilename(String filename) {
	this->filename = filename;
}

String Document::getFilename() {
	return filename;
}

String Document::getPdfFilename() {
	return pdfFilename;
}

cairo_surface_t * Document::getPreview() {
	return this->preview;
}

void Document::setPreview(cairo_surface_t * preview) {
	if (this->preview) {
		cairo_surface_destroy(this->preview);
	}
	if (preview) {
		this->preview = cairo_surface_reference(preview);
	} else {
		this->preview = NULL;
	}

}

const char * Document::getEvMetadataFilename() {
	String uri = "file://";
	if (filename.c_str() != NULL) {
		uri += filename;
		return uri.c_str();
	}
	if (pdfFilename.c_str() != NULL) {
		uri += pdfFilename;
		return uri.c_str();
	}
	return NULL;
}

bool Document::isPdfDocumentLoaded() {
	return pdfDocument != NULL;
}

void Document::linkFromDest(LinkDestination *link, PopplerDest *dest) {
	const char *unimplementedDest = NULL;

	switch (dest->type) {
	case POPPLER_DEST_XYZ: {
		double height;
		PopplerPage *popplerPage = getPdfPage(MAX (0, dest->page_num - 1));
		if (!popplerPage) {
			return;
		}

		poppler_page_get_size(popplerPage, NULL, &height);

		link->setPage(dest->page_num - 1);
		if (dest->change_left) {
			link->setChangeLeft(dest->left);
		}

		if (dest->change_top) {
			link->setChangeTop(height - MIN (height, dest->top));
		}

		if (dest->change_zoom) {
			link->setChangeZoom(dest->zoom);
		}

		return;
	}
	case POPPLER_DEST_FITB:
	case POPPLER_DEST_FIT:
		link->setPage(dest->page_num - 1);
		//		ev_dest = ev_link_dest_new_fit(dest->page_num - 1);
		return;
	case POPPLER_DEST_FITBH:
	case POPPLER_DEST_FITH: {
		//		PopplerPage *popplerPage = getPage(MAX (0, dest->page_num - 1));
		//		if (!popplerPage) {
		//			return;
		//		}
		//		double height;
		//		poppler_page_get_size(popplerPage, NULL, &height);
		//		ev_dest = ev_link_dest_new_fith(dest->page_num - 1, height - MIN (height, dest->top), dest->change_top);
		link->setPage(dest->page_num - 1);
		return;
	}
	case POPPLER_DEST_FITBV:
	case POPPLER_DEST_FITV:
		//		ev_dest = ev_link_dest_new_fitv(dest->page_num - 1, dest->left, dest->change_left);
		link->setPage(dest->page_num - 1);
		return;
	case POPPLER_DEST_FITR: {
		link->setPage(dest->page_num - 1);
		//		PopplerPage *poppler_page;
		//		double height;
		//
		//		poppler_page = poppler_document_get_page(pdf_document->document, MAX (0, dest->page_num - 1));
		//		poppler_page_get_size(poppler_page, NULL, &height);
		//		ev_dest = ev_link_dest_new_fitr(dest->page_num - 1, dest->left, height - MIN (height, dest->bottom),
		//				dest->right, height - MIN (height, dest->top));
		//		g_object_unref(poppler_page);
		return;
	}
	case POPPLER_DEST_NAMED: {
		PopplerDest * dest2 = poppler_document_find_dest(this->pdfDocument, dest->named_dest);
		if (dest2) {
			linkFromDest(link, dest2);
			poppler_dest_free(dest2);
		}
		return;
	}
	case POPPLER_DEST_UNKNOWN:
		unimplementedDest = "POPPLER_DEST_UNKNOWN";
		break;
	}

	if (unimplementedDest) {
		g_warning ("Unimplemented destination: %s",
				unimplementedDest);
	}

	link->setPage(dest->page_num - 1);
}

LinkDest * Document::linkFromAction(PopplerAction *action) {
	LinkDest *link = link_dest_new();
	link->dest = new LinkDestination();
	const char *unimplementedAction = NULL;

	switch (action->type) {
	case POPPLER_ACTION_NONE:
		break;
	case POPPLER_ACTION_GOTO_DEST: {
		if (action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
			PopplerDest *dest = poppler_document_find_dest(pdfDocument, action->goto_dest.dest->named_dest);
			if (dest) {
				link->dest->setPage(dest->page_num);
				poppler_dest_free(dest);
				break;
			}
		} else {
			linkFromDest(link->dest, action->goto_dest.dest);
		}
	}
		break;
	case POPPLER_ACTION_GOTO_REMOTE: {
		//		EvLinkDest *dest;
		//
		//		dest = ev_link_dest_from_dest(pdf_document, action->goto_remote.dest);
		//		ev_action = ev_link_action_new_remote(dest, action->goto_remote.file_name);

	}
		break;
	case POPPLER_ACTION_LAUNCH:
		//		ev_action = ev_link_action_new_launch(action->launch.file_name, action->launch.params);
		break;
	case POPPLER_ACTION_URI:
		//		ev_action = ev_link_action_new_external_uri(action->uri.uri);
		break;
	case POPPLER_ACTION_NAMED:
		//		ev_action = ev_link_action_new_named(action->named.named_dest);
		break;
	case POPPLER_ACTION_MOVIE:
		unimplementedAction = "POPPLER_ACTION_MOVIE";
		break;
	case POPPLER_ACTION_UNKNOWN:
		unimplementedAction = "POPPLER_ACTION_UNKNOWN";
	}

	if (unimplementedAction) {
		g_warning ("Unimplemented action: %s", unimplementedAction);
	}

	if (link->dest->getPage() != -1) {
		translatePdfPageToXournal(link->dest);
	}

	return link;
}

void Document::translatePdfPageToXournal(LinkDestination *dest) {
	int page = dest->getPage();
	for (int i = 0; i < this->pageCount; i++) {
		XojPage * p = this->pages[i];
		if (p->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			if (p->getPdfPageNr() == page) {
				dest->setPage(i);
				return;
			}
		}
	}

	// TODO: Handle this, and ask the user if he want to insert the page which obvious don't exist in the current document
	dest->setPage(-1);
}

void Document::buildTreeContentsModel(GtkTreeIter *parent, PopplerIndexIter *iter) {
	do {
		GtkTreeIter treeIter = { 0 };
		LinkDest * link = NULL;

		PopplerAction *action = poppler_index_iter_get_action(iter);
		gboolean expand = poppler_index_iter_is_open(iter);

		if (!action) {
			continue;
		}

		link = linkFromAction(action);

		if (strlen(action->any.title) <= 0) {
			g_object_unref(link);
			poppler_action_free(action);
			continue;
		}

		link->dest->setExpand(expand);
		link->dest->setName(action->any.title);

		gtk_tree_store_append(GTK_TREE_STORE (contentsModel), &treeIter, parent);
		char *titleMarkup = g_markup_escape_text(action->any.title, -1);

		gtk_tree_store_set(GTK_TREE_STORE (contentsModel), &treeIter, DOCUMENT_LINKS_COLUMN_NAME, titleMarkup,
				DOCUMENT_LINKS_COLUMN_LINK, link, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, "", -1);

		g_free(titleMarkup);
		g_object_unref(link);

		PopplerIndexIter *child = poppler_index_iter_get_child(iter);
		if (child) {
			buildTreeContentsModel(&treeIter, child);
		}
		poppler_index_iter_free(child);
		poppler_action_free(action);

	} while (poppler_index_iter_next(iter));
}

void Document::buildContentsModel() {
	if (contentsModel) {
		g_object_unref(contentsModel);
		contentsModel = NULL;
	}

	if (pdfDocument == NULL) {
		return;
	}

	PopplerIndexIter *iter = poppler_index_iter_new(pdfDocument);
	if (iter == NULL) {
		// No Bookmarks
		return;
	}

	contentsModel = (GtkTreeModel *) gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_BOOLEAN, G_TYPE_STRING);
	g_object_ref(contentsModel);
	buildTreeContentsModel(NULL, iter);
	poppler_index_iter_free(iter);
}

GtkTreeModel * Document::getContentsModel() {
	return contentsModel;
}

bool Document::fillPageLabels(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, Document *doc) {
	LinkDest * link = NULL;

	gtk_tree_model_get(tree_model, iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

	if (!link) {
		return false;
	}

	gint page = link->dest->getPage();

	if (page < 0) {
		return false;
	}

	gchar *pageLabel = g_strdup_printf("%i", page + 1);
	gtk_tree_store_set(GTK_TREE_STORE (tree_model), iter, DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, pageLabel, -1);

	g_free(pageLabel);

	g_object_unref(link);
	return false;
}

void Document::updateIndexPageNumbers() {
	if (contentsModel != NULL) {
		gtk_tree_model_foreach(contentsModel, (GtkTreeModelForeachFunc) fillPageLabels, this);
	}
}

bool Document::readPdf(String filename, bool initPages) {
	GError *poppler_error = NULL;
	PopplerDocument * oldDoc = pdfDocument;

	String uri = "file://";
	uri += filename;

	pdfDocument = poppler_document_new_from_file(uri.c_str(), password.c_str(), &poppler_error);

	if (pdfDocument == NULL) {
		char * txt = g_strdup_printf("Document == NULL! (%s)", filename.c_str());
		lastError = txt;
		g_free(txt);
		return false;
	}

	pdfFilename = filename;

	lastError = NULL;
	if (oldDoc) {
		g_object_unref(oldDoc);
	}
	pdfPageCount = 0;

	if (initPages) {
		for (int i = 0; i < this->pageCount; i++) {
			this->pages[i]->unreference();
		}
		g_free(this->pages);
		this->pages = NULL;
		this->pageCount = 0;
		this->pagesArrayLen = 0;
	}

	// this is not working...
	for (int i = 0; i < pdfPageCount; i++) {
		if (pdfPages[i]) {
			g_object_unref(pdfPages[i]);
		}
	}
	delete[] pdfPages;

	pdfPageCount = poppler_document_get_n_pages(pdfDocument);

	pdfPages = new PopplerPage *[pdfPageCount];
	for (int i = 0; i < pdfPageCount; i++) {
		pdfPages[i] = NULL;
	}

	if (initPages) {
		for (int i = 0; i < pdfPageCount; i++) {
			double width = 0;
			double height = 0;
			poppler_page_get_size(pdfPages[i], &width, &height);

			XojPage * p = new XojPage(width, height);
			p->setBackgroundPdfPageNr(i);
			addPage(p);
		}
	}

	buildContentsModel();
	updateIndexPageNumbers();

	handler->fireDocumentChanged(DOCUMENT_CHANGE_PDF_BOOKMARKS);

	return true;
}

void Document::setPageSize(XojPage * p, double width, double height) {
	p->setSize(width, height);

	int id = indexOf(p);
	if (id >= 0 && id < this->pageCount) {
		firePageSizeChanged(id);
	}
}

String Document::getLastErrorMsg() {
	return lastError;
}

void Document::deletePage(int pNr) {
	this->pages[pNr]->unreference();
	for (int i = pNr; i < this->pageCount; i++) {
		this->pages[i] = this->pages[i + 1];
	}
	this->pageCount--;
}

void Document::insertPage(XojPage * p, int position) {
	if (this->pagesArrayLen <= this->pageCount + 1) {
		this->pagesArrayLen += 100;
		this->pages = (XojPage **) g_realloc(this->pages, sizeof(XojPage *) * this->pagesArrayLen);
	}

	for (int i = position; i < this->pageCount; i++) {
		this->pages[i + 1] = this->pages[i];
	}

	this->pages[position] = p;

	this->pageCount++;
	p->reference();
}

void Document::addPage(XojPage * p) {
	if (this->pagesArrayLen <= this->pageCount + 1) {
		this->pagesArrayLen += 100;
		this->pages = (XojPage **) g_realloc(this->pages, sizeof(XojPage *) * this->pagesArrayLen);
	}

	this->pages[this->pageCount++] = p;
	p->reference();
}

int Document::indexOf(XojPage * page) {
	for (int i = 0; i < this->pageCount; i++) {
		if (page == this->pages[i]) {
			return i;
		}
	}

	return -1;
}

XojPage * Document::getPage(int page) {
	if (getPageCount() <= page) {
		return NULL;
	}
	if (page < 0) {
		return NULL;
	}

	return this->pages[page];
}

void Document::firePageSizeChanged(int page) {
	handler->firePageSizeChanged(page);
}

TextPage * Document::getTextPage(int page) {
	PopplerPage * p = getPdfPage(page);
	if (p == NULL) {
		return NULL;
	}

	return poppler_page_get_text_page(p);
}

PopplerPage * Document::getPdfPage(int page) {
	if (page >= pdfPageCount || page < 0) {
		g_critical("Document::getPdfPage(%i) out of range! (count=%i)", page, pdfPageCount);
		return NULL;
	}

	if (pdfPages == NULL) {
		return NULL;
	}

	PopplerPage * p = pdfPages[page];
	if (p == NULL) {
		p = poppler_document_get_page(pdfDocument, page);
		pdfPages[page] = p;
	}

	return p;
}

void Document::operator=(const Document & doc) {
	clearDocument();

	this->pdfDocument = doc.pdfDocument;
	if (pdfDocument) {
		g_object_ref(pdfDocument);
	}

	this->password = doc.password;
	this->pdfPageCount = doc.pdfPageCount;
	this->createBackupOnSave = doc.createBackupOnSave;
	this->pdfFilename = doc.pdfFilename;
	this->filename = doc.filename;

	pdfPages = new PopplerPage *[pdfPageCount];
	for (int i = 0; i < pdfPageCount; i++) {
		pdfPages[i] = doc.pdfPages[i];
		if (pdfPages[i]) {
			g_object_ref(pdfPages[i]);
		}
	}

	for (int i = 0; i < doc.pageCount; i++) {
		XojPage * p = doc.pages[i];
		addPage(p);
	}

	buildContentsModel();
	updateIndexPageNumbers();

	handler->fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
}

void Document::setCreateBackupOnSave(bool backup) {
	this->createBackupOnSave = backup;
}

bool Document::shouldCreateBackupOnSave() {
	return this->createBackupOnSave;
}

///////////////////////////////////////////////////////////
//Document Handler/////////////////////////////////////////
///////////////////////////////////////////////////////////

DocumentHandler::DocumentHandler() {
	listener = NULL;
}

DocumentHandler::~DocumentHandler() {
	// Do not delte the listeners!
	g_list_free(listener);
}

void DocumentHandler::addListener(DocumentListener * l) {
	listener = g_list_append(listener, l);
}

void DocumentHandler::removeListener(DocumentListener * l) {
	listener = g_list_remove(listener, l);
}

void DocumentHandler::fireDocumentChanged(DocumentChangeType type) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->documentChanged(type);
	}
}

void DocumentHandler::firePageSizeChanged(int page) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageSizeChanged(page);
	}
}

void DocumentHandler::firePageChanged(int page) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageChanged(page);
	}
}

void DocumentHandler::firePageInserted(int page) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageInserted(page);
	}
}

void DocumentHandler::firePageDeleted(int page) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageDeleted(page);
	}
}

void DocumentHandler::firePageSelected(int page) {
	for (GList * l = listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageSelected(page);
	}
}

///////////////////////////////////////////////////////////
//Document Listener////////////////////////////////////////
///////////////////////////////////////////////////////////


DocumentListener::DocumentListener() {
	this->handler = NULL;
}

DocumentListener::~DocumentListener() {
	unregisterListener();
}

void DocumentListener::registerListener(DocumentHandler * handler) {
	this->handler = handler;
	handler->addListener(this);
}

void DocumentListener::unregisterListener() {
	if (this->handler) {
		this->handler->removeListener(this);
	}
}
