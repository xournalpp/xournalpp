/*
 * Xournal++
 *
 * View active text edition
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
class TextEditor;
class Range;

namespace xoj::util {
template <typename T>
class Rectangle;
};

namespace xoj::view {
class Repaintable;

class TextEditionView final: public ToolView, public xoj::util::Listener<TextEditionView> {

public:
    TextEditionView(const TextEditor* handler, Repaintable* parent);
    ~TextEditionView() noexcept override;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;
    void drawWithoutDrawingAids(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    xoj::util::Rectangle<double> toWindowCoordinates(const xoj::util::Rectangle<double>& r) const;

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
    const TextEditor* textEditor;

    /**
     * @brief Pointer to the context menu displayed above the text editor
     */
    std::unique_ptr<TextEditorContextMenu> contextMenu;

public:
    // Padding between the text logical box and the frame
    static constexpr int PADDING_IN_PIXELS = 5;
    // Width of the lines making the frame
    static constexpr int BORDER_WIDTH_IN_PIXELS = 1;
    // Width of the cursor in insertion mode (or at the end of a line)
    static constexpr int INSERTION_CURSOR_WIDTH_IN_PIXELS = 2;
};
};  // namespace xoj::view
