#include "Page.h"

#include "Document.h"
#include "../util/Stacktrace.h"

XojPage::XojPage(double width, double heigth, int x) {
	XOJ_INIT_TYPE(XojPage);

	this->pdfBackgroundPage = -1;
	this->bgType = BACKGROUND_TYPE_LINED;

	this->width = width;
	this->height = heigth;

	this->layer = NULL;
	this->ref = 1;
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

void XojPage::reference(int debugId) {
	XOJ_CHECK_TYPE(XojPage);

	this->ref++;

	printf("debug:pref:%i (%i)\n", debugId, this->ref);
}

void XojPage::unreference(int debugId) {
	XOJ_CHECK_TYPE(XojPage);

	this->ref--;
	// TODO: there are some bugs
	printf("debug:punref:%i (%i)\n", debugId, this->ref);
	if (ref < 1) {
		delete this;
	}
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

////////////////////////////////////////////////////////

class BackgroundImageContents {
public:
	BackgroundImageContents(String filename, GError ** error) {
		this->filename = filename;
		this->ref = 1;
		this->pageId = -1;
		this->attach = false;
		this->pixbuf = gdk_pixbuf_new_from_file(filename.c_str(), error);
	}

private:
	~BackgroundImageContents() {
		gdk_pixbuf_unref(this->pixbuf);
	}

	void unreference() {
		this->ref--;
		if (this->ref < 0) {
			delete this;
		}
	}

	void reference() {
		this->ref++;
	}

private:
	int ref;
	String filename;
	bool attach;
	int pageId;
	GdkPixbuf * pixbuf;

	friend class BackgroundImage;
};

BackgroundImage::BackgroundImage() {
	XOJ_INIT_TYPE(BackgroundImage);

	this->img = NULL;
}

BackgroundImage::~BackgroundImage() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		BackgroundImageContents * img = (BackgroundImageContents *) this->img;
		img->unreference();
	}

	XOJ_RELEASE_TYPE(BackgroundImage);
}

String BackgroundImage::getFilename() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL) {
		return ((BackgroundImageContents *) this->img)->filename;
	}
	return NULL;
}

void BackgroundImage::loadFile(String filename, GError ** error) {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = new BackgroundImageContents(filename, error);
}

void BackgroundImage::setAttach(bool attach) {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img != NULL) {
		((BackgroundImageContents *) this->img)->attach = true;
	} else {
		g_warning("BackgroundImage::setAttach:please load first an image before call setAttach!");
		Stacktrace::printStracktrace();
	}
}

void BackgroundImage::operator =(BackgroundImage & img) {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = img.img;
	if (this->img) {
		((BackgroundImageContents *) this->img)->reference();
	}
}

bool BackgroundImage::operator ==(const BackgroundImage & img) {
	XOJ_CHECK_TYPE(BackgroundImage);

	return this->img == img.img;
}

void BackgroundImage::free() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = NULL;
}

void BackgroundImage::clearSaveState() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		((BackgroundImageContents *) this->img)->pageId = -1;
	}
}

int BackgroundImage::getCloneId() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		return ((BackgroundImageContents *) this->img)->pageId;
	}
	return -1;
}

void BackgroundImage::setCloneId(int id) {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		((BackgroundImageContents *) this->img)->pageId = id;
	}
}

void BackgroundImage::setFilename(String filename) {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		((BackgroundImageContents *) this->img)->filename = filename;
	}
}

bool BackgroundImage::isAttached() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		return ((BackgroundImageContents *) this->img)->attach;
	}
	return false;
}

GdkPixbuf * BackgroundImage::getPixbuf() {
	XOJ_CHECK_TYPE(BackgroundImage);

	if (this->img) {
		return ((BackgroundImageContents *) this->img)->pixbuf;
	}
	return NULL;
}

