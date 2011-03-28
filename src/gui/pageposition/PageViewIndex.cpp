#include "PageViewIndex.h"
#include "PagePosition.h"
#include "../PageView.h"

class PageViewIndexEntry {
public:
	PageViewIndexEntry(PageView * view, int area) {
		XOJ_INIT_TYPE(PageViewIndexEntry);

		this->view = view;
		this->area = area;
	}

	~PageViewIndexEntry() {
		XOJ_CHECK_TYPE(PageViewIndexEntry);

		this->view = NULL;

		XOJ_RELEASE_TYPE(PageViewIndexEntry);
	}


public:
	XOJ_TYPE_ATTRIB;

	PageView * view;
	int area;
};

PageViewIndex::PageViewIndex(int x, int y, int width, int height) {
	XOJ_INIT_TYPE(PageViewIndex);

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;

	this->data = NULL;
}

PageViewIndex::~PageViewIndex() {
	XOJ_RELEASE_TYPE(PageViewIndex);

	for(GList * l = this->data; l!=NULL;l=l->next) {
		PageViewIndexEntry * e = (PageViewIndexEntry *)l->data;
		delete e;
	}

	g_list_free(this->data);
	this->data = NULL;
}

void PageViewIndex::addView(PageView * v) {
	GdkRectangle r1;
	GdkRectangle r2;
	GdkRectangle r3;

	r1.x = v->getX();
	r1.y = v->getY();
	r1.width = v->getDisplayWidth();
	r1.height = v->getDisplayHeight();

	r2.x = this->x;
	r2.y = this->y;
	r2.width = this->width;
	r2.height = this->height;

	gdk_rectangle_intersect(&r1, &r2, &r3);

	int area = r3.width * r3.height;

	this->data = g_list_append(this->data, new PageViewIndexEntry(v, area));
}

void PageViewIndex::add(PagePosition * pp, int y) {
	XOJ_CHECK_TYPE(PageViewIndex);

	PageView * v1 = pp->getViewAt(this->x, y);
	PageView * v2 = pp->getViewAt(this->x + this->width, y);

	if(v1 != NULL) {
		addView(v1);
	}
	if(v2 != NULL && v1 != v2) {
		addView(v2);
	}
}

PageView * PageViewIndex::getHighestIntersects() {
	XOJ_CHECK_TYPE(PageViewIndex);

	PageViewIndexEntry * b = NULL;

	for(GList * l = this->data; l != NULL;l = l->next) {
		PageViewIndexEntry * e = (PageViewIndexEntry *)l->data;

		if(b == NULL || b->area < e->area) {
			b = e;
		}
	}

	if(b != NULL) {
		return b->view;
	}

	return NULL;
}

