/*
 * Xournal++
 *
 * A setsquare view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cmath>

#include <cairo.h>  // for cairo_t

#include "model/Setsquare.h"  // for Setsquare
#include "util/DispatchPool.h"
#include "view/overlays/BaseStrokeToolView.h"

#include "GeometryToolView.h"

class Stroke;
class OverlayBase;
class ZoomControl;

/**
 * @brief A class that renders a setsquare
 *
 * The setsquare has vertical, horizontal and angular marks.
 * The intersection angle with the x-axis is displayed as well.
 *
 * See the file development/documentation/Setsquare_Readme.md for
 * details how the setsquare is rendered.
 *
 * A temporary stroke is displayed when it is created near the
 * longest side of the setsquare or from a point to the mid point
 * of the longest side of the setsquare.
 */

namespace xoj::view {
class Repaintable;

class SetsquareView: public GeometryToolView {

public:
    SetsquareView(const Setsquare* s, Repaintable* parent, ZoomControl* zoomControl);
    ~SetsquareView() noexcept override;

    void on(FlagDirtyRegionRequest, const Range& rg) override;
    void on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) override;
    void deleteOn(FinalizationRequest, const Range& rg) override;

private:
    void drawGeometryTool(cairo_t* cr) const override;
    void drawDisplays(cairo_t* cr) const override;

    double height = Setsquare::INITIAL_HEIGHT;
    double rotation = 0.;
    cairo_matrix_t matrix{CM, 0., 0., CM, Setsquare::INITIAL_X, Setsquare::INITIAL_Y};


    /**
     * @brief the radius of the semi-circle for the angular marks
     */
    double radius = 4.5;

    /**
     * @brief the distance of the circle containing the rotation angle display from the mid point of the longest side of
     * the setsquare
     */
    double circlePos = 6.0;

    /**
     * @brief the distance (in cm) of the vertical marks from the symmetry axis of the setsquare
     */
    double horPosVmarks = 2.5;

    /**
     * @brief the index of the first vertical mark which should be drawn (which should not overlap with the measuring
     * marks)
     */
    int minVmark = 3;

    /**
     * @brief the index of the last vertical mark to be drawn
     */
    int maxVmark = 35;

    /**
     * @brief the number of angular marks that are left away on both ends (in order not to overlap with the measuring
     * marks)
     */
    int offset = 4;

    /**
     * @brief the index of the last horizontal mark to be drawn
     */
    int maxHmark = 70;

    void drawVerticalMarks(cairo_t* cr) const;
    void drawHorizontalMarks(cairo_t* cr) const;
    void drawAngularMarks(cairo_t* cr) const;
    void drawOutline(cairo_t* cr) const;
    void drawRotation(cairo_t* cr) const;
    void clipVerticalStripes(cairo_t* cr) const;
    void clipHorizontalStripes(cairo_t* cr) const;

public:
    static constexpr double LINE_WIDTH_IN_CM = .02;
};
};  // namespace xoj::view
