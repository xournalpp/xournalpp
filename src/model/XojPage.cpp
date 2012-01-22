#include "XojPage.h"

#include "Document.h"
#include "BackgroundImage.h"

XojPage::XojPage(double width, double heigth) {
	XOJ_INIT_TYPE(XojPage);

	this->pdfBackgroundPage = -1;
	this->bgType = BACKGROUND_TYPE_LINED;

	this->width = width;
	this->height = heigth;

	this->layer = NULL;
	this->ref = 0;
	this->currentLayer = -1;
}

XojPage::~XojPage() {
	XOJ_CHECK_TYPE(XojPage);

	for (GList * l = this->layer; l != NULL; l = l->next) {
		delete (Layer *) l->data;
	}
	g_list_free(this->layer);
	this->layer = NULL;

	XOJ_RELEASE_TYPE(XojPage);
}

void XojPage::reference() {
	XOJ_CHECK_TYPE(XojPage);

	this->ref++;
}

void XojPage::unreference() {
	XOJ_CHECK_TYPE(XojPage);

	this->ref--;
	if (ref < 1) {
		delete this;
	}
}

XojPage * XojPage::clone() {
	XojPage * page = new XojPage(this->width, this->height);

	page->backgroundImage = this->backgroundImage;
	for (GList * l = this->layer; l != NULL; l = l->next) {
		Layer * layer = (Layer *) l->data;
		page->addLayer(layer->clone());
	}

	page->currentLayer = this->currentLayer;
	page->bgType = this->bgType;
	page->pdfBackgroundPage = this->pdfBackgroundPage;
	page->backgroundColor = this->backgroundColor;

	return page;
}

void XojPage::addLayer(Layer * layer) {
	XOJ_CHECK_TYPE(XojPage);

	this->layer = g_list_append(this->layer, layer);
	this->currentLayer = -1;
}

void XojPage::insertLayer(Layer * layer, int index) {
	XOJ_CHECK_TYPE(XojPage);

	this->layer = g_list_insert(this->layer, layer, index);
	this->currentLayer = index + 1;
}

void XojPage::removeLayer(Layer * layer) {
	XOJ_CHECK_TYPE(XojPage);

	this->layer = g_list_remove(this->layer, layer);
	this->currentLayer = -1;
}

void XojPage::setSelectedLayerId(int id) {
	this->currentLayer = id;
}

ListIterator<Layer*> XojPage::layerIterator() {
	XOJ_CHECK_TYPE(XojPage);

	return ListIterator<Layer *> (this->layer);
}

int XojPage::getLayerCount() {
	XOJ_CHECK_TYPE(XojPage);

	return g_list_length(this->layer);
}

/**
 * Layer ID 0 = Background, Layer ID 1 = Layer 1
 */
int XojPage::getSelectedLayerId() {
	XOJ_CHECK_TYPE(XojPage);

	if (this->currentLayer == -1) {
		this->currentLayer = g_list_length(this->layer);
	}

	return this->currentLayer;
}

void XojPage::setBackgroundPdfPageNr(int page) {
	XOJ_CHECK_TYPE(XojPage);

	this->pdfBackgroundPage = page;
	this->bgType = BACKGROUND_TYPE_PDF;
}

void XojPage::setBackgroundColor(int color) {
	XOJ_CHECK_TYPE(XojPage);

	this->backgroundColor = color;
}

int XojPage::getBackgroundColor() {
	XOJ_CHECK_TYPE(XojPage);

	return this->backgroundColor;
}

void XojPage::setSize(double width, double height) {
	XOJ_CHECK_TYPE(XojPage);

	this->width = width;
	this->height = height;
}

double XojPage::getWidth() {
	XOJ_CHECK_TYPE(XojPage);

	return this->width;
}

double XojPage::getHeight() {
	XOJ_CHECK_TYPE(XojPage);

	return this->height;
}

int XojPage::getPdfPageNr() {
	XOJ_CHECK_TYPE(XojPage);

	return this->pdfBackgroundPage;
}

bool XojPage::isAnnotated() {
	XOJ_CHECK_TYPE(XojPage);

	ListIterator<Layer*> it = layerIterator();
	while (it.hasNext()) {
		if (it.next()->isAnnotated()) {
			return true;
		}
	}
	return false;
}

void XojPage::setBackgroundType(BackgroundType bgType) {
	XOJ_CHECK_TYPE(XojPage);

	this->bgType = bgType;

	if (bgType != BACKGROUND_TYPE_PDF) {
		this->pdfBackgroundPage = -1;
	}
	if (bgType != BACKGROUND_TYPE_IMAGE) {
		this->backgroundImage.free();
	}
}

BackgroundType XojPage::getBackgroundType() {
	XOJ_CHECK_TYPE(XojPage);

	return this->bgType;
}

BackgroundImage & XojPage::getBackgroundImage() {
	XOJ_CHECK_TYPE(XojPage);

	return this->backgroundImage;
}

void XojPage::setBackgroundImage(BackgroundImage img) {
	XOJ_CHECK_TYPE(XojPage);

	this->backgroundImage = img;
}

Layer * XojPage::getSelectedLayer() {
	XOJ_CHECK_TYPE(XojPage);

	if (this->layer == NULL) {
		addLayer(new Layer());
	}
	int layer = getSelectedLayerId() - 1;

	if (layer < 0) {
		layer = 0;
	}

	return (Layer *) g_list_nth(this->layer, layer)->data;
}


