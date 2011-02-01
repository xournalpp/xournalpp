#include "XojPopplerDocument.h"
#include "XojPopplerIter.h"

#include <glib.h>
#include <poppler/PDFDoc.h>
#include <poppler/CairoOutputDev.h>
#include <poppler/GlobalParams.h>
#include <poppler/ErrorCodes.h>
#include <poppler/Outline.h>

class _IntPopplerDocument {
public:
	_IntPopplerDocument(PDFDoc *doc) {
		this->ref = 1;

		this->doc = doc;

		output_dev = new CairoOutputDev();
		output_dev->startDoc(this->doc->getXRef(), this->doc->getCatalog());

		Catalog * catalog = this->doc->getCatalog();

		pages = new XojPopplerPage*[doc->getNumPages()];
		for (int i = 0; i < doc->getNumPages(); i++) {
			Page * page = catalog->getPage(i + 1);

			pages[i] = new XojPopplerPage(doc, output_dev, page, i);
		}
	}

private:
	~_IntPopplerDocument() {
		layersFree();

		for (int i = 0; i < doc->getNumPages(); i++) {
			delete pages[i];
		}

		delete[] pages;
		this->pages = NULL;
		delete output_dev;
		this->output_dev = NULL;
		delete doc;
		this->doc = NULL;
	}

public:
	void unreference() {
		ref--;

		if (ref <= 0) {
			delete this;
		}
	}

	void reference() {
		ref++;
	}

private:
	void layersFree() {
		//		if (!this->layers) {
		//			return;
		//		}
		//
		////		g_list_foreach(this->layers, (GFunc) layer_free, NULL);
		////		g_list_free(this->layers);
		//
		//		g_list_foreach(this->layers_rbgroups, (GFunc) g_list_free, NULL);
		//		g_list_free(this->layers_rbgroups);
		//
		////		this->layers = NULL;
		//		this->layers_rbgroups = NULL;
	}

	//	static void layer_free(Layer *layer) {
	//		if (!layer) {
	//			return;
	//		}
	//
	//		if (layer->kids) {
	//			g_list_foreach(layer->kids, (GFunc) layer_free, NULL);
	//			g_list_free(layer->kids);
	//		}
	//
	//		if (layer->label) {
	//			g_free(layer->label);
	//		}
	//
	//		g_slice_free (Layer, layer);
	//	}

public:
	PDFDoc *doc;

	//	GList *layers;
	GList *layers_rbgroups;
	CairoOutputDev *output_dev;

	XojPopplerPage ** pages;

private:
	int ref;

};

XojPopplerDocument::XojPopplerDocument() {
	this->data = NULL;
}

XojPopplerDocument::XojPopplerDocument(const XojPopplerDocument & doc) {
	this->data = doc.data;
	if (this->data) {
		this->data->reference();
	}
}

XojPopplerDocument::~XojPopplerDocument() {
	if (this->data) {
		this->data->unreference();
	}

	this->data = NULL;
}

bool XojPopplerDocument::operator==(XojPopplerDocument & doc) {
	return this->data == doc.data;
}

void XojPopplerDocument::operator=(XojPopplerDocument & doc) {
	if (this->data) {
		this->data->unreference();
	}

	this->data = doc.data;

	if (this->data) {
		this->data->reference();
	}
}

XojPopplerIter * XojPopplerDocument::getContentsIter() {
	if (this->data == NULL) {
		return NULL;
	}

	Outline * outline = this->data->doc->getOutline();
	if (outline == NULL) {
		return NULL;
	}

	GooList *items = outline->getItems();
	if (items == NULL) {
		return NULL;
	}

	return new XojPopplerIter(*this, items);
}

XojPopplerPage * XojPopplerDocument::getPage(int page) {
	if (page >= this->getPageCount() || page < 0) {
		g_critical("Document::getPdfPage(%i) out of range! (count=%i)", page, this->getPageCount());
		return NULL;
	}

	return this->data->pages[page];
}

bool XojPopplerDocument::isLoaded() {
	return this->data != NULL;
}

int XojPopplerDocument::getPageCount() {
	if (!this->data) {
		return 0;
	}
	return this->data->doc->getNumPages();
}

void XojPopplerDocument::load(char *data, int length) {
	if (!globalParams) {
		globalParams = new GlobalParams();
	}

	Object obj;
	// create stream
	obj.initNull();
	MemStream *str = new MemStream(data, 0, length, &obj);

	PDFDoc *newDoc = new PDFDoc(str, NULL, NULL);

	if (this->data) {
		this->data->unreference();
	}
	this->data = new _IntPopplerDocument(newDoc);
}

bool XojPopplerDocument::load(const char *uri, const char *password, GError **error) {
	PDFDoc * newDoc;
	GooString * filename_g;
	GooString * password_g;
	char * filename;

	if (!globalParams) {
		globalParams = new GlobalParams();
	}

	filename = g_filename_from_uri(uri, NULL, error);
	if (!filename) {
		return false;
	}

	password_g = NULL;
	if (password != NULL) {
		if (g_utf8_validate(password, -1, NULL)) {
			gchar *password_latin;

			password_latin = g_convert(password, -1, "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
			password_g = new GooString(password_latin);
			g_free(password_latin);
		} else {
			password_g = new GooString(password);
		}
	}

#ifdef G_OS_WIN32
	wchar_t *filenameW;
	int length;

	length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);

	filenameW = new WCHAR[length];
	if (!filenameW)
	return NULL;

	length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, filenameW, length);

	newDoc = new PDFDoc(filenameW, length, password_g, password_g);
	delete filenameW;
#else
	filename_g = new GooString(filename);
	newDoc = new PDFDoc(filename_g, password_g, password_g);
#endif
	g_free(filename);

	delete password_g;

	if (!newDoc->isOk()) {
		int fopen_errno;
		switch (newDoc->getErrorCode()) {
		case errOpenFile:
			// If there was an error opening the file, count it as a G_FILE_ERROR
			// and set the GError parameters accordingly. (this assumes that the
			// only way to get an errOpenFile error is if newDoc was created using
			// a filename and thus fopen was called, which right now is true.
			fopen_errno = newDoc->getFopenErrno();
			g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(fopen_errno), "%s", g_strerror(fopen_errno));
			break;
		case errBadCatalog:
			g_set_error(error, 0, 0, "Failed to read the document catalog");
			break;
		case errDamaged:
			g_set_error(error, 0, 0, "PDF document is damaged");
			break;
		case errEncrypted:
			g_set_error(error, 0, 0, "Document is encrypted");
			break;
		default:
			g_set_error(error, 0, 0, "Failed to load document");
		}

		delete newDoc;
		return false;
	}

	if (this->data) {
		this->data->unreference();
	}
	this->data = new _IntPopplerDocument(newDoc);

	return true;
}

PDFDoc * XojPopplerDocument::getDoc() {
	if (this->data) {
		return this->data->doc;
	}
	return NULL;
}

gsize XojPopplerDocument::getId() {
	return (gsize)this->data;
}


