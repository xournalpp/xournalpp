#include "Page.h"

#include "Document.h"
#include "../util/Stacktrace.h"
// TODO: AA: type check

XojPage::XojPage(double width, double heigth) {
	this->pdfBackgroundPage = -1;
	this->bgType = BACKGROUND_TYPE_LINED;

	this->width = width;
	this->height = heigth;

	this->layer = NULL;
	this->ref = 0;
	this->currentLayer = -1;
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

	if (bgType != BACKGROUND_TYPE_PDF) {
		pdfBackgroundPage = -1;
	}
	if (bgType != BACKGROUND_TYPE_IMAGE) {
		backgroundImage.free();
	}
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
	this->img = NULL;
}

BackgroundImage::~BackgroundImage() {
	if (this->img) {
		BackgroundImageContents * img = (BackgroundImageContents *) this->img;
		img->unreference();
	}
}

String BackgroundImage::getFilename() {
	if (this->img != NULL) {
		return ((BackgroundImageContents *) this->img)->filename;
	}
	return NULL;
}

void BackgroundImage::loadFile(String filename, GError ** error) {
	if (this->img != NULL) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = new BackgroundImageContents(filename, error);
}

void BackgroundImage::setAttach(bool attach) {
	if (this->img != NULL) {
		((BackgroundImageContents *) this->img)->attach = true;
	} else {
		g_warning("BackgroundImage::setAttach:please load first an image before call setAttach!");
		Stacktrace::printStracktrace();
	}
}

void BackgroundImage::operator =(BackgroundImage & img) {
	if (this->img) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = img.img;
	if (this->img) {
		((BackgroundImageContents *) this->img)->reference();
	}
}

bool BackgroundImage::operator ==(const BackgroundImage & img) {
	return this->img == img.img;
}

void BackgroundImage::free() {
	if (this->img) {
		((BackgroundImageContents *) this->img)->unreference();
	}
	this->img = NULL;
}

void BackgroundImage::clearSaveState() {
	if (this->img) {
		((BackgroundImageContents *) this->img)->pageId = -1;
	}
}

int BackgroundImage::getCloneId() {
	if (this->img) {
		return ((BackgroundImageContents *) this->img)->pageId;
	}
	return -1;
}

void BackgroundImage::setCloneId(int id) {
	if (this->img) {
		((BackgroundImageContents *) this->img)->pageId = id;
	}
}

void BackgroundImage::setFilename(String filename) {
	if (this->img) {
		((BackgroundImageContents *) this->img)->filename = filename;
	}
}

bool BackgroundImage::isAttached() {
	if (this->img) {
		return ((BackgroundImageContents *) this->img)->attach;
	}
	return false;
}

GdkPixbuf * BackgroundImage::getPixbuf() {
	if (this->img) {
		return ((BackgroundImageContents *) this->img)->pixbuf;
	}
	return NULL;
}

