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

#include "model/ImageFrame.h"  // for ImageFrame
#include "model/PageRef.h"     // for PageRef

#include "CursorSelectionType.h"  // for CursorSelectionTypes

enum SCALING { TOP, TOP_RIGHT, TOP_LEFT, BOTTOM, BOTTOM_RIGHT, BOTTOM_LEFT, LEFT, RIGHT, NO_SCALING };


class ImageFrameEditor {
public:
    ImageFrameEditor(const PageRef& page, double x, double y);
    virtual ~ImageFrameEditor();

    // user interactions
    void mouseDown(double x, double y);
    void mouseMove(double x, double y);
    void mouseUp(double x, double y);

    auto currentlyScaling() -> bool;

    void updateImageFrame(double x, double y);
    auto getSelectionTypeForPos(double x, double y) -> CursorSelectionType;

private:
    auto searchForImageFrame(double x, double y) -> bool;
    void calculateScalingPosition(double x, double y);


private:
    bool scaling = false;
    SCALING current = NO_SCALING;

    PageRef page;
    ImageFrame* imageFrame = nullptr;
};