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

class Stroke;
class XojPageView;
class OverlayBase;

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

constexpr double rad(double n) { return n * M_PI / 180.; }
constexpr double rad(int n) { return rad(static_cast<double>(n)); }
constexpr double deg(double a) { return a * 180.0 / M_PI; }
inline double cathete(double h, double o) { return std::sqrt(std::pow(h, 2) - std::pow(o, 2)); }

class SetsquareView: public ToolView, public xoj::util::Listener<SetsquareView> {

public:
    SetsquareView(const Setsquare* s, Repaintable* parent);
    ~SetsquareView() override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, const Range& rg);

    static constexpr struct UpdateValuesRequest {
    } UPDATE_VALUES = {};
    void on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    /**
     * @brief Called before the setsquare's destruction
     * @param rg The bounding box of the entire setsquare
     */
    void deleteOn(FinalizationRequest, const Range& rg);

    /**
     * @brief draws the setsquare and temporary stroke to a cairo context
     * @param cr the cairo context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

private:
    /**
     * @brief draws the setsquare to a cairo context
     * @param cr the cairo context drawn to
     */
    void drawSetsquare(cairo_t* cr) const;

    /**
     * @brief draws the temporary stroke to a cairo context
     * @param cr the cairo context drawn to
     */
    void drawTemporaryStroke(cairo_t* cr) const;

    /**
     * @brief the underlying setsquare
     */
    const Setsquare* s;

    /**
     * @brief The stroke drawn aligned to the longest side of the setsquare or ending at the midpoint of the longest
     * side of the setsquare
     */
    Stroke* stroke = nullptr;

    /**
     * @brief renders text centered and possibly rotated at the current position on a cairo context
     * @param cr the cairo context
     * @param text the text string
     * @param angle the rotation angle
     */
    void showTextCenteredAndRotated(cairo_t* cr, std::string text, double angle) const;

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

    /**
     * @brief updates the values radius, horPosVmarks, minVmark, maxVmark computed from the height of the setsquare
     */
    void updateValues(double h, double rot, cairo_matrix_t m);
};
};  // namespace xoj::view
