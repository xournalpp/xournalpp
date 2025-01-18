#include "GeometryToolController.h"

#include <memory>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "model/Document.h"
#include "model/GeometryTool.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/InsertUndoAction.h"

using xoj::util::Rectangle;

constexpr double MARK_SIZE = 2.;

GeometryToolController::GeometryToolController(XojPageView* view, GeometryTool* geometryTool):
        view(view), geometryTool(geometryTool) {}

GeometryToolController::~GeometryToolController() = default;

void GeometryToolController::translate(double x, double y) {
    geometryTool->setTranslationX(geometryTool->getTranslationX() + x);
    geometryTool->setTranslationY(geometryTool->getTranslationY() + y);
    geometryTool->notify();
}

void GeometryToolController::rotate(double da, double cx, double cy) {
    geometryTool->setRotation(geometryTool->getRotation() + da);
    const auto offsetX = geometryTool->getTranslationX() - cx;
    const auto offsetY = geometryTool->getTranslationY() - cy;
    const auto mx = offsetX * cos(da) - offsetY * sin(da);
    const auto my = offsetX * sin(da) + offsetY * cos(da);
    geometryTool->setTranslationX(cx + mx);
    geometryTool->setTranslationY(cy + my);
    geometryTool->notify();
}

void GeometryToolController::scale(double f, double cx, double cy) {
    geometryTool->setHeight(geometryTool->getHeight() * f);
    const auto offsetX = geometryTool->getTranslationX() - cx;
    const auto offsetY = geometryTool->getTranslationY() - cy;
    const auto mx = offsetX * f;
    const auto my = offsetY * f;
    geometryTool->setTranslationX(cx + mx);
    geometryTool->setTranslationY(cy + my);
    geometryTool->notify(true);
}

void GeometryToolController::markPoint(double x, double y) {
    const auto control = view->getXournal()->getControl();
    const auto h = control->getToolHandler();
    auto cross = std::make_unique<Stroke>();
    cross->setWidth(h->getToolThickness(TOOL_PEN)[TOOL_SIZE_FINE]);
    cross->setColor(h->getTool(TOOL_PEN).getColor());
    cross->addPoint(Point(x + MARK_SIZE, y + MARK_SIZE));
    cross->addPoint(Point(x - MARK_SIZE, y - MARK_SIZE));
    cross->addPoint(Point(x, y));
    cross->addPoint(Point(x + MARK_SIZE, y - MARK_SIZE));
    cross->addPoint(Point(x - MARK_SIZE, y + MARK_SIZE));

    auto* ptr = cross.get();
    const auto doc = control->getDocument();
    const auto page = view->getPage();
    doc->lock();
    const auto layer = page->getSelectedLayer();
    layer->addElement(std::move(cross));
    doc->unlock();

    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, ptr));


    const Rectangle<double> rect{ptr->getX(), ptr->getY(), ptr->getElementWidth(), ptr->getElementHeight()};
    view->rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void GeometryToolController::addStrokeToLayer() {
    const auto xournal = view->getXournal();
    const auto control = xournal->getControl();
    const auto doc = control->getDocument();
    const auto page = view->getPage();

    auto ptr = stroke.get();
    doc->lock();
    const auto layer = page->getSelectedLayer();
    layer->addElement(std::move(this->stroke));
    doc->unlock();

    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, ptr));
    const Rectangle<double> rect{ptr->getX(), ptr->getY(), ptr->getElementWidth(), ptr->getElementHeight()};
    view->rerenderRect(rect.x, rect.y, rect.width, rect.height);
    geometryTool->setStroke(nullptr);
    xournal->getCursor()->updateCursor();
}

void GeometryToolController::initializeStroke() {
    const auto h = view->getXournal()->getControl()->getToolHandler();
    stroke = std::make_unique<Stroke>();
    geometryTool->setStroke(stroke.get());
    stroke->setWidth(h->getThickness());
    stroke->setColor(h->getColor());
    stroke->setFill(h->getFill());
    stroke->setLineStyle(h->getLineStyle());
    switch (h->getToolType()) {
        case TOOL_PEN:
            stroke->setToolType(StrokeTool::PEN);
            break;
        case TOOL_HIGHLIGHTER:
            stroke->setToolType(StrokeTool::HIGHLIGHTER);
            break;
        case TOOL_ERASER:
            stroke->setToolType(StrokeTool::ERASER);
            break;
        default:
            g_warning("Unhandled tool when initializing stroke in geometry tool controller");
            break;
    }
}

auto GeometryToolController::getPage() const -> const PageRef { return view->getPage(); }

auto GeometryToolController::getView() const -> XojPageView* { return view; }
