/*
 * Xournal++
 *
 * View active shape tools (rectangle, ellipse and so on)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>  // for cairo_t

#include "util/DispatchPool.h"  // for Listener

#include "BaseShapeOrSplineToolView.h"

class BaseShapeHandler;
class OverlayBase;
class Range;

namespace xoj::view {
class Repaintable;

class ShapeToolView final: public BaseShapeOrSplineToolView, public xoj::util::Listener<ShapeToolView> {

public:
    ShapeToolView(const BaseShapeHandler* handler, const Repaintable* parent);
    ~ShapeToolView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, const Range& rg);

private:
    const BaseShapeHandler* toolHandler;
};
};  // namespace xoj::view
