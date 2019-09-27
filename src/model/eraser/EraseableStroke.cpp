#include "EraseableStroke.h"

#include "EraseableStrokePart.h"
#include "PartList.h"
#include "model/Stroke.h"

#include <Range.h>

#include <cmath>

EraseableStroke::EraseableStroke(Stroke* stroke)
 : stroke(stroke)
{
	this->parts = new PartList();
	g_mutex_init(&this->partLock);

	for (int i = 1; i < stroke->getPointCount(); i++)
	{
		this->parts->add(new EraseableStrokePart(stroke->getPoint(i - 1), stroke->getPoint(i)));
	}
}

EraseableStroke::~EraseableStroke()
{
	delete this->parts;
	this->parts = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void EraseableStroke::draw(cairo_t* cr)
{
	Point lastPoint;

	g_mutex_lock(&this->partLock);
	PartList * tmpCopy = this->parts->clone();
	g_mutex_unlock(&this->partLock);

	double w = this->stroke->getWidth();

	for (GList* l = tmpCopy->data; l != nullptr; l = l->next)
	{
		EraseableStrokePart* part = (EraseableStrokePart*) l->data;
		if (part->getWidth() == Point::NO_PRESSURE)
		{
			cairo_set_line_width(cr, w);
		}
		else
		{
			cairo_set_line_width(cr, part->getWidth());
		}

		GList* pl = part->getPoints();
		Point* p = (Point*) pl->data;
		cairo_move_to(cr, p->x, p->y);

		pl = pl->next;
		for (; pl != nullptr; pl = pl->next)
		{
			Point* p = (Point*) pl->data;
			cairo_line_to(cr, p->x, p->y);
		}
		cairo_stroke(cr);
	}

	delete tmpCopy;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The only public method
 */
Range* EraseableStroke::erase(double x, double y, double halfEraserSize, Range* range)
{
	this->repaintRect = range;

	g_mutex_lock(&this->partLock);
	PartList* tmpCopy = this->parts->clone();
	g_mutex_unlock(&this->partLock);

	for (GList* l = tmpCopy->data; l != nullptr;)
	{
		EraseableStrokePart* p = (EraseableStrokePart*) l->data;
		l = l->next;
		erase(x, y, halfEraserSize, p, tmpCopy);
	}

	g_mutex_lock(&this->partLock);
	PartList* old = this->parts;
	this->parts = tmpCopy;
	g_mutex_unlock(&this->partLock);

	delete old;

	return this->repaintRect;
}

void EraseableStroke::addRepaintRect(double x, double y, double width, double height)
{
	if (this->repaintRect)
	{
		this->repaintRect->addPoint(x, y);
	}
	else
	{
		this->repaintRect = new Range(x, y);
	}

	this->repaintRect->addPoint(x + width, y + height);
}

void EraseableStroke::erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list)
{
	if (part->points->next == nullptr)
	{
		return;
	}

	Point eraser(x, y);

	Point* a = (Point*) g_list_first(part->points)->data;
	Point* b = (Point*) g_list_last(part->points)->data;

	if (eraser.lineLengthTo(*a) < halfEraserSize * 1.2 && eraser.lineLengthTo(*b) < halfEraserSize * 1.2)
	{
		list->data = g_list_remove(list->data, part);
		addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());

		delete part;
		return;
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
	if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2)
	{
		bool deleteAfter = false;

		if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter))
		{
			addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
			part->calcSize();
		}

		if (deleteAfter)
		{
			delete part;
		}

		return;
	}

	// check last point
	if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2)
	{
		bool deleteAfter = false;

		if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter))
		{
			addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
			part->calcSize();
		}

		if (deleteAfter)
		{
			delete part;
		}

		return;
	}

	double len = hypot(bX - aX, bY - aY);
	/**
	 * The normale to a vector, the padding to a point
	 */
	double p = std::abs((x - aX) * (aY - bY) + (y - aY) * (bX - aX)) / hypot(aX - x, aY - y);

	// The space to the line is in the range, but it can also be parallel
	// and not enough close, so calculate a "circle" with the center on the
	// center of the line

	if (p <= halfEraserSize)
	{
		double centerX = (aX + x) / 2;
		double centerY = (aY + y) / 2;
		double distance = hypot(x - centerX, y - centerY);

		// we should calculate the length of the line within the rectangle, to find out
		// the distance from the border to the point, but the stroke are not rectangular
		// so we can do it simpler
		distance -= hypot((x2 - x1) / 2, (y2 - y1) / 2);

		if (distance <= (len / 2) + 0.1)
		{
			bool deleteAfter = false;

			if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter))
			{
				addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
				part->calcSize();
			}

			if (deleteAfter)
			{
				delete part;
			}

			return;
		}
	}
}

bool EraseableStroke::erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part,
								PartList* list, bool* deleteStrokeAfter)
{
	bool changed = false;

	part->splitFor(halfEraserSize);

	double x1 = x - halfEraserSize;
	double x2 = x + halfEraserSize;
	double y1 = y - halfEraserSize;
	double y2 = y + halfEraserSize;

	/**
	 * erase the beginning
	 */
	for (GList* l = part->getPoints(); l != nullptr;)
	{
		Point* p = (Point*) l->data;
		l = l->next;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2)
		{
			part->points = g_list_remove(part->points, p);
			delete p;
			changed = true;
		}
		else
		{
			// only the beginning is handled here
			break;
		}
	}

	/**
	 * erase the end
	 */
	for (GList* l = g_list_last(part->getPoints()); l != nullptr;)
	{
		Point* p = (Point*) l->data;
		l = l->prev;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2)
		{
			part->points = g_list_remove(part->points, p);
			delete p;
			changed = true;
		}
		else
		{
			// only the end is handled here
			break;
		}
	}

	/**
	 * handle the rest
	 */

	GList* lists = nullptr;
	GList* current = nullptr;

	for (GList* l = part->points; l != nullptr;)
	{
		Point* p = (Point*) l->data;
		l = l->next;
		if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2)
		{
			part->points = g_list_remove(part->points, p);
			delete p;
			if (current)
			{
				lists = g_list_append(lists, current);
				current = nullptr;
			}
			changed = true;
		}
		else
		{
			current = g_list_append(current, p);
		}
	}

	if (current)
	{
		lists = g_list_append(lists, current);
		current = nullptr;
	}

	g_list_free(part->points);
	part->points = nullptr;
	if (lists)
	{
		part->points = (GList*) lists->data;
		lists = g_list_delete_link(lists, lists);

		int pos = g_list_index(list->data, part) + 1;
		
		// create data structure for all new (splitted) parts
		for (GList* l = lists; l != nullptr; l = l->next)
		{
			EraseableStrokePart* newPart = new EraseableStrokePart(part->width);
			newPart->points = (GList*) l->data;
			list->data = g_list_insert(list->data, newPart, pos++);
		}

		g_list_free(lists);
	}
	else
	{
		// no parts, all deleted
		list->data = g_list_remove(list->data, part);
		*deleteStrokeAfter = true;
	}

	return changed;
}

GList* EraseableStroke::getStroke(Stroke* original)
{
	GList* list = nullptr;

	Stroke* s = nullptr;
	Point lastPoint(NAN, NAN);
	for (GList* l = this->parts->data; l != nullptr; l = l->next)
	{
		EraseableStrokePart* p = (EraseableStrokePart*) l->data;
		GList* points = p->getPoints();
		if (g_list_length(points) < 2)
		{
			continue;
		}

		Point a = *((Point*) g_list_first(points)->data);
		Point b = *((Point*) g_list_last(points)->data);
		a.z = p->width;

		if (!lastPoint.equalsPos(a) || s == nullptr)
		{
			if (s)
			{
				s->addPoint(lastPoint);
			}
			s = new Stroke();
			s->setColor(original->getColor());
			s->setToolType(original->getToolType());
			s->setLineStyle(original->getLineStyle());
			s->setWidth(original->getWidth());
			list = g_list_append(list, s);
		}
		s->addPoint(a);
		lastPoint = b;
	}
	if (s)
	{
		s->addPoint(lastPoint);
	}

	return list;
}
