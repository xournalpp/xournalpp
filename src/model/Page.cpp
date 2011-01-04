#include "Page.h"

#include "Document.h"

XojPage::XojPage() {
	pdfBackgroundPage = -1;

	bgType = BACKGROUND_TYPE_LINED;

	width = 595.00;
	height = 842.00;

	layer = NULL;

	ref = 0;

	currentLayer = -1;
}

void XojPage::reference() {
	ref++;
}

void XojPage::unreference() {
	ref--;
	if (ref < 1) {
		delete this;
	}
}

XojPage::~XojPage() {
	for (GList * l = layer; l != NULL; l = l->next) {
		delete (Layer *) l->data;
	}
	g_list_free(layer);
	layer = NULL;
}

void XojPage::addLayer(Layer * layer) {
	this->layer = g_list_append(this->layer, layer);
	currentLayer = -1;
}

void XojPage::insertLayer(Layer * layer, int index) {
	this->layer = g_list_insert(this->layer, layer, index);
	currentLayer = index + 1;
}

void XojPage::removeLayer(Layer * layer) {
	this->layer = g_list_remove(this->layer, layer);
	currentLayer = -1;
}

void XojPage::setSelectedLayerId(int id) {
	this->currentLayer = id;
}

ListIterator<Layer*> XojPage::layerIterator() {
	return ListIterator<Layer *> (this->layer);
}

int XojPage::getLayerCount() {
	return g_list_length(this->layer);
}

/**
 * Layer ID 0 = Background, Layer ID 1 = Layer 1
 */
int XojPage::getSelectedLayerId() {
	if (currentLayer == -1) {
		currentLayer = g_list_length(this->layer);
	}

	return currentLayer;
}

void XojPage::setBackgroundPdfPageNr(int page) {
	pdfBackgroundPage = page;
	bgType = BACKGROUND_TYPE_PDF;
}

void XojPage::setBackgroundColor(int color) {
	backgroundColor = color;
}

int XojPage::getBackgroundColor() {
	return backgroundColor;
}

void XojPage::setSize(double width, double height) {
	this->width = width;
	this->height = height;
}

double XojPage::getWidth() {
	return width;
}

double XojPage::getHeight() {
	return height;
}

int XojPage::getPdfPageNr() {
	return pdfBackgroundPage;
}

bool XojPage::isAnnotated() {
	ListIterator<Layer*> it = layerIterator();
	while (it.hasNext()) {
		if (it.next()->isAnnotated()) {
			return true;
		}
	}
	return false;
}

void XojPage::setBackgroundType(BackgroundType bgType) {
	this->bgType = bgType;
	pdfBackgroundPage = -1;
}

BackgroundType XojPage::getBackgroundType() {
	return bgType;
}

Layer * XojPage::getSelectedLayer() {
	if (this->layer == NULL) {
		addLayer(new Layer());
	}
	int layer = getSelectedLayerId() - 1;

	if (layer < 0) {
		layer = 0;
	}

	return (Layer *) g_list_nth(this->layer, layer)->data;
}

