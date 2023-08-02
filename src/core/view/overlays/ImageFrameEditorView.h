/*
 * Xournal++
 *
 * View Image Frames in Edition
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>  // for cairo_t

#include "util/DispatchPool.h"  // for Listener
#include "view/overlays/OverlayView.h"

class OverlayBase;
class ImageFrameEditor;
class Range;

namespace xoj::util {
template <typename T>
class Rectangle;
};

namespace xoj::view {
class Repaintable;

class ImageFrameEditorView final: public ToolView, public xoj::util::Listener<ImageFrameEditorView> {

public:
    ImageFrameEditorView(const ImageFrameEditor* handler, xoj::view::Repaintable* parent);
    ~ImageFrameEditorView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;

    auto isViewOf(const OverlayBase* overlay) const -> bool override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, Range rg);

    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest, Range rg);

private:
    const ImageFrameEditor* imageFrameEditor;
};
};  // namespace xoj::view
