/*
 * Xournal++
 *
 * View active VerticalTool (move elements up and down)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>  // for cairo_t

#include "control/tools/VerticalToolHandler.h"  // for Side
#include "control/zoom/ZoomListener.h"
#include "util/Color.h"
#include "util/DispatchPool.h"  // for Listener
#include "view/Mask.h"
#include "view/overlays/OverlayView.h"

class OverlayBase;
class Range;
class Settings;
class ZoomControl;

namespace xoj::view {
class Repaintable;

class VerticalToolView final: public ToolView, public ZoomListener, public xoj::util::Listener<VerticalToolView> {

public:
    VerticalToolView(const VerticalToolHandler* handler, Repaintable* parent, ZoomControl* zoomControl,
                     const Settings* settings);
    ~VerticalToolView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;
    void drawWithoutDrawingAids(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Zoom interface
     */
    void zoomChanged() override;

    /**
     * Listener interface
     */
    static constexpr struct SetVerticalShiftRequest {
    } SET_VERTICAL_SHIFT_REQUEST = {};
    void on(SetVerticalShiftRequest, double shift);

    static constexpr struct SwitchDirectionRequest {
    } SWITCH_DIRECTION_REQUEST = {};
    void on(SwitchDirectionRequest);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest);

private:
    Mask createMask(cairo_t* tgtcr) const;

private:
    const VerticalToolHandler* toolHandler;
    ZoomControl* zoomControl;
    double lastShift = 0.0;
    const Color aidColor;

    mutable Mask mask;

public:
    // Width of the line delimiting moved elements
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    // Opacity of the background
    static constexpr double BACKGROUND_OPACITY = 0.3;
};
};  // namespace xoj::view
