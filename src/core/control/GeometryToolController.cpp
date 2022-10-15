#include "GeometryToolController.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "model/GeometryTool.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/InsertUndoAction.h"

using xoj::util::Rectangle;
GeometryToolController::GeometryToolController(XojPageView* view, GeometryTool* s): view(view), s(s) {}

GeometryToolController::~GeometryToolController() = default;

void GeometryToolController::move(double x, double y) {
    s->setTranslationX(s->getTranslationX() + x);
    s->setTranslationY(s->getTranslationY() + y);
    s->notify();
}

void GeometryToolController::rotate(double da, double cx, double cy) {
    s->setRotation(s->getRotation() + da);
    const auto tx = s->getTranslationX();
    const auto ty = s->getTranslationY();
    const auto offsetX = tx - cx;
    const auto offsetY = ty - cy;
    const auto mx = offsetX * cos(da) - offsetY * sin(da);
    const auto my = offsetX * sin(da) + offsetY * cos(da);
    s->setTranslationX(cx + mx);
    s->setTranslationY(cy + my);
    s->notify();
}

void GeometryToolController::scale(double f, double cx, double cy) {
    s->setHeight(s->getHeight() * f);
    const auto tx = s->getTranslationX();
    const auto ty = s->getTranslationY();
    const auto offsetX = tx - cx;
    const auto offsetY = ty - cy;
    const auto mx = offsetX * f;
    const auto my = offsetY * f;
    s->setTranslationX(cx + mx);
    s->setTranslationY(cy + my);
    s->notify();
}


void GeometryToolController::addStrokeToLayer() {
    const auto xournal = view->getXournal();
    const auto control = xournal->getControl();
    const auto page = view->getPage();
    control->getLayerController()->ensureLayerExists(page);
    const auto layer = page->getSelectedLayer();

    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke));

    layer->addElement(stroke);

    const Rectangle<double> rect{stroke->getX(), stroke->getY(), stroke->getElementWidth(), stroke->getElementHeight()};
    view->rerenderRect(rect.x, rect.y, rect.width, rect.height);
    stroke = nullptr;
    s->setStroke(nullptr);
    xournal->getCursor()->updateCursor();
}

void GeometryToolController::initializeStroke() {
    const auto h = view->getXournal()->getControl()->getToolHandler();
    stroke = new Stroke();
    s->setStroke(stroke);
    stroke->setWidth(h->getThickness());
    stroke->setColor(h->getColor());
    stroke->setFill(h->getFill());
    stroke->setLineStyle(h->getLineStyle());
}

auto GeometryToolController::getPage() const -> PageRef { return view->getPage(); }

auto GeometryToolController::getView() const -> XojPageView* { return view; }
