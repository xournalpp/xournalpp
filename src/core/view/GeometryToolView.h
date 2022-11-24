/*
 * Xournal++
 *
 * A GeometryTool view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cmath>

#include <cairo.h>  // for cairo_t

#include "control/zoom/ZoomListener.h"
#include "model/GeometryTool.h"  // for GeometryTool
#include "view/overlays/BaseStrokeToolView.h"

#include "Mask.h"

class Stroke;
class OverlayBase;
class ZoomControl;

/**
 * @brief A class that renders a geometry tool
 */

namespace xoj::view {
class Repaintable;

constexpr double rad(double n) { return n * M_PI / 180.; }
constexpr double rad(int n) { return rad(static_cast<double>(n)); }
constexpr double deg(double a) { return a * 180.0 / M_PI; }
inline double cathete(double h, double o) { return std::sqrt(std::pow(h, 2) - std::pow(o, 2)); }

class GeometryToolView: public ToolView, public ZoomListener, public xoj::util::Listener<GeometryToolView> {

public:
    GeometryToolView(const GeometryTool* geometryTool, Repaintable* parent, ZoomControl* zoomControl);
    virtual ~GeometryToolView();

    /**
     * Zoom interface
     */
    void zoomChanged() override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    virtual void on(FlagDirtyRegionRequest, const Range& rg) = 0;

    static constexpr struct UpdateValuesRequest {
    } UPDATE_VALUES = {};
    virtual void on(UpdateValuesRequest, double h, double rot, cairo_matrix_t m) = 0;

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    /**
     * @brief Called before the corresponding GeometryTool's destruction
     * @param rg The bounding box of the entire geometry tool
     */
    virtual void deleteOn(FinalizationRequest, const Range& rg) = 0;

    static constexpr struct ResetMaskRequest {
    } RESET_MASK = {};
    void on(ResetMaskRequest);

    /**
     * @brief draws the geometry tool and temporary stroke to a cairo context
     * @param cr the cairo context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

private:
    /**
     * @brief draws the GeometryTool to a cairo context
     * @param cr the cairo context drawn to
     */
    virtual void drawGeometryTool(cairo_t* cr) const = 0;

    /**
     * @brief draws displays (that may change with rotation and/or translation)
     * of the geometry tool
     *
     * @param cr the cairo context drawn to
     */
    virtual void drawDisplays(cairo_t* cr) const = 0;

    /**
     * @brief draws the temporary stroke to a cairo context
     * @param cr the cairo context drawn to
     */
    void drawTemporaryStroke(cairo_t* cr) const;

    /**
     * @brief Creates a mask for caching the drawing of the geometry tool (without stroke)
     */
    Mask createMask(cairo_t* targetCr) const;

protected:
    /**
     * @brief the underlying GeometryTool
     */
    const GeometryTool* geometryTool;

    /**
     * @brief The stroke drawn aligned to the longest side of the GeometryTool or ending at the midpoint of the longest
     * side of the GeometryTool
     */
    Stroke* stroke = nullptr;

    /**
     * @brief renders text centered and possibly rotated at the current position on a cairo context
     * @param cr the cairo context
     * @param text the text string
     * @param angle the rotation angle
     */
    void showTextCenteredAndRotated(cairo_t* cr, const std::string& text, double angle) const;

private:
    mutable Mask mask;
    ZoomControl* zoomControl;
};
};  // namespace xoj::view