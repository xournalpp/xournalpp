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
        flipShift(flipShift),
        flipControl(flipControl),
        snappingHandler(xournal->getControl()->getSettings()) {}

BaseStrokeHandler::~BaseStrokeHandler() = default;

void BaseStrokeHandler::draw(cairo_t* cr) {
    double zoom = xournal->getZoom();
    int dpiScaleFactor = xournal->getDpiScaleFactor();

    cairo_scale(cr, zoom * dpiScaleFactor, zoom * dpiScaleFactor);

    std::lock_guard lock(this->strokeMutex);
    if (!stroke) {
        return;
    }

    view.drawStroke(cr, stroke.get(), 0);
}

void BaseStrokeHandler::updateStroke(const PositionInputData& pos) {
    Rectangle<double> rect;
    {
        std::lock_guard lock(this->strokeMutex);
        if (!stroke) {
            return;
        }
        rect = stroke->boundingRect();
    }
    auto shape = this->createShape(pos);
    {
        std::lock_guard lock(this->strokeMutex);
        stroke->swapPointVector(shape);
        rect.unite(stroke->boundingRect());
    }
    redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);
}

auto BaseStrokeHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (event->is_modifier) {
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

        this->updateStroke(pos);

        return true;
    }
    return false;
}

auto BaseStrokeHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {

    double zoom = xournal->getZoom();
    double x = pos.x / zoom;
    double y = pos.y / zoom;

    Point newPoint(x, y);
    if (!validMotion(newPoint, this->currPoint)) {
        return true;
    }
    this->currPoint = newPoint;

    this->updateStroke(pos);

    return true;
}

void BaseStrokeHandler::onMotionCancelEvent() {
    std::lock_guard lock(this->strokeMutex);
    stroke.reset();
}

void BaseStrokeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    xournal->getCursor()->activateDrawDirCursor(false);  // in case released within  fixate_Dir_Mods_Dist

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
                {
                    std::lock_guard lock(strokeMutex);
                    stroke.reset();
                }
                this->userTapped = true;

                BaseStrokeHandler::lastStrokeTime = pos.timestamp;

                xournal->getCursor()->updateCursor();

                return;
            }
        }
        BaseStrokeHandler::lastStrokeTime = pos.timestamp;
    }

    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    Stroke* s;
    {
        std::lock_guard lock(strokeMutex);
        s = stroke.release();
    }

    // This is not a valid stroke
    if (!s || s->getPointCount() < 2) {
        g_warning("BaseStrokeHandler::Stroke incomplete!");
        delete s;
        return;
    }

    s->freeUnusedPointItems();

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, s));

    layer->addElement(s);
    page->fireElementChanged(s);

    xournal->getCursor()->updateCursor();
}

void BaseStrokeHandler::onButtonPressEvent(const PositionInputData& pos) {
    double zoom = xournal->getZoom();
    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;
    this->startStrokeTime = pos.timestamp;
    this->startPoint = snappingHandler.snapToGrid(this->buttonDownPoint, pos.isAltDown());

    auto s = createStroke(startPoint, this->xournal->getControl());
    s->addPoint(startPoint);  // Second copy of the point, for rendering

    std::lock_guard lock(strokeMutex);
    std::swap(stroke, s);
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
