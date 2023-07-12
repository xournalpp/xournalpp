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
    auto insertImageFrame(xoj::util::Rectangle<double> size) -> bool;

    auto insertImage(xoj::util::Rectangle<double> size) -> bool;

    /**
     * lets the user choose an image file and then creates the image
     */
    auto chooseAndCreateImage(double x, double y) -> std::tuple<Image*, int, int>;

    /**
     * creates the image from the given file
     */
    auto createImageFromFile(GFile* file, double x, double y) -> std::tuple<Image*, int, int>;

    auto addImageToDocument(Image* img, bool addUndoAction) -> bool;

    auto addImageFrameToDocument(Image* img, xoj::util::Rectangle<double> space, bool addUndoAction) -> bool;

    /**
     * scale down (only if necessary) the image so that it then fits on the page
     * applies (potentially adjusted) width/height to the image
     */
    void automaticScaling(Image* img, double x, double y, int width, int height);


private:
    Control* control;
    XojPageView* view;
};
