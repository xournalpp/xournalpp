#include "ImageFrame.h"

#include "control/Control.h"  // for Control
#include "control/settings/Settings.h"
#include "view/ImageView.h"
#include "view/View.h"


using xoj::util::Rectangle;

ImageFrame::ImageFrame(Rectangle<double> space): Element(ELEMENT_IMAGEFRAME) {
    this->x = space.x;
    this->y = space.y;
    this->height = space.height;
    this->width = space.width;
}

ImageFrame::~ImageFrame() = default;

auto ImageFrame::clone() const -> ImageFrame* {
    // todo p0mm
    return nullptr;
}

void ImageFrame::serialize(ObjectOutputStream& out) const {
    // todo p0mm
}

void ImageFrame::readSerialized(ObjectInputStream& in) {
    // todo p0mm
}

auto ImageFrame::hasImage() const -> bool { return containsImage; }

void ImageFrame::drawPartialImage(const xoj::view::Context& ctx, double xIgnoreP, double yIgnoreP, double xDrawP,
                                  double yDrawP, double alphaForIgnore) const {
    if (this->containsImage) {
        auto elementView = xoj::view::ElementView::createFromElement(image);

        // using static_cast instead of dynamic, as this will 100% be an imageView
        auto* imageView = static_cast<xoj::view::ImageView*>(elementView.get());
        imageView->drawPartial(ctx, xIgnoreP, yIgnoreP, xDrawP, yDrawP, alphaForIgnore);
    }
}

void ImageFrame::scale(double x0, double y0, double fx, double fy, double, bool) {
    this->x -= x0;
    this->x *= fx;
    this->x += x0;
    this->y -= y0;
    this->y *= fy;
    this->y += y0;

    this->width *= fx;
    this->height *= fy;

    if (this->width <= 0) {
        this->width = 1;
    }
    if (this->height <= 0) {
        this->height = 1;
    }

    this->calcSize();

    if (this->containsImage) {
        adjustImageToFrame();
    }
}

// not available until image rotation is (or free form image cutting?)
void ImageFrame::rotate(double x0, double y0, double th) {}

void ImageFrame::calcSize() const {
    if (!this->editable && this->containsImage) {
        // todo p0mm snappedBound should only be the visible image part!!
        this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
    } else {
        this->snappedBounds = Rectangle<double>(this->x - 1, this->y - 1, this->width + 2, this->height + 2);
    }
    this->sizeCalculated = true;
}

auto ImageFrame::intersectsArea(const GdkRectangle* src) const -> bool {
    return Element::intersectsArea(src) || this->image->intersectsArea(src);
}

auto ImageFrame::intersectsArea(double x, double y, double width, double height) const -> bool {
    return Element::intersectsArea(x, y, width, height) || this->image->intersectsArea(x, y, width, height);
}

void ImageFrame::setImage(Image* img) {
    this->image = img;
    this->image->setX(this->x);
    this->image->setY(this->y);

    adjustImageToFrame();
    this->containsImage = true;
}

void ImageFrame::move(double dx, double dy) {
    Element::move(dx, dy);
    this->image->move(dx, dy);
}

void ImageFrame::centerImage() {
    if (image == nullptr) {
        return;
    }

    this->image->setY(y);
    this->image->setX(x);

    if (this->image->getElementHeight() > height) {
        this->image->setY(this->y - ((this->image->getElementHeight() - height) * 0.5));
    } else if (this->image->getElementHeight() < height) {
        this->image->setY(this->y + ((height - this->image->getElementHeight()) * 0.5));
    }

    if (this->image->getElementWidth() > width) {
        this->image->setX(this->x - ((this->image->getElementWidth() - width) * 0.5));
    } else if (this->image->getElementWidth() < width) {
        this->image->setX(this->x + ((width - this->image->getElementWidth()) * 0.5));
    }
}

void ImageFrame::scaleImageDown() {
    if (image == nullptr) {
        return;
    }
    auto [img_width, img_height] = this->image->getImageSize();

    const double scaling = std::min(height / img_height, width / img_width);

    this->image->setWidth(scaling * img_width);
    this->image->setHeight(scaling * img_height);
}

void ImageFrame::scaleImageUp() {
    if (image == nullptr) {
        return;
    }
    auto [img_width, img_height] = this->image->getImageSize();

    const double scaling = std::max(height / img_height, width / img_width);

    image->setHeight(scaling * img_height);
    image->setWidth(scaling * img_width);
}

void ImageFrame::adjustImageToFrame() {
    if (image == nullptr) {
        return;
    }

    switch (this->mode) {
        case FILL:
            this->image->setWidth(this->width);
            this->image->setHeight(this->height);
            break;
        case SCALE_DOWN:
            scaleImageDown();
            break;
        case SCALE_UP:
            scaleImageUp();
            break;
    }
    if (this->containsImage) {
        this->image->setX(x + (imageXOffset * image->getElementWidth()));
        this->image->setY(y + (imageYOffset * image->getElementHeight()));
    } else {
        centerImage();
        imageXOffset = (image->getX() - x) / image->getElementWidth();
        imageYOffset = (image->getY() - y) / image->getElementHeight();
    }
}

auto ImageFrame::couldBeEdited() const -> bool { return editable; }

auto ImageFrame::getImagePosition() const -> Rectangle<double> {
    if (!this->containsImage) {
        return {0.0, 0.0, 0.0, 0.0};
    }
    return {this->image->getX(), this->image->getY(), this->image->getElementWidth(), this->image->getElementHeight()};
}
