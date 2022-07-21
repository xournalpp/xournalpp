/*
 * Xournal++
 *
 * View active stroke tool -- filled strokes, but not filled highlighter.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <vector>

#include <cairo.h>

#include "model/Point.h"
#include "util/Point.h"

#include "StrokeToolView.h"

namespace xoj::view {
class StrokeToolFilledHighlighterView;

/**
 * @brief View active stroke tool -- with a filling
 *
 * In draw, the filling is painted directly, while the stroke itself is blitted from the mask
 */
class StrokeToolFilledView: public StrokeToolView {
public:
    StrokeToolFilledView(const StrokeHandler* strokeHandler, const Stroke& stroke, Repaintable* parent);
    virtual ~StrokeToolFilledView() noexcept;

    void drawFilling(cairo_t* cr, const std::vector<Point>& pts) const override;

    void on(AddPointRequest, const Point& p) override;
    void on(StrokeReplacementRequest, const Stroke& newStroke) override;

protected:
    class FillingData {
    public:
        FillingData(double alpha, const Point& p): alpha(alpha), firstPoint(p.x, p.y), contour{p} {}

        const double alpha;
        utl::Point<double> firstPoint;  // Store a copy for safe concurrent access


        void appendSegments(const std::vector<Point>& pts);

        /**
         * @brief Filling contour (i.e. the stroke's entire path)
         */
        std::vector<Point> contour;
    };

    mutable FillingData filling;
};
};  // namespace xoj::view
