#include "PdfCache.h"
#include <stdio.h>

class PdfCacheEntry
{
public:
	PdfCacheEntry(XojPopplerPage* popplerPage, cairo_surface_t* img)
	{
		XOJ_INIT_TYPE(PdfCacheEntry);

		this->popplerPage = popplerPage;
		this->rendered = img;
	}
	~PdfCacheEntry()
	{
		XOJ_CHECK_TYPE(PdfCacheEntry);

		this->popplerPage = NULL;
		cairo_surface_destroy(this->rendered);
		this->rendered = NULL;

		XOJ_RELEASE_TYPE(PdfCacheEntry);
	}

	XOJ_TYPE_ATTRIB;

	XojPopplerPage* popplerPage;
	cairo_surface_t* rendered;
};

PdfCache::PdfCache(int size)
{
	XOJ_INIT_TYPE(PdfCache);

	this->data = NULL;
	this->size = size;
	this->zoom = -1;

	g_mutex_init(&this->renderMutex);
}

PdfCache::~PdfCache()
{
	XOJ_CHECK_TYPE(PdfCache);

	clearCache();
	this->size = 0;

	XOJ_RELEASE_TYPE(PdfCache);
}

void PdfCache::setZoom(double zoom)
{
	XOJ_CHECK_TYPE(PdfCache);

	if (this->zoom == zoom)
	{
		return;
	}
	this->zoom = zoom;

	clearCache();
}

void PdfCache::clearCache()
{
	XOJ_CHECK_TYPE(PdfCache);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfCacheEntry* e = (PdfCacheEntry*) l->data;
		delete e;
	}
	g_list_free(this->data);
	this->data = NULL;
}

cairo_surface_t* PdfCache::lookup(XojPopplerPage* popplerPage)
{
	XOJ_CHECK_TYPE(PdfCache);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfCacheEntry* e = (PdfCacheEntry*) l->data;
		XOJ_CHECK_TYPE_OBJ(e, PdfCacheEntry);
		if (e->popplerPage == popplerPage)
		{
			return e->rendered;
		}
	}

	return NULL;
}

void PdfCache::cache(XojPopplerPage* popplerPage, cairo_surface_t* img)
{
	XOJ_CHECK_TYPE(PdfCache);

	PdfCacheEntry* ne = new PdfCacheEntry(popplerPage, img);
	this->data = g_list_insert(this->data, ne, 0);
	int i = 0;
	for (GList* l = this->data; l != NULL; i++)
	{
		PdfCacheEntry* e = (PdfCacheEntry*) l->data;
		XOJ_CHECK_TYPE_OBJ(e, PdfCacheEntry);

		GList* le = l;

		l = l->next;

		if (i >= this->size)
		{
			delete e;
			this->data = g_list_delete_link(this->data, le);
		}
	}
}

void PdfCache::render(cairo_t* cr, XojPopplerPage* popplerPage, double zoom)
{
	XOJ_CHECK_TYPE(PdfCache);

	g_mutex_lock(&this->renderMutex);

	this->setZoom(zoom);

	cairo_surface_t* img = lookup(popplerPage);
	if (img == NULL)
	{
		img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                 popplerPage->getWidth() * this->zoom, popplerPage->getHeight() * this->zoom);
		cairo_t* cr2 = cairo_create(img);

		cairo_scale(cr2, this->zoom, this->zoom);
		popplerPage->render(cr2, false);
		cairo_destroy(cr2);
		cache(popplerPage, img);
	}

	cairo_matrix_t mOriginal;
	cairo_matrix_t mScaled;
	cairo_get_matrix(cr, &mOriginal);
	cairo_get_matrix(cr, &mScaled);
	mScaled.xx = 1;
	mScaled.yy = 1;
	mScaled.xy = 0;
	mScaled.yx = 0;
	cairo_set_matrix(cr, &mScaled);
	cairo_set_source_surface(cr, img, 0, 0);
	cairo_paint(cr);
	cairo_set_matrix(cr, &mOriginal);

	g_mutex_unlock(&this->renderMutex);
}
