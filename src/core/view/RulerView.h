#pragma once

#include <cairo.h>

#include "model/Ruler.h"
#include "view/GeometryToolView.h"

namespace xoj::view {

class RulerView: public GeometryToolView {
public:
    RulerView(const Ruler* ruler, Repaintable* parent, ZoomControl* zoomControl);
    ~RulerView() noexcept override;

    void on(FlagDirtyRegionRequest, const Range& rg) override;
    void on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) override;
    void deleteOn(FinalizationRequest, const Range& rg) override;

private:
    void drawGeometryTool(cairo_t* cr) const override;
    void drawDisplays(cairo_t* cr) const override;

    double length = Ruler::INITIAL_LENGTH;
    double rotation = 0.;
    cairo_matrix_t matrix{CM, 0., 0., CM, 0., 0.};
};

}  // namespace xoj::view
