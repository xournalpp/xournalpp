#include "SplineHandler.h"

#include <cmath>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "undo/InsertUndoAction.h"
#include "util/cpp14memory.h"

/**
 * @brief A class to handle splines
 *
 * Drawing of a spline is started by a ButtonPressEvent. After every ButtonReleaseEvent,
 * a new knot point is added. The spline is finished through a ButtonDoublePressEvent,
 * the escape key or a ButtonPressEvent near the first knot. The latter event closes the spline.
 *
 * Splines segments can be linear or cubic (as in Inkscape). Join of two cubic segments
 * is supposed to be smooth. As for now only linear splines are implemented.
 */
SplineHandler::SplineHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        InputHandler(xournal, redrawable, page) {}

SplineHandler::~SplineHandler() = default;

void SplineHandler::draw(cairo_t* cr) {
    if (!stroke) {
        return;
    }

    double zoom = xournal->getZoom();
    double radius = radiusConst * zoom;
    if (xournal->getControl()->getToolHandler()->getDrawingType() != DRAWING_TYPE_SPLINE) {
        g_warning("Drawing type is not spline any longer");
        stroke = nullptr;
        xournal->getCursor()->updateCursor();
        return;
    }

    int dpiScaleFactor = xournal->getDpiScaleFactor();
    cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);

    // draw circles around knot points
    double lineWidth = 0.5 * zoom;
    int pointCount = stroke->getPointCount();
    cairo_set_line_width(cr, lineWidth);
    Point firstPoint = stroke->getPoint(0);
    Point currPoint = stroke->getPoint(pointCount - 1);
    double dist = currPoint.lineLengthTo(firstPoint);
    cairo_set_source_rgb(cr, 1, 0, 0);                               // use red color for first knot
    cairo_move_to(cr, firstPoint.x + radius, firstPoint.y);          // move to start point of circle arc;
    cairo_arc(cr, firstPoint.x, firstPoint.y, radius, 0, 2 * M_PI);  // draw circle
    if (dist < radius && pointCount > 3) {  // current point lies within the circle around the first knot
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);       // use gray color for all knots except first one
    for (size_t i = 1; i < pointCount - 1; i++) {  // dynamically changing knot is not circled
        Point p = stroke->getPoint(i);
        cairo_move_to(cr, p.x + radius, p.y);          // move to start point of circle arc;
        cairo_arc(cr, p.x, p.y, radius, 0, 2 * M_PI);  // draw circle
    }
    cairo_stroke(cr);

    // draw spline
    view.drawStroke(cr, stroke, 0);
}

auto SplineHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (!stroke) {
        return false;
    }
    int pointCount = stroke->getPointCount();
    double zoom = xournal->getZoom();
    double radius = radiusConst * zoom;

    switch (event->keyval) {
        case GDK_KEY_Escape: {
            if (pointCount > 1) {
                // remove dynamically changing point at cursor position
                stroke->deletePoint(pointCount - 1);
                this->redrawable->repaintRect(stroke->getX() - radius, stroke->getY() - radius,
                                              stroke->getElementWidth() + 2 * radius,
                                              stroke->getElementHeight() + 2 * radius);
            }
            finalizeSpline();
            return true;
        }
            /* TODO (Roland): Actions after backspace or key arrow
            case GDK_KEY_BackSpace: {
                if (pointCount > 2) {
                    // delete last non dynammically changing point
                }
                break;
            }
            case GDK_KEY_uparrow: {
                if (pointCount > 2) {
                    // move last non dynammically changing point up
                }
                break;
            }
            case GDK_KEY_downarrow: {
                if (pointCount > 2) {
                    // move last non dynammically changing point down
                }
                break;
            }
            case GDK_KEY_rightarrow: {
                if (pointCount > 2) {
                    // move last non dynammically changing point right
                }
                break;
            }
            case GDK_KEY_leftarrow: {
                if (pointCount > 2) {
                    // move last non dynammically changing point left
                }
                break;
            }
            */
    }
    return false;
}

auto SplineHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    if (!stroke) {
        return false;
    }

    double zoom = xournal->getZoom();
    double radius = radiusConst * zoom;
    int pointCount = stroke->getPointCount();

    Point currentPoint = Point(pos.x / zoom, pos.y / zoom);
    Rectangle rect = stroke->boundingRect();

    if (pointCount > 0) {
        if (!validMotion(currentPoint, stroke->getPoint(pointCount - 1))) {
            return true;
        }
    }

    this->redrawable->repaintRect(stroke->getX() - radius, stroke->getY() - radius,
                                  stroke->getElementWidth() + 2 * radius, stroke->getElementHeight() + 2 * radius);

    drawShape(currentPoint, pos);

    rect.add(stroke->boundingRect());
    double w = stroke->getWidth();

    redrawable->repaintRect(rect.x - w, rect.y - w, rect.width + 2 * w, rect.height + 2 * w);

    return true;
}

void SplineHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    if (!stroke) {
        return;
    }

    double zoom = xournal->getZoom();
    double radius = radiusConst * zoom;
    Point currPoint = Point(pos.x / zoom, pos.y / zoom);
    Point firstPoint = stroke->getPoint(0);
    double dist = currPoint.lineLengthTo(firstPoint);
    int pointCount = stroke->getPointCount();
    if (dist < radius && pointCount > 1) {
        stroke->setLastPoint(firstPoint);
        finalizeSpline();
    } else {
        stroke->addPoint(currPoint);
        this->redrawable->rerenderRect(currPoint.x - radius, currPoint.y - radius, 2 * radius, 2 * radius);
    }
}

void SplineHandler::onButtonPressEvent(const PositionInputData& pos) {
    double zoom = xournal->getZoom();
    if (!stroke) {
        createStroke(Point(pos.x / zoom, pos.y / zoom));
    }
}

void SplineHandler::onButtonDoublePressEvent(const PositionInputData& pos) { finalizeSpline(); }

void SplineHandler::finalizeSpline() {
    if (!stroke) {
        return;
    }

    Control* control = xournal->getControl();

    // This is not a valid stroke
    if (stroke->getPointCount() < 2) {
        g_warning("Stroke incomplete!");
        delete stroke;
        stroke = nullptr;
        return;
    }

    stroke->freeUnusedPointItems();
    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();


    undo->addUndoAction(mem::make_unique<InsertUndoAction>(page, layer, stroke));

    layer->addElement(stroke);
    page->fireElementChanged(stroke);  // calls draw method
    double zoom = xournal->getZoom();
    double radius = radiusConst * zoom;


    this->redrawable->rerenderRect(stroke->getX() - radius, stroke->getY() - radius,
                                   stroke->getElementWidth() + 2 * radius, stroke->getElementHeight() + 2 * radius);


    stroke = nullptr;

    xournal->getCursor()->updateCursor();
}

void SplineHandler::drawShape(Point& c, const PositionInputData& pos) {
    if (!stroke) {
        return;
    }

    int pointCount = stroke->getPointCount();
    if (pointCount > 1) {
        // remove dynamically changing point at (previous) cursor position
        stroke->deletePoint(pointCount - 1);
    }

    stroke->addPoint(c);
}