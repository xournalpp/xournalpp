#include "EraseableStroke.h"
#include "Stroke.h"
#include <math.h>

EraseableStroke::EraseableStroke(Stroke * stroke) {
	this->parts = NULL;

	for (int i = 1; i < stroke->getPointCount(); i++) {
		this->parts = g_list_append(this->parts, new EraseableStrokePart(stroke->getPoint(i - 1), stroke->getPoint(i)));
	}
}

EraseableStroke::~EraseableStroke() {
	for (GList * l = this->parts; l != NULL; l = l->next) {
		EraseableStrokePart * p = (EraseableStrokePart *) l->data;
		delete p;
	}
}

bool EraseableStroke::erasePart(double x, double y, double halfEraserSize, EraseableStrokePart * part) {
	bool changed = false;

	part->splitFor(halfEraserSize);

	double x1 = x - halfEraserSize;
	double x2 = x + halfEraserSize;
	double y1 = y - halfEraserSize;
	double y2 = y + halfEraserSize;

	/**
	 * erase the beginning
	 */
	for (GList * l = part->getPoints(); l != NULL;) {
		Point * p = (Point *) l->data;
		l = l->next;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
			part->points = g_list_remove(part->points, p);
			delete p;
			changed = true;
		} else {
			// only the beginning is handled here
			break;
		}
	}

	/**
	 * erase the end
	 */
	for (GList * l = g_list_last(part->getPoints()); l != NULL;) {
		Point * p = (Point *) l->data;
		l = l->prev;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
			part->points = g_list_remove(part->points, p);
			delete p;
			changed = true;
		} else {
			// only the end is handled here
			break;
		}
	}

	/**
	 * handle the rest
	 */

	GList * lists = NULL;
	GList * current = NULL;

	for (GList * l = part->points; l != NULL;) {
		Point * p = (Point *) l->data;
		l = l->next;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
			part->points = g_list_remove(part->points, p);
			delete p;
			if (current) {
				lists = g_list_append(lists, current);
				current = NULL;
			}
			changed = true;
		} else {
			current = g_list_append(current, p);
		}
	}

	if (current) {
		lists = g_list_append(lists, current);
		current = NULL;
	}

	g_list_free(part->points);
	part->points = NULL;
	if (lists) {
		part->points = (GList *) lists->data;
		lists = g_list_delete_link(lists, lists);

		int pos = g_list_index(this->parts, part) + 1;

		// create data structure for all new (splitted) parts
		for (GList * l = lists; l != NULL; l = l->next) {
			EraseableStrokePart * newPart = new EraseableStrokePart(part->width);
			newPart->points = (GList *) l->data;
			this->parts = g_list_insert(this->parts, newPart, pos++);
		}

		g_list_free(lists);
	} else {
		// no parts, all deleted
		this->parts = g_list_remove(this->parts, part);
		delete part;
	}

	return changed;
}

bool EraseableStroke::erase(double x, double y, double halfEraserSize, EraseableStrokePart * part) {
	if (part->points->next == NULL) {
		return false;
	}

	Point eraser(x, y);

	Point * a = (Point *) g_list_first(part->points)->data;
	Point * b = (Point *) g_list_last(part->points)->data;

	if (eraser.lineLengthTo(*a) < halfEraserSize * 1.2 && eraser.lineLengthTo(*b) < halfEraserSize * 1.2) {
		this->parts = g_list_remove(this->parts, part);
		delete part;
		return true;
	}

	double x1 = x - halfEraserSize;
	double x2 = x + halfEraserSize;
	double y1 = y - halfEraserSize;
	double y2 = y + halfEraserSize;

	double aX = a->x;
	double aY = a->y;
	double bX = b->x;
	double bY = b->y;

	// check first point
	if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2) {
		return erasePart(x, y, halfEraserSize, part);
	}

	// check last point
	if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2) {
		return erasePart(x, y, halfEraserSize, part);
	}

	double len = hypot(bX - aX, bY - aY);
	/**
	 * The normale to a vector, the padding to a point
	 */
	double p = ABS((x - aX) * (aY - bY) + (y - aY) * (bX - aX)) / hypot(aX - x, aY - y);

	// The space to the line is in the range, but it can also be parallel
	// and not enough close, so calculate a "circle" with the center on the
	// center of the line

	if (p <= halfEraserSize) {
		double centerX = (aX + x) / 2;
		double centerY = (aY + y) / 2;
		double distance = hypot(x - centerX, y - centerY);

		// we should calculate the length of the line within the rectangle, to find out
		// the distance from the border to the point, but the stroke are not rectangular
		// so we can do it simpler
		distance -= hypot((x2 - x1) / 2, (y2 - y1) / 2);

		if (distance <= (len / 2) + 0.1) {
			return erasePart(x, y, halfEraserSize, part);
		}
	}

	return false;
}

bool EraseableStroke::erase(double x, double y, double halfEraserSize) {
	bool repaint = false;
	for (GList * l = this->parts; l != NULL;) {
		EraseableStrokePart * p = (EraseableStrokePart *) l->data;
		l = l->next;
		repaint |= erase(x, y, halfEraserSize, p);
	}

	return repaint;
}

GList * EraseableStroke::getParts() {
	return this->parts;
}

GList * EraseableStroke::getStroke(Stroke * original) {
	GList * list = NULL;

	Stroke * s = NULL;
	Point lastPoint(NAN, NAN);
	for (GList * l = this->parts; l != NULL; l = l->next) {
		EraseableStrokePart * p = (EraseableStrokePart *) l->data;
		GList * points = p->getPoints();
		if (g_list_length(points) < 2) {
			continue;
		}

		Point a = *((Point *) g_list_first(points)->data);
		Point b = *((Point *) g_list_last(points)->data);
		a.z = p->width;

		if (!lastPoint.equalsPos(a) || s == NULL) {
			if (s) {
				s->addPoint(lastPoint);
			}
			s = new Stroke();
			s->setColor(original->getColor());
			s->setToolType(original->getToolType());
			s->setWidth(original->getWidth());
			list = g_list_append(list, s);
		}
		s->addPoint(a);
		lastPoint = b;
	}
	if (s) {
		s->addPoint(lastPoint);
	}

	return list;
}

EraseableStrokePart::EraseableStrokePart(Point a, Point b) {
	this->points = NULL;
	addPoint(a);
	addPoint(b);
	this->width = a.z;

	this->splitSize = 0;
}

EraseableStrokePart::EraseableStrokePart(double width) {
	this->points = NULL;
	this->width = width;
	this->splitSize = 0;
}

EraseableStrokePart::~EraseableStrokePart() {
	for (GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *) l->data;
		delete p;
	}
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
