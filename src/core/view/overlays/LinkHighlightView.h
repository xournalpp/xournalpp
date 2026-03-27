/*
 * Xournal++
 *
 * A view for highlighting links
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <cairo.h>  // for

#include "util/DispatchPool.h"          // for Listener
#include "view/overlays/OverlayView.h"  // for OverlayView

class OverlayBase;
class LinkHandler;
class Range;
namespace xoj::view {
class Repaintable;

class LinkHighlightView: public OverlayView, public xoj::util::Listener<LinkHighlightView> {

public:
    LinkHighlightView(const LinkHandler* handler, Repaintable* parent);
    ~LinkHighlightView() noexcept;

    void draw(cairo_t* cr) const override;
    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, const Range& rg);
    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    /**
     * @brief Called when the link is unhighlighted
     * @param rg The bounding box of the link including padding
     */
    void deleteOn(FinalizationRequest, const Range& rg);

private:
    const LinkHandler* handler;
};
};  // namespace xoj::view
