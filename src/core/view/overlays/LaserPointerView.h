/*
 * Xournal++
 *
 * View active Laser pointer strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstdint>

#include <cairo.h>

#include "util/DispatchPool.h"
#include "util/Range.h"
#include "view/Mask.h"
#include "view/overlays/OverlayView.h"

class OverlayBase;
class Settings;
class ZoomControl;
class LaserPointerHandler;
class StrokeHandler;

namespace xoj::view {
class Repaintable;
class StrokeToolView;

class LaserPointerView final: public OverlayView, public xoj::util::Listener<LaserPointerView> {

public:
    LaserPointerView(const LaserPointerHandler* handler, Repaintable* parent);
    ~LaserPointerView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct InputCancellationRequest {
    } INPUT_CANCELLATION_REQUEST = {};
    void on(InputCancellationRequest, const Range& rg);

    static constexpr struct StartNewStrokeRequest {
    } START_NEW_STROKE_REQUEST = {};
    void on(StartNewStrokeRequest, StrokeHandler* handler);

    static constexpr struct FinishStrokeRequest {
    } FINISH_STROKE_REQUEST = {};
    /// Deletes the TemporaryStrokeView and all references to the handler's StrokeHandler
    void on(FinishStrokeRequest, const Range& strokeBox);

    /// For fade out
    static constexpr struct SetAlphaRequest {
    } SET_ALPHA_REQUEST = {};
    void on(SetAlphaRequest, uint8_t alpha);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest);

private:
    Mask createMask(cairo_t* tgtcr) const;

private:
    const LaserPointerHandler* handler;

    std::unique_ptr<StrokeToolView> activeStrokeView;

    mutable Mask mask;

    uint8_t alpha = 255;  ///< for fadeout
    Range extents;        ///< for fadeout repaints
};
};  // namespace xoj::view
