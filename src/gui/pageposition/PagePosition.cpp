#include "PagePosition.h"

#include "../PageView.h"

PagePosition::PagePosition(PageView * pv) {
	this->y1 = pv->getY();
	this->y2 = this->y1 + pv->getDisplayHeight();

	this->views = g_list_append(NULL, pv);
}

PagePosition::PagePosition() {
	this->y1 = 0;
	this->y2 = 0;

	this->views = NULL;
}

PagePosition::~PagePosition() {
	g_list_free(this->views);
}

bool PagePosition::add(PageView * pv) {
	int y1 = pv->getY();
	int y2 = this->y1 + pv->getDisplayHeight();

	if (containsY(y1) || containsY(y2) || pv->containsY(this->y1) || pv->containsY(this->y2)) {
		this->views = g_list_append(this->views, pv);

		this->y1 = MIN(this->y1, y1);
		this->y2 = MAX(this->y2, y2);

		return true;
	}
	return false;
}

PageView * PagePosition::getViewAt(int x, int y) {
	for (GList * l = this->views; l != NULL; l = l->next) {
		PageView * v = (PageView *) l->data;
		if (v->containsPoint(x, y)) {
			return v;
		}
	}
	return NULL;
}

bool PagePosition::containsY(int y) {
	return (y >= this->y1 && y <= this->y2);
}

bool PagePosition::isYSmallerThan(int y) {
	return y > this->y2;
}

bool PagePosition::isYGraterThan(int y) {
	return y < this->y1;
}

