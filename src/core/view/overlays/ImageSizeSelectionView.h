/*
 * Xournal++
 *
 * Displays a Rectangle to see while selecting space for an image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "util/DispatchPool.h"  // for Listener

#include "OverlayView.h"  // for OverlayView

class ImageSizeSelection;
class Range;

namespace xoj::view {
class Repaintable;

class ImageSizeSelectionView: public OverlayView, public xoj::util::Listener<ImageSizeSelectionView> {
public:
    ImageSizeSelectionView(const ImageSizeSelection* imageSizeSelection, Repaintable* parent);
    virtual ~ImageSizeSelectionView() noexcept;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    auto isViewOf(const OverlayBase* overlay) const -> bool override;

    // Listener
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, Range rg);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest, Range rg);

private:
    const ImageSizeSelection* imageSizeSelection;
};

};  // namespace xoj::view