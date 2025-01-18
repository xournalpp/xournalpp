#include "BaseShapeHandler.h"

#include <cmath>   // for pow, NAN
#include <memory>  // for make_unique, __share...

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Alt_L, GDK_K...

#include "control/Control.h"                       // for Control
#include "control/layer/LayerController.h"         // for LayerController
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/InputHandler.h"            // for InputHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHandler
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Document.h"                        // for Document
#include "model/Layer.h"                           // for Layer
#include "model/Stroke.h"                          // for Stroke
#include "model/XojPage.h"                         // for XojPage
#include "undo/InsertUndoAction.h"                 // for InsertUndoAction
#include "undo/UndoRedoHandler.h"                  // for UndoRedoHandler
#include "util/Assert.h"                           // for xoj_assert
#include "util/DispatchPool.h"                     // for DispatchPool
#include "view/overlays/ShapeToolView.h"           // for ShapeToolView


BaseShapeHandler::BaseShapeHandler(Control* control, const PageRef& page, bool flipShift, bool flipControl):
        InputHandler(control, page),
        flipShift(flipShift),
        flipControl(flipControl),
        snappingHandler(control->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::ShapeToolView>>()) {}

BaseShapeHandler::~BaseShapeHandler() = default;

void BaseShapeHandler::updateShape(bool isAltDown, bool isShiftDown, bool isControlDown) {
    auto [shape, rg] = this->createShape(isAltDown, isShiftDown, isControlDown);
    std::swap(shape, this->shape);
    Range repaintRange = rg.unite(lastSnappingRange);
    lastSnappingRange = rg;
    repaintRange.addPadding(0.5 * this->stroke->getWidth());
    viewPool->dispatch(xoj::view::ShapeToolView::FLAG_DIRTY_REGION, repaintRange);
}

void BaseShapeHandler::cancelStroke() {
    this->shape.clear();
    Range repaintRange = this->lastSnappingRange;
    repaintRange.addPadding(0.5 * this->stroke->getWidth());
    this->viewPool->dispatchAndClear(xoj::view::ShapeToolView::FINALIZATION_REQUEST, repaintRange);
    this->lastSnappingRange = Range();
}

auto BaseShapeHandler::onKeyEvent(const KeyEvent& event, bool pressed) -> bool {
    bool isAltDown = event.state & GDK_MOD1_MASK;
    bool isShiftDown = event.state & GDK_SHIFT_MASK;
    bool isControlDown = event.state & GDK_CONTROL_MASK;
    // event->state contains the modifiers' states BEFORE the current event
    if (event.keyval == GDK_KEY_Shift_L || event.keyval == GDK_KEY_Shift_R) {
        isShiftDown = pressed;
    } else if (event.keyval == GDK_KEY_Control_L || event.keyval == GDK_KEY_Control_R) {
        isControlDown = pressed;
    } else if (event.keyval == GDK_KEY_Alt_L || event.keyval == GDK_KEY_Alt_R) {
        isAltDown = pressed;
    } else {
        return false;
    }
    this->updateShape(isAltDown, isShiftDown, isControlDown);

    return true;
}

auto BaseShapeHandler::onKeyPressEvent(const KeyEvent& event) -> bool { return onKeyEvent(event, true); }

auto BaseShapeHandler::onKeyReleaseEvent(const KeyEvent& event) -> bool { return onKeyEvent(event, false); }


auto BaseShapeHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) -> bool {
    Point newPoint(pos.x / zoom, pos.y / zoom);
    if (!validMotion(newPoint, this->currPoint)) {
        return true;
    }
    this->currPoint = newPoint;

    this->updateShape(pos.isAltDown(), pos.isShiftDown(), pos.isControlDown());

    return true;
}

void BaseShapeHandler::onSequenceCancelEvent() { this->cancelStroke(); }

void BaseShapeHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    control->getCursor()->activateDrawDirCursor(false);  // in case released within  fixate_Dir_Mods_Dist

    if (this->shape.size() <= 1) {
        // We need at least two points to make a stroke (it can be twice the same)
        this->cancelStroke();
        return;
    }

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    stroke->setPointVector(this->shape, &lastSnappingRange);

    Range repaintRange = lastSnappingRange;
    repaintRange.addPadding(0.5 * this->stroke->getWidth());
    this->viewPool->dispatchAndClear(xoj::view::ShapeToolView::FINALIZATION_REQUEST, repaintRange);

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    auto ptr = stroke.get();
    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(std::move(stroke));
    doc->unlock();
    page->fireElementChanged(ptr);

    control->getCursor()->updateCursor();
}

void BaseShapeHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    xoj_assert(this->viewPool->empty());
    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    this->startPoint = snappingHandler.snapToGrid(this->buttonDownPoint, pos.isAltDown());
    this->currPoint = this->startPoint;

    this->stroke = createStroke(this->control);
}

void BaseShapeHandler::onButtonDoublePressEvent(const PositionInputData&, double) {
    // nothing to do
}

void BaseShapeHandler::modifyModifiersByDrawDir(double width, double height, double zoom, bool changeCursor) {
    bool gestureShift = this->flipShift;
    bool gestureControl = this->flipControl;

    if (this->drawModifierFixed == NONE) {
        // User hasn't dragged out past DrawDirModsRadius  i.e. modifier not yet locked.
        gestureShift = (width < 0) != gestureShift;
        gestureControl = (height < 0) != gestureControl;

        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;

        double fixate_Dir_Mods_Dist = control->getSettings()->getDrawDirModsRadius() / zoom;
        xoj_assert(fixate_Dir_Mods_Dist > 0.0);
        if (std::abs(width) > fixate_Dir_Mods_Dist || std::abs(height) > fixate_Dir_Mods_Dist) {
            this->drawModifierFixed = static_cast<DIRSET_MODIFIERS>(SET | (gestureShift ? SHIFT : NONE) |
                                                                    (gestureControl ? CONTROL : NONE));
            if (changeCursor) {
                control->getCursor()->activateDrawDirCursor(false);
            }
        } else {
            if (changeCursor) {
                control->getCursor()->activateDrawDirCursor(true, this->modShift, this->modControl);
            }
        }
    } else {
        gestureShift = gestureShift == !(this->drawModifierFixed & SHIFT);
        gestureControl = gestureControl == !(this->drawModifierFixed & CONTROL);
        this->modShift = this->modShift == !gestureShift;
        this->modControl = this->modControl == !gestureControl;
    }
}

auto BaseShapeHandler::getShape() const -> const std::vector<Point>& { return this->shape; }

auto BaseShapeHandler::createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> {
    return std::make_unique<xoj::view::ShapeToolView>(this, parent);
}

auto BaseShapeHandler::getViewPool() const
        -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::ShapeToolView>>& {
    return viewPool;
}
