/*
 * Xournal++
 *
 * View pdf selections
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>  // for cairo_t

#include "util/Color.h"
#include "util/DispatchPool.h"  // for Listener
#include "view/overlays/OverlayView.h"

class OverlayBase;
class PdfElemSelection;
class Range;

namespace xoj::view {
class Repaintable;

class PdfElementSelectionView final: public OverlayView, public xoj::util::Listener<PdfElementSelectionView> {

public:
    PdfElementSelectionView(const PdfElemSelection* selection, Repaintable* parent, Color selectionColor);
    ~PdfElementSelectionView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Listener interface
     */
    static constexpr struct CancelSelectionRequest {
    } CANCEL_SELECTION_REQUEST = {};
    void deleteOn(CancelSelectionRequest, const Range& rg);
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION_REQUEST = {};
    void on(FlagDirtyRegionRequest, const Range& rg) const;

private:
    const PdfElemSelection* selection;
    const Color selectionColor;

public:
    // Opacity of the background
    static constexpr double SELECTION_OPACITY = 0.3;
};
};  // namespace xoj::view
