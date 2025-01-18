#include "ShapeToolView.h"

#include <vector>

#include "control/tools/BaseShapeHandler.h"
#include "util/raii/CairoWrappers.h"
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

ShapeToolView::ShapeToolView(const BaseShapeHandler* toolHandler, Repaintable* parent):
        BaseShapeOrSplineToolView(toolHandler, parent), toolHandler(toolHandler) {
    this->registerToPool(toolHandler->getViewPool());
}

ShapeToolView::~ShapeToolView() noexcept { this->unregisterFromPool(); }

void ShapeToolView::draw(cairo_t* cr) const {
    const auto& pts = this->toolHandler->getShape();

    if (pts.empty()) {
        // The input sequence has been cancelled. This view should soon be deleted
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(cr);  // cairo_save

    cairo_t* effCr = this->prepareContext(cr);

    StrokeViewHelper::pathToCairo(effCr, pts);

    this->commitDrawing(cr);
}

bool ShapeToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->toolHandler; }

void ShapeToolView::on(ShapeToolView::FlagDirtyRegionRequest, const Range& rg) {
    maskWipeExtent = maskWipeExtent.unite(rg);
    this->parent->flagDirtyRegion(rg);
}

void ShapeToolView::deleteOn(ShapeToolView::FinalizationRequest, const Range& rg) {
    this->parent->drawAndDeleteToolView(this, rg);
}
