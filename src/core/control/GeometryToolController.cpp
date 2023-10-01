#include "GeometryToolController.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "model/Document.h"
#include "model/GeometryTool.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "model/path/PiecewiseLinearPath.h"
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
    Stroke* cross = new Stroke();
    cross->setWidth(h->getToolThickness(TOOL_PEN)[TOOL_SIZE_FINE]);
    cross->setColor(h->getTool(TOOL_PEN).getColor());
    auto path = std::make_shared<PiecewiseLinearPath>(Point(x + MARK_SIZE, y + MARK_SIZE), 4);  // Reserve 4 segments
    path->addLineSegmentTo(Point(x - MARK_SIZE, y - MARK_SIZE));
    path->addLineSegmentTo(Point(x, y));
    path->addLineSegmentTo(Point(x + MARK_SIZE, y - MARK_SIZE));
    path->addLineSegmentTo(Point(x - MARK_SIZE, y + MARK_SIZE));
    cross->setPath(std::move(path));

    const auto doc = control->getDocument();
    const auto page = view->getPage();
    doc->lock();
    const auto layer = page->getSelectedLayer();
    layer->addElement(cross);
    doc->unlock();

    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, cross));

    const Rectangle<double> rect{cross->getX(), cross->getY(), cross->getElementWidth(), cross->getElementHeight()};
    view->rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void GeometryToolController::addStrokeToLayer() {
    const auto xournal = view->getXournal();
    const auto control = xournal->getControl();
    const auto doc = control->getDocument();
    const auto page = view->getPage();

    doc->lock();
    const auto layer = page->getSelectedLayer();
    layer->addElement(stroke.get());
    doc->unlock();
    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));
    const Rectangle<double> rect{stroke->getX(), stroke->getY(), stroke->getElementWidth(), stroke->getElementHeight()};
    view->rerenderRect(rect.x, rect.y, rect.width, rect.height);
    stroke.release();
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
