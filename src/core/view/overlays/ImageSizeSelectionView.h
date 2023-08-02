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

#include <array>  // for std::array

#include "util/Color.h"         // for Color
#include "util/DispatchPool.h"  // for Listener

#include "OverlayView.h"  // for OverlayView

class ImageSizeSelection;
class Range;

namespace xoj::view {
class Repaintable;

class ImageSizeSelectionView: public OverlayView, public xoj::util::Listener<ImageSizeSelectionView> {
public:
    ImageSizeSelectionView(const ImageSizeSelection* imageSizeSelection, Repaintable* parent, Color selectionColor);
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
    const Color selectionColor;

    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    static constexpr double OPACITY_FOR_FILL = 0.1;
    static constexpr std::array<const double, 2> DASH_PATTERN = {6.0, 4.0};
};

};  // namespace xoj::view