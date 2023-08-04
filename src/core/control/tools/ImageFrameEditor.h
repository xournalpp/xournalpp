/*
 * Xournal++
 *
 * Allows Image Frames to be edited
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/ImageFrame.h"                    // for ImageFrame
#include "model/OverlayBase.h"                   // for OverlayBase
#include "model/PageRef.h"                       // for PageRef
#include "util/Color.h"                          // for Color
#include "util/DispatchPool.h"                   // for Listener
#include "view/overlays/ImageFrameEditorView.h"  // for ImageFrameEditorView

#include "CursorSelectionType.h"  // for CursorSelectionTypes

class Control;

enum IMAGE_FRAME_MODE {
    TOP,
    TOP_RIGHT,
    TOP_LEFT,
    BOTTOM,
    BOTTOM_RIGHT,
    BOTTOM_LEFT,
    LEFT,
    RIGHT,
    NO_SCALING,
    MOVE_IMAGE,
    PRE_MOVE_IMAGE
};

using namespace xoj::view;

class ImageFrameEditor: public OverlayBase {
public:
    ImageFrameEditor(Control* control, const PageRef& page, double x, double y);
    virtual ~ImageFrameEditor();

    // user interactions
    void mouseDown(double x, double y);
    void mouseMove(double x, double y);
    void mouseUp(double x, double y);

    auto currentlyScaling() -> bool;

    auto getSelectionColor() const -> Color;

    void updateImageFrame(double x, double y);
    auto getSelectionTypeForPos(double x, double y) -> CursorSelectionType;

    inline auto getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<ImageFrameEditorView>>& {
        return viewPool;
    }

    /// call immediately before deleting this
    void resetView();

    void drawImageFrame(cairo_t* cr, double zoom) const;

private:
    auto searchForImageFrame(double x, double y) -> bool;
    void calculateScalingPosition(double x, double y);

    void calculateRangeOfImageFrame();

private:
    bool scaling = false;
    IMAGE_FRAME_MODE current = NO_SCALING;
    std::pair<double, double> pos_buf = std::make_pair(-1.0, -1.0);

    Control* control;
    PageRef page;
    ImageFrame* imageFrame = nullptr;

    Range box;
    std::shared_ptr<xoj::util::DispatchPool<ImageFrameEditorView>> viewPool;
};