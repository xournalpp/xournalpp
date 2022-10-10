/*
 * Xournal++
 *
 * Displays the content of a selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "util/Color.h"
#include "util/DispatchPool.h"

#include "OverlayView.h"
#include "cairo.h"

class Selection;
class Range;

namespace xoj::view {
class Repaintable;

class SelectionView: public OverlayView, public xoj::util::Listener<SelectionView> {
public:
    SelectionView(const Selection* selection, Repaintable* parent, Color selectionColor);
    virtual ~SelectionView() noexcept;

    /**
     * @brief Draws the container to the given context
     */
    void draw(cairo_t* cr) const override;
    bool isViewOf(const OverlayBase* overlay) const override;

    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, Range rg);

    static constexpr struct DeleteViewsRequest {
    } DELETE_VIEWS_REQUEST = {};
    void deleteOn(DeleteViewsRequest, Range rg);

private:
    const Selection* selection;
    Color selectionColor;

    static constexpr double BORDER_WIDTH_IN_PIXELS = 1;
    static constexpr double FILLING_OPACITY = 0.3;
};
};  // namespace xoj::view
