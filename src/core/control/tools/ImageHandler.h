/*
 * Xournal++
 *
 * Image Tool handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <tuple>  // for tuple

#include <gio/gio.h>  // for GFile

#include "model/Image.h"  // for Image

class Control;
class XojPageView;

class ImageHandler {
public:
    ImageHandler(Control* control, XojPageView* view);
    virtual ~ImageHandler();

public:
    bool insertImage(double x, double y);
    // does the complete insertion based on create and add
    bool insertImage(GFile* file, double x, double y);
    // creates the image and does automatic scaling of the image
    std::tuple<Image*, int, int> createImage(GFile* file, double x, double y);
    bool addImageToDocument(Image* img, bool addUndoAction);
    // scale down (only if necessary) the image so that the it fits on the page
    // applies (potentially adjusted) width/height to the image
    void automaticScaling(Image* img, double x, double y, int width, int height);

private:
    Control* control;
    XojPageView* view;
};
