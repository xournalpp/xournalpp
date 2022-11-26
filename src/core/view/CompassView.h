/*
 * Xournal++
 *
 * A Compass view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "model/Compass.h"  // for Compass
#include "util/DispatchPool.h"
#include "view/overlays/BaseStrokeToolView.h"

#include "GeometryToolView.h"

class Stroke;
class OverlayBase;
class ZoomControl;

/**
 * @brief A class that renders a compass
 *
 * The compass has vertical, horizontal and angular marks.
 * The intersection angle with the x-axis is displayed as well.
 *
 * See the file development/documentation/Compass_Readme.md for
 * details how the Compass is rendered.
 *
 * A temporary stroke is displayed when it is created near the
 * outline of the Compass or at the marked radius (with angle 0).
 */

namespace xoj::view {
class Repaintable;

class CompassView: public GeometryToolView {

public:
    CompassView(const Compass* s, Repaintable* parent, ZoomControl* zoomControl);
    ~CompassView() noexcept override;

    void on(FlagDirtyRegionRequest, const Range& rg) override;
    void on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) override;
    void deleteOn(FinalizationRequest, const Range& rg) override;

private:
    void drawGeometryTool(cairo_t* cr) const override;
    void drawDisplays(cairo_t* cr) const override;

    double height = Compass::INITIAL_HEIGHT;
    double rotation = 0.;
    cairo_matrix_t matrix{CM, 0., 0., CM, Compass::INITIAL_X, Compass::INITIAL_Y};

    /**
     * @brief the distance of the circle containing the rotation angle display from the center
     * of the compass
     */
    double circlePos = 2.;

    /**
     * @brief the distance of the angular captions (written numbers) for certain angles
     */
    double angularCaptionPos = .9;

    /**
     * @brief the index of the last horizontal mark to be drawn
     */
    int maxHmark = 10 * Compass::INITIAL_HEIGHT;

    /**
     * @brief after how many degrees should a new small angular mark be drawn
     */
    int angularOffset = 1;

    /**
     * @brief after how many degrees should a new big angular mark be drawn and a number be added
     */
    int angularCaptionOffset = 30;

    /**
     * @brief should the numbers along the measuring marks be drawn
     */
    bool drawRadialCaption = true;

    /**
     * @brief should the display of the rotation be drawn
     */
    bool drawRotationDisplay = true;

    void drawHorizontalMarks(cairo_t* cr) const;
    void drawAngularMarks(cairo_t* cr) const;
    void drawOutline(cairo_t* cr) const;
    void drawRotation(cairo_t* cr) const;

public:
    static constexpr double LINE_WIDTH_IN_CM = .02;
};
};  // namespace xoj::view
