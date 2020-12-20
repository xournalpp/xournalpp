#include "BaseStrokeHandler.h"

#include <cmath>
#include <memory>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "undo/InsertUndoAction.h"


guint32 BaseStrokeHandler::lastStrokeTime;  // persist for next stroke


BaseStrokeHandler::BaseStrokeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift,
                                     bool flipControl):
        InputHandler(xournal, redrawable, page),
        snappingHandler(xournal->getControl()->getSettings()),
        flipShift(flipShift),
        flipControl(flipControl) {}

BaseStrokeHandler::~BaseStrokeHandler() = default;

void BaseStrokeHandler::draw(cairo_t* cr) {
    double zoom = xournal->getZoom();
    int dpiScaleFactor = xournal->getDpiScaleFactor();

    cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);
    view.drawStroke(cr, stroke, 0);
}

auto BaseStrokeHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (event->is_modifier) {
        Rectangle<double> rect = stroke->boundingRect();

        PositionInputData pos{};
        pos.x = pos.y = pos.pressure = 0;  // not used in redraw
        if (event->keyval == GDK_KEY_Shift_L || event->keyval == GDK_KEY_Shift_R) {
            pos.state = static_cast<GdkModifierType>(
                    event->state ^ GDK_SHIFT_MASK);  // event state does not include current this modifier keypress - so
                                                     // ^toggle will work for press and release.
        } else if (event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) {
            pos.state = static_cast<GdkModifierType>(event->state ^ GDK_CONTROL_MASK);
        } else if (event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R) {
            pos.state = static_cast<GdkModifierType>(event->state ^ GDK_MOD1_MASK);
        } else {
            return false;
        }

        this->redrawable->repaintRect(stroke->getX(), stroke->getY(), stroke->getElementWidth(),
                                      stroke->getElementHeight());


        Point malleablePoint = this->currPoint;  // make a copy as it might get snapped to grid.
        this->drawShape(malleablePoint, pos);


        rect.unite(stroke->boundingRect());

        double w = stroke->getWidth();
        redrawable->repaintRect(rect.x - w, rect.y - w, rect.width + 2 * w, rect.height + 2 * w);

        return true;
    }
    return false;
}

auto BaseStrokeHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
    if (!stroke) {
        return false;
    }

    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;
    int pointCount = stroke->getPointCount();

    Point currentPoint(x, y);
    Rectangle<double> rect = stroke->boundingRect();

    if (pointCount > 0) {
        if (!validMotion(currentPoint, stroke->getPoint(pointCount - 1))) {
            return true;
        }
    }

    this->redrawable->repaintRect(stroke->getX(), stroke->getY(), stroke->getElementWidth(),
                                  stroke->getElementHeight());

    drawShape(currentPoint, pos);

    rect.unite(stroke->boundingRect());
    double w = stroke->getWidth();

    redrawable->repaintRect(rect.x - w, rect.y - w, rect.width + 2 * w, rect.height + 2 * w);

    return true;
}

void BaseStrokeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    xournal->getCursor()->activateDrawDirCursor(false);  // in case released within  fixate_Dir_Mods_Dist

    if (stroke == nullptr) {
        return;
    }


    Control* control = xournal->getControl();
    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For simple strokes see StrokeHandler which has a slightly
                                             // different version of this filter.  See //!
    {
        int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
        double strokeFilterIgnoreLength = NAN;

        settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
        double dpmm = settings->getDisplayDpi() / 25.4;

        double zoom = xournal->getZoom();
        double lengthSqrd = (pow(((pos.x / zoom) - (this->buttonDownPoint.x)), 2) +
                             pow(((pos.y / zoom) - (this->buttonDownPoint.y)), 2)) *
                            pow(xournal->getZoom(), 2);

        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
            pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
            if (pos.timestamp - BaseStrokeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here.
                delete stroke;
                stroke = nullptr;
                this->userTapped = true;

                BaseStrokeHandler::lastStrokeTime = pos.timestamp;

                xournal->getCursor()->updateCursor();

                return;
            }
        }
        BaseStrokeHandler::lastStrokeTime = pos.timestamp;
    }


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

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke));

    layer->addElement(stroke);
    page->fireElementChanged(stroke);

    stroke = nullptr;

    xournal->getCursor()->updateCursor();
}

void BaseStrokeHandler::onButtonPressEvent(const PositionInputData& pos) {
    double zoom = xournal->getZoom();
    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    if (!stroke) {
        createStroke(Point(this->buttonDownPoint.x, this->buttonDownPoint.y));
    }

    this->startStrokeTime = pos.timestamp;
}

void BaseStrokeHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
    // nothing to do
}

void BaseStrokeHandler::modifyModifiersByDrawDir(double width, double height, bool changeCursor) {
    bool gestureShift = this->flipShift;
    bool gestureControl = this->flipControl;

    if (this->drawModifierFixed ==
        NONE) {  // User hasn't dragged out past DrawDirModsRadius  i.e. modifier not yet locked.
        gestureShift = (width < 0) != gestureShift;
        gestureControl = (height < 0) != gestureControl;

        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;

        double zoom = xournal->getZoom();
        double fixate_Dir_Mods_Dist =
                std::pow(xournal->getControl()->getSettings()->getDrawDirModsRadius() / zoom, 2.0);
        if (std::pow(width, 2.0) > fixate_Dir_Mods_Dist || std::pow(height, 2.0) > fixate_Dir_Mods_Dist) {
            this->drawModifierFixed = static_cast<DIRSET_MODIFIERS>(SET | (gestureShift ? SHIFT : NONE) |
                                                                    (gestureControl ? CONTROL : NONE));
            if (changeCursor) {
                xournal->getCursor()->activateDrawDirCursor(false);
            }
        } else {
            if (changeCursor) {
                xournal->getCursor()->activateDrawDirCursor(true, this->modShift, this->modControl);
            }
        }
    } else {
        gestureShift = gestureShift == !(this->drawModifierFixed & SHIFT);
        gestureControl = gestureControl == !(this->drawModifierFixed & CONTROL);
        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;
    }
}
