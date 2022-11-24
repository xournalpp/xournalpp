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
GeometryToolController::GeometryToolController(XojPageView* view, GeometryTool* geometryTool):
        view(view), geometryTool(geometryTool) {}

GeometryToolController::~GeometryToolController() = default;

void GeometryToolController::move(double x, double y) {
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
    geometryTool->notify();
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
    geometryTool->setStroke(nullptr);
    xournal->getCursor()->updateCursor();
}

void GeometryToolController::initializeStroke() {
    const auto h = view->getXournal()->getControl()->getToolHandler();
    stroke = new Stroke();
    geometryTool->setStroke(stroke);
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

auto GeometryToolController::getPage() const -> PageRef { return view->getPage(); }

auto GeometryToolController::getView() const -> XojPageView* { return view; }
