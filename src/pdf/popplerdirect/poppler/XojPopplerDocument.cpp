#include "XojPopplerDocument.h"

#include <poppler/PDFDoc.h>
#include <poppler/GlobalParams.h>
#include <poppler/ErrorCodes.h>
#include <poppler/Outline.h>
#include "XojPopplerIter.h"

#include <config.h>
#include <Util.h>

#include <glib.h>

class _IntPopplerDocument
{
public:

	_IntPopplerDocument(PDFDoc* doc)
	{
		XOJ_INIT_TYPE(_IntPopplerDocument);
		this->ref = 1;

		this->doc = doc;

		g_mutex_init(&this->docMutex);

		output_dev = new CairoOutputDev();
		output_dev->startDoc(this->doc);

		Catalog* catalog = this->doc->getCatalog();

		for (int i = 0; i < doc->getNumPages(); i++)
		{
			Page* page = catalog->getPage(i + 1);

			pages.push_back(std::make_shared<XojPopplerPage>(doc, &this->docMutex, output_dev, page, i));
		}
		
		this->layers_rbgroups = NULL;
	}

private:

	~_IntPopplerDocument()
	{
		XOJ_CHECK_TYPE(_IntPopplerDocument);

		layersFree();

		this->pages.clear();
		delete output_dev;
		this->output_dev = NULL;
		delete doc;
		this->doc = NULL;

		XOJ_RELEASE_TYPE(_IntPopplerDocument);
	}

public:

	void unreference()
	{
		XOJ_CHECK_TYPE(_IntPopplerDocument);

		this->ref--;

		if (this->ref <= 0)
		{
			delete this;
		}
	}

	void reference()
	{
		XOJ_CHECK_TYPE(_IntPopplerDocument);

		this->ref++;
	}

private:

	void layersFree()
	{
		XOJ_CHECK_TYPE(_IntPopplerDocument);

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
	XOJ_TYPE_ATTRIB;

	PDFDoc* doc;

	//	GList *layers;
	GList* layers_rbgroups;
	CairoOutputDev* output_dev;

	vector<XojPdfPageSPtr> pages;

private:
	int ref;

	GMutex docMutex;

};

XojPopplerDocument::XojPopplerDocument()
{
	XOJ_INIT_TYPE(XojPopplerDocument);

	this->data = NULL;
}

XojPopplerDocument::XojPopplerDocument(const XojPopplerDocument& doc)
{
	XOJ_INIT_TYPE(XojPopplerDocument);

	this->data = doc.data;
	if (this->data)
	{
		this->data->reference();
	}
}

XojPopplerDocument::~XojPopplerDocument()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (this->data)
	{
		this->data->unreference();
	}

	this->data = NULL;

	XOJ_RELEASE_TYPE(XojPopplerDocument);
}

bool XojPopplerDocument::operator==(XojPopplerDocument& doc)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	return this->data == doc.data;
}

void XojPopplerDocument::operator=(XojPopplerDocument& doc)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	this->assign(&doc);
}

void XojPopplerDocument::assign(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (this->data)
	{
		this->data->unreference();
	}

	this->data = ((XojPopplerDocument*)doc)->data;

	if (this->data)
	{
		this->data->reference();
	}
}

bool XojPopplerDocument::equals(XojPdfDocumentInterface* doc)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	return this->data == ((XojPopplerDocument*)doc)->data;
}

XojPdfBookmarkIterator* XojPopplerDocument::getContentsIter()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (this->data == NULL)
	{
		return NULL;
	}

	Outline* outline = this->data->doc->getOutline();
	if (outline == NULL)
	{
		return NULL;
	}

	GooList* items = const_cast<GooList *>(outline->getItems());
	if (items == NULL)
	{
		return NULL;
	}

	return new XojPopplerIter(*this, items);
}

XojPdfPageSPtr XojPopplerDocument::getPage(size_t page)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (page == size_t_npos || page >= this->getPageCount())
	{
		g_critical("Document::getPdfPage(%lu) out of range! (count=%lu)", page, this->getPageCount());
		return NULL;
	}

	return this->data->pages[page];
}

bool XojPopplerDocument::isLoaded()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	return this->data != NULL;
}

size_t XojPopplerDocument::getPageCount()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (!this->data)
	{
		return 0;
	}
	return static_cast<size_t>(this->data->doc->getNumPages()); // It shouldn't be lower than 0, but I don't trust poppler ;)
}

void XojPopplerDocument::load(char* data, int length)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (!globalParams)
	{
		globalParams = new GlobalParams();
	}

	MemStream* str = new MemStream(data, 0, length, Object(objNull));

	PDFDoc* newDoc = new PDFDoc(str, NULL, NULL);

	if (this->data)
	{
		this->data->unreference();
	}
	this->data = new _IntPopplerDocument(newDoc);
}

bool XojPopplerDocument::load(path filename, string password, GError** error)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	PDFDoc* newDoc;

	if (!globalParams)
	{
		globalParams = new GlobalParams();
	}

	if (filename.empty())
	{
		return false;
	}

	GooString* filename_g = new GooString(filename.c_str());
	GooString* password_g = new GooString(password.c_str());
	newDoc = new PDFDoc(filename_g, password_g, password_g);
	delete password_g;

	if (!newDoc->isOk())
	{
		int fopen_errno;
		switch (newDoc->getErrorCode())
		{
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

	if (this->data)
	{
		this->data->unreference();
	}
	this->data = new _IntPopplerDocument(newDoc);

	return true;
}

PDFDoc* XojPopplerDocument::getDoc()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (this->data)
	{
		return this->data->doc;
	}
	return NULL;
}

gsize XojPopplerDocument::getId()
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	return (gsize) this->data;
}

bool XojPopplerDocument::save(path filename, GError** error)
{
	XOJ_CHECK_TYPE(XojPopplerDocument);

	if (this->data == NULL)
	{
		return false;
	}

	// TODO !!!!!!
	return false;

	//	GooString * fname = new GooString(filename.c_str());
	//	int err_code = this->data->doc->saveAs(fname);
	//	delete fname;
	//
	//	switch (err_code) {
	//	case errNone:
	//		break;
	//	case errOpenFile:
	//		g_set_error(error, 0, 0, _("Failed to open file for writing"));
	//		break;
	//	case errEncrypted:
	//		g_set_error(error, 0, 0, _("Document is encrypted"));
	//		break;
	//	default:
	//		g_set_error(error, 0, 0, _("Failed to save document"));
	//	}
	//
	//	return err_code == errNone;
}
