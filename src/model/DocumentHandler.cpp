#include "DocumentHandler.h"
#include "DocumentListener.h"

DocumentHandler::DocumentHandler() {
	this->listener = NULL;
}

DocumentHandler::~DocumentHandler() {
	// Do not delete the listeners!
	g_list_free(this->listener);
}

void DocumentHandler::addListener(DocumentListener * l) {
	this->listener = g_list_append(this->listener, l);
}

void DocumentHandler::removeListener(DocumentListener * l) {
	this->listener = g_list_remove(this->listener, l);
}

void DocumentHandler::fireDocumentChanged(DocumentChangeType type) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->documentChanged(type);
	}
}

void DocumentHandler::firePageSizeChanged(int page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageSizeChanged(page);
	}
}

void DocumentHandler::firePageChanged(int page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageChanged(page);
	}
}

void DocumentHandler::firePageInserted(int page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageInserted(page);
	}
}

void DocumentHandler::firePageDeleted(int page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageDeleted(page);
	}
}

void DocumentHandler::firePageSelected(int page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		DocumentListener * dl = (DocumentListener *) l->data;
		dl->pageSelected(page);
	}
}
