#include "EraseableStrokePart.h"

EraseableStrokePart::EraseableStrokePart(Point a, Point b) {
	this->points = NULL;
	addPoint(a);
	addPoint(b);
	this->width = a.z;

	this->splitSize = 0;

	calcSize();
}

EraseableStrokePart::~EraseableStrokePart() {
	for (GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *) l->data;
		delete p;
	}
}

EraseableStrokePart::EraseableStrokePart(double width) {
	this->points = NULL;
	this->width = width;
	this->splitSize = 0;

	calcSize();
}

void EraseableStrokePart::calcSize() {
	if (this->points == NULL) {
		this->x = 0;
		this->y = 0;
		this->elementWidth = 0;
		this->elementHeight = 0;
		return;
	}

	double x1 = ((Point *) g_list_first(this->points)->data)->x;
	double y1 = ((Point *) g_list_first(this->points)->data)->y;
	double x2 = ((Point *) g_list_first(this->points)->data)->x;
	double y2 = ((Point *) g_list_first(this->points)->data)->y;

	for (GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *) l->data;
		x1 = MIN(x1, p->x);
		x2 = MAX(x2, p->x);
		y1 = MIN(y1, p->y);
		y2 = MAX(y2, p->y);
	}

	this->x = x1;
	this->y = y1;
	this->elementWidth = x2 - x1;
	this->elementHeight = y2 - y1;
}

EraseableStrokePart * EraseableStrokePart::clone() {
	EraseableStrokePart * part = new EraseableStrokePart(this->width);

	for (GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *) l->data;
		part->addPoint(*p);
	}

	part->splitSize = this->splitSize;

	return part;
}

double EraseableStrokePart::getX() {
	return this->x;
}

double EraseableStrokePart::getY() {
	return this->y;
}

double EraseableStrokePart::getElementWidth() {
	return this->elementWidth;
}

double EraseableStrokePart::getElementHeight() {
	return this->elementHeight;
}

void EraseableStrokePart::addPoint(Point p) {
	this->points = g_list_append(this->points, new Point(p));
}

double EraseableStrokePart::getWidth() {
	return this->width;
}

GList * EraseableStrokePart::getPoints() {
	return this->points;
}

void EraseableStrokePart::clearSplitData() {
	for (GList * l = this->points->next; l->next != NULL;) {
		Point * p = (Point *) l->data;
		delete p;
		GList * link = l;
		l = l->next;

		this->points = g_list_delete_link(this->points, link);
	}
}

void EraseableStrokePart::splitFor(double halfEraserSize) {
	if (halfEraserSize == this->splitSize) {
		return;
	}

	this->splitSize = halfEraserSize;

	Point * a = (Point *) g_list_first(this->points)->data;
	Point * b = (Point *) g_list_last(this->points)->data;

	// nothing to do, the size is enough small
	if (a->lineLengthTo(*b) <= halfEraserSize) {
		return;
	}

	clearSplitData();

	double len = a->lineLengthTo(*b);
	halfEraserSize /= 2;

	while (len > halfEraserSize) {
		this->points = g_list_insert(this->points, new Point(a->lineTo(*b, len)), 1);
		len -= halfEraserSize;
	}
}
