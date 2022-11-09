#include "VerticalToolHandler.h"

#include <algorithm>  // for max, min, minmax
#include <cassert>    // for assert
#include <cmath>      // for abs
#include <memory>     // for __shared_ptr_access

#include <cairo.h>           // for cairo_fill, cairo_...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Control_L
#include <glib.h>            // for g_assert, g_assert...

#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "gui/LegacyRedrawable.h"                  // for Redrawable
#include "model/Element.h"                         // for Element
#include "model/Layer.h"                           // for Layer
#include "model/XojPage.h"                         // for XojPage
#include "undo/MoveUndoAction.h"                   // for MoveUndoAction
#include "util/DispatchPool.h"
#include "view/overlays/VerticalToolView.h"

class Settings;

VerticalToolHandler::VerticalToolHandler(const PageRef& page, Settings* settings, double y, bool initiallyReverse):
        page(page),
        layer(this->page->getSelectedLayer()),
        spacingSide(initiallyReverse ? Side::Above : Side::Below),
        snappingHandler(settings),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::VerticalToolView>>()) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    this->startY = ySnapped;
    this->endY = ySnapped;

    this->adoptElements(this->spacingSide);
}

VerticalToolHandler::~VerticalToolHandler() = default;

void VerticalToolHandler::adoptElements(const Side side) {
    this->spacingSide = side;

    // Return current elements back to page
    for (Element* e: this->elements) { this->layer->addElement(e); }
    this->elements.clear();

    // Add new elements based on position
    for (Element* e: this->layer->getElements()) {
        if ((side == Side::Below && e->getY() >= this->startY) ||
            (side == Side::Above && e->getY() + e->getElementHeight() <= this->startY)) {
            this->elements.push_back(e);
        }
    }

    for (Element* e: this->elements) { this->layer->removeElement(e, false); }

    Range rg = this->ownedElementsOriginalBoundingBox;
    this->ownedElementsOriginalBoundingBox = this->computeElementsBoundingBox();
    rg = rg.unite(this->ownedElementsOriginalBoundingBox);
    if (!rg.empty()) {
        this->page->fireRangeChanged(rg);
    }
}

void VerticalToolHandler::currentPos(double x, double y) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    if (this->endY == ySnapped) {
        return;
    }
    this->endY = ySnapped;
    this->viewPool->dispatch(xoj::view::VerticalToolView::SET_VERTICAL_SHIFT_REQUEST, ySnapped);
}

bool VerticalToolHandler::onKeyPressEvent(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) &&
        this->spacingSide == Side::Below) {
        this->adoptElements(Side::Above);
        this->viewPool->dispatch(xoj::view::VerticalToolView::SWITCH_DIRECTION_REQUEST);
        return true;
    }
    return false;
}

bool VerticalToolHandler::onKeyReleaseEvent(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) &&
        this->spacingSide == Side::Above) {
        this->adoptElements(Side::Below);
        this->viewPool->dispatch(xoj::view::VerticalToolView::SWITCH_DIRECTION_REQUEST);
        return true;
    }
    return false;
}

auto VerticalToolHandler::getElements() const -> const std::vector<Element*>& { return this->elements; }

auto VerticalToolHandler::computeElementsBoundingBox() const -> Range {
    Range rg;
    for (Element* e: this->elements) {
        rg = rg.unite(Range(e->boundingRect()));
    }
    return rg;
}

auto VerticalToolHandler::finalize() -> std::unique_ptr<MoveUndoAction> {

    // Erase the blue area indicating the shift
    this->viewPool->dispatchAndClear(xoj::view::VerticalToolView::FINALIZATION_REQUEST);

    if (this->elements.empty()) {
        return nullptr;
    }

    const double dY = this->endY - this->startY;
    auto undo =
            std::make_unique<MoveUndoAction>(this->layer, this->page, &this->elements, 0, dY, this->layer, this->page);

    for (Element* e: this->elements) {
        e->move(0, dY);

        this->layer->addElement(e);
    }
    this->elements.clear();

    this->ownedElementsOriginalBoundingBox.translate(0, dY);
    page->fireRangeChanged(this->ownedElementsOriginalBoundingBox);

    return undo;
}

double VerticalToolHandler::getPageWidth() const { return page->getWidth(); }

auto VerticalToolHandler::createView(xoj::view::Repaintable* parent, ZoomControl* zoomControl,
                                     const Settings* settings) const -> std::unique_ptr<xoj::view::OverlayView> {
    return std::make_unique<xoj::view::VerticalToolView>(this, parent, zoomControl, settings);
}
