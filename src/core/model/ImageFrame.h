/*
 * Xournal++
 *
 * A Frame that might contain an Image
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Rectangle.h"  // for Rectangle
#include "view/View.h"       // for View

#include "Element.h"  // for Element
#include "Image.h"    // for Image

class ObjectInputStream;
class ObjectOutputStream;


enum ImageFrameMode { FILL, SCALE_DOWN, SCALE_UP };

class ImageFrame: public Element {
public:
    ImageFrame(xoj::util::Rectangle<double> space);
    ~ImageFrame() override;

    // no copy or move semantics
    ImageFrame(const ImageFrame&) = delete;
    ImageFrame(ImageFrame&&) = delete;
    auto operator=(const ImageFrame&) -> ImageFrame& = delete;
    auto operator=(ImageFrame&&) -> ImageFrame& = delete;

    auto clone() const -> ImageFrame* override;

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    auto intersectsArea(const GdkRectangle* src) const -> bool override;
    auto intersectsArea(double x, double y, double width, double height) const -> bool override;

    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;

public:
    void setImage(Image* img);


    auto hasImage() const -> bool;
    auto couldBeEdited() const -> bool;

    /**
     * helper method for ImageFrameView
     */
    void drawPartialImage(const xoj::view::Context& ctx, double xIgnoreP, double yIgnoreP, double xDrawP, double yDrawP,
                          double alphaForIgnore) const;

    /**
     * helper method for ImageFrameView
     */
    auto getImagePosition() const -> xoj::util::Rectangle<double>;


    void moveOnlyFrame(double x, double y, double width, double height);

    void moveOnlyImage(double x, double y, double width, double height);

    // todo p0mm move to private
    /**
     * calculates which part of the image is visible & returns coords etc of this part
     */
    xoj::util::Rectangle<double> getVisiblePartOfImage() const;

private:
    /**
     * recalculates size and location of the image, based on selected mode
     */
    void adjustImageToFrame();

    /**
     * scales image down to fit given space
     *
     * this will not stretch the image
     */
    void scaleImageDown();

    /**
     * scales image up to fill given space
     *
     * this will not stretch the image
     */
    void scaleImageUp();


    /**
     * adjust coordinates to center image in given space (after scaling)
     */
    void centerImage();

private:
    bool editable = true;
    bool containsImage = false;
    Image* image = nullptr;
    double imageXOffset = 0.0;
    double imageYOffset = 0.0;

    // todo p0mm have a preference setting for this default value,or only use scale_up???
    ImageFrameMode mode = ImageFrameMode::SCALE_UP;
};
