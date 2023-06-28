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
    // todo p0mm documentation!
    // does the complete insertion based on create and add
    bool insertImage(double x, double y);
    // creates the image and does automatic scaling of the image
    std::tuple<Image*, int, int> createImage(double x, double y);
    std::tuple<Image*, int, int> createImage(GFile* file, double x, double y);
    bool addImageToDocument(Image* img, bool addUndoAction);
    // scale down (only if necessary) the image so that the it fits on the page
    // applies (potentially adjusted) width/height to the image
    void automaticScaling(Image* img, double x, double y, int width, int height);

    /**
     * inserts an image with predetermined size
     */
    bool insertImageWithSize(xoj::util::Rectangle<double> space);

    /**
     * scales image down to fit given space, without stretching the image
     */
    void scaleImageDown(Image* img, xoj::util::Rectangle<double> space);

    void scaleImageUp(Image* img, xoj::util::Rectangle<double> space);

private:
    Control* control;
    XojPageView* view;
};
