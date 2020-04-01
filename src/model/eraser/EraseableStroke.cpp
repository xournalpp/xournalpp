#include "EraseableStroke.h"

#include <cmath>

#include "model/Stroke.h"

#include "EraseableStrokePart.h"
#include "PartList.h"
#include "Range.h"

EraseableStroke::EraseableStroke(Stroke* stroke): stroke(stroke) {
    this->parts = new PartList();
    g_mutex_init(&this->partLock);

    for (int i = 1; i < stroke->getPointCount(); i++) {
        this->parts->add(new EraseableStrokePart(stroke->getPoint(i - 1), stroke->getPoint(i)));
    }
}

EraseableStroke::~EraseableStroke() {
    delete this->parts;
    this->parts = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void EraseableStroke::draw(cairo_t* cr) {
    g_mutex_lock(&this->partLock);
    PartList* tmpCopy = this->parts->clone();
    g_mutex_unlock(&this->partLock);

    double w = this->stroke->getWidth();

    for (GList* l = tmpCopy->data; l != nullptr; l = l->next) {
        auto* part = static_cast<EraseableStrokePart*>(l->data);
        if (part->getWidth() == Point::NO_PRESSURE) {
            cairo_set_line_width(cr, w);
        } else {
            cairo_set_line_width(cr, part->getWidth());
        }

        GList* pl = part->getPoints();
        auto* p = static_cast<Point*>(pl->data);
        cairo_move_to(cr, p->x, p->y);

        pl = pl->next;
        for (; pl != nullptr; pl = pl->next) {
            auto* p = static_cast<Point*>(pl->data);
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
auto EraseableStroke::erase(double x, double y, double halfEraserSize, Range* range) -> Range* {
    this->repaintRect = range;

    g_mutex_lock(&this->partLock);
    PartList* tmpCopy = this->parts->clone();
    g_mutex_unlock(&this->partLock);

    for (GList* l = tmpCopy->data; l != nullptr;) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
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

void EraseableStroke::addRepaintRect(double x, double y, double width, double height) {
    if (this->repaintRect) {
        this->repaintRect->addPoint(x, y);
    } else {
        this->repaintRect = new Range(x, y);
    }

    this->repaintRect->addPoint(x + width, y + height);
}

void EraseableStroke::erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list) {
    if (part->points->next == nullptr) {
        return;
    }

    Point eraser(x, y);

    auto* a = static_cast<Point*>(g_list_first(part->points)->data);
    auto* b = static_cast<Point*>(g_list_last(part->points)->data);

    if (eraser.lineLengthTo(*a) < halfEraserSize * 1.2 && eraser.lineLengthTo(*b) < halfEraserSize * 1.2) {
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
    if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
    }

    // check last point
    if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
    }

    double len = hypot(bX - aX, bY - aY);
    /**
     * The distance of the center of the eraser box to the line passing through (aX, aY) and (bX, bY)
     */
    double p = std::abs((x - aX) * (aY - bY) + (y - aY) * (bX - aX)) / len;

    // If the distance p of the center of the eraser box to the (full) line is in the range,
    // we check whether the eraser box is not too far from the line segment through the two points.

    if (p <= halfEraserSize) {
        double centerX = (aX + bX) / 2;
        double centerY = (aY + bY) / 2;
        double distance = hypot(x - centerX, y - centerY);

        // For the above check we imagine a circle whose center is the mid point of the two points of the stroke
        // and whose radius is half the length of the line segment plus half the diameter of the eraser box
        // plus some small padding
        // If the center of the eraser box lies within that circle then we consider it to be close enough

        distance -= halfEraserSize * std::sqrt(2);

        constexpr double PADDING = 0.1;

        if (distance <= len / 2 + PADDING) {
            bool deleteAfter = false;

            if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
                addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
                part->calcSize();
            }

            if (deleteAfter) {
                delete part;
            }

            return;
        }
    }
}

auto EraseableStroke::erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list,
                                bool* deleteStrokeAfter) -> bool {
    bool changed = false;

    part->splitFor(halfEraserSize);

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    /**
     * erase the beginning
     */
    for (GList* l = part->getPoints(); l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
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
    for (GList* l = g_list_last(part->getPoints()); l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
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

    GList* lists = nullptr;
    GList* current = nullptr;

    for (GList* l = part->points; l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
        l = l->next;
        if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
            part->points = g_list_remove(part->points, p);
            delete p;
            if (current) {
                lists = g_list_append(lists, current);
                current = nullptr;
            }
            changed = true;
        } else {
            current = g_list_append(current, p);
        }
    }

    if (current) {
        lists = g_list_append(lists, current);
        current = nullptr;
    }

    g_list_free(part->points);
    part->points = nullptr;
    if (lists) {
        part->points = static_cast<GList*>(lists->data);
        lists = g_list_delete_link(lists, lists);

        int pos = g_list_index(list->data, part) + 1;

        // create data structure for all new (splitted) parts
        for (GList* l = lists; l != nullptr; l = l->next) {
            auto* newPart = new EraseableStrokePart(part->width);
            newPart->points = static_cast<GList*>(l->data);
            list->data = g_list_insert(list->data, newPart, pos++);
        }

        g_list_free(lists);
    } else {
        // no parts, all deleted
        list->data = g_list_remove(list->data, part);
        *deleteStrokeAfter = true;
    }

    return changed;
}

auto EraseableStroke::getStroke(Stroke* original) -> GList* {
    GList* list = nullptr;

    Stroke* s = nullptr;
    Point lastPoint(NAN, NAN);
    for (GList* l = this->parts->data; l != nullptr; l = l->next) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        GList* points = p->getPoints();
        if (g_list_length(points) < 2) {
            continue;
        }

        Point a = *(static_cast<Point*>(g_list_first(points)->data));
        Point b = *(static_cast<Point*>(g_list_last(points)->data));
        a.z = p->width;

        if (!lastPoint.equalsPos(a) || s == nullptr) {
            if (s) {
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
    if (s) {
        s->addPoint(lastPoint);
    }

    return list;
}
