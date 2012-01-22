#include "PageRef.h"
#include "XojPage.h"
#include "BackgroundImage.h"

PageRef::PageRef() {
	XOJ_INIT_TYPE(PageRef);
	this->page = NULL;
}

PageRef::PageRef(const PageRef & ref) {
	XOJ_INIT_TYPE(PageRef);
	this->page = ref.page;
	if (this->page) {
		this->page->reference();
	}
}

PageRef::PageRef(XojPage * page) {
	XOJ_INIT_TYPE(PageRef);
	this->page = page;
	if (this->page) {
		this->page->reference();
	}
}

PageRef::~PageRef() {
	XOJ_CHECK_TYPE(PageRef);

	if (this->page) {
		this->page->unreference();
		this->page = NULL;
	}

	XOJ_RELEASE_TYPE(PageRef);
}

void PageRef::operator=(const PageRef & ref) {
	*this = ref.page;
}

void PageRef::operator=(XojPage * page) {
	if(this->page) {
		this->page->unreference();
	}
	this->page = page;
	if(this->page) {
		this->page->reference();
	}
}

PageRef PageRef::clone() {
	return PageRef(this->page->clone());
}

bool PageRef::isValid() {
	XOJ_CHECK_TYPE(PageRef);

	return this->page != NULL;
}

PageRef::operator XojPage *() {
	XOJ_CHECK_TYPE(PageRef);

	return this->page;
}

bool PageRef::operator==(const PageRef & ref) {
	XOJ_CHECK_TYPE(PageRef);

	return this->page == ref.page;
}

void PageRef::setBackgroundPdfPageNr(int page) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setBackgroundPdfPageNr(page);
}

void PageRef::setBackgroundType(BackgroundType bgType) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setBackgroundType(bgType);
}

BackgroundType PageRef::getBackgroundType() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, BACKGROUND_TYPE_NONE);

	return this->page->getBackgroundType();
}

void PageRef::setSize(double width, double height) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setSize(width, height);
}

double PageRef::getWidth() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, 0);

	return this->page->getWidth();
}

double PageRef::getHeight() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, 0);

	return this->page->getHeight();
}

void PageRef::addLayer(Layer * layer) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->addLayer(layer);
}

void PageRef::insertLayer(Layer * layer, int index) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->insertLayer(layer, index);
}

void PageRef::removeLayer(Layer * layer) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->removeLayer(layer);
}

int PageRef::getPdfPageNr() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, -1);

	return this->page->getPdfPageNr();
}

bool PageRef::isAnnotated() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, false);

	return this->page->isAnnotated();
}

void PageRef::setBackgroundColor(int color) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setBackgroundColor(color);
}

int PageRef::getBackgroundColor() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, 0);

	return this->page->getBackgroundColor();
}

ListIterator<Layer*> PageRef::layerIterator() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, NULL);

	return this->page->layerIterator();
}

int PageRef::getLayerCount() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, 0);

	return this->page->getLayerCount();
}

int PageRef::getSelectedLayerId() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, 0);

	return this->page->getSelectedLayerId();
}

void PageRef::setSelectedLayerId(int id) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setSelectedLayerId(id);
}

Layer * PageRef::getSelectedLayer() {
	XOJ_CHECK_TYPE(PageRef);
	g_return_val_if_fail(this->page != NULL, NULL);

	return this->page->getSelectedLayer();
}

BackgroundImage & PageRef::getBackgroundImage() {
	XOJ_CHECK_TYPE(PageRef);

	return this->page->getBackgroundImage();
}

void PageRef::setBackgroundImage(BackgroundImage & img) {
	XOJ_CHECK_TYPE(PageRef);
	g_return_if_fail(this->page != NULL);

	this->page->setBackgroundImage(img);
}

