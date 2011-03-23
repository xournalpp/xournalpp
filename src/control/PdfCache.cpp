#include "PdfCache.h"
#include <stdio.h>
// TODO: AA: type check

class PdfCacheEntry {
public:
	PdfCacheEntry(XojPopplerPage * popplerPage, cairo_surface_t * img) {
		this->popplerPage = popplerPage;
		this->rendered = img;
	}
	~PdfCacheEntry() {
		this->popplerPage = NULL;
		cairo_surface_destroy(this->rendered);
		this->rendered = NULL;
	}

	XojPopplerPage * popplerPage;
	cairo_surface_t * rendered;
};

PdfCache::PdfCache(int size) {
	this->data = NULL;
	this->size = size;
	this->zoom = -1;

	this->renderMutex = g_mutex_new();
}

PdfCache::~PdfCache() {
	clearCache();
	this->size = 0;

	g_mutex_free(this->renderMutex);
	this->renderMutex = NULL;
}

void PdfCache::setZoom(double zoom) {
	if (this->zoom == zoom) {
		return;
	}
	this->zoom = zoom;

	clearCache();
}

void PdfCache::clearCache() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfCacheEntry * e = (PdfCacheEntry *) l->data;
		delete e;
	}
	g_list_free(this->data);
	this->data = NULL;
}

cairo_surface_t * PdfCache::lookup(XojPopplerPage * popplerPage) {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfCacheEntry * e = (PdfCacheEntry *) l->data;
		if (e->popplerPage == popplerPage) {
			return e->rendered;
		}
	}

	return NULL;
}

void PdfCache::cache(XojPopplerPage * popplerPage, cairo_surface_t * img) {
	PdfCacheEntry * ne = new PdfCacheEntry(popplerPage, img);
	this->data = g_list_insert(this->data, ne, 0);
	int i = 0;
	for (GList * l = this->data; l != NULL; i++) {
		PdfCacheEntry * e = (PdfCacheEntry *) l->data;

		GList * le = l;

		l = l->next;

		if (i >= this->size) {
			delete e;
			this->data = g_list_delete_link(this->data, le);
		}
	}
}

void PdfCache::render(cairo_t * cr, XojPopplerPage * popplerPage, double zoom) {
	g_mutex_lock(this->renderMutex);

	this->setZoom(zoom);

	cairo_surface_t * img = lookup(popplerPage);
	if (img == NULL) {
		img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, popplerPage->getWidth() * this->zoom, popplerPage->getHeight() * this->zoom);
		cairo_t * cr2 = cairo_create(img);

		cairo_scale(cr2, this->zoom, this->zoom);
		popplerPage->render(cr2, false);
		cairo_destroy(cr2);
		cache(popplerPage, img);
	}

	assert(this->zoom > 0);

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

	g_mutex_unlock(this->renderMutex);
}
