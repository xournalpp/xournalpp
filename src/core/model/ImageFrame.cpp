#include "ImageFrame.h"

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

void ImageFrame::drawImage(const xoj::view::Context& ctx) const {
    if (this->containsImage) {
        auto elementView = xoj::view::ElementView::createFromElement(image);
        elementView->draw(ctx);
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
    this->calcSize();

    if (this->containsImage) {
        adjustImageToFrame();
    }
}

// not available until image rotation is (or image cutting?)
void ImageFrame::rotate(double x0, double y0, double th) {}

void ImageFrame::calcSize() const {
    // todo p0mm what if the image is bigger then the frame?
    this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
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
    this->containsImage = true;
    this->image->setX(this->x);
    this->image->setY(this->y);

    adjustImageToFrame();
}
void ImageFrame::move(double dx, double dy) {
    Element::move(dx, dy);
    this->image->move(dx, dy);
}

void ImageFrame::centerImage() {
    if (!containsImage) {
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
    if (!containsImage) {
        return;
    }
    auto [img_width, img_height] = this->image->getImageSize();

    const double scaling = std::min(height / img_height, width / img_width);

    this->image->setWidth(scaling * img_width);
    this->image->setHeight(scaling * img_height);
}

void ImageFrame::scaleImageUp() {
    if (!containsImage) {
        return;
    }
    auto [img_width, img_height] = this->image->getImageSize();

    const double scaling = std::max(height / img_height, width / img_width);

    image->setHeight(scaling * img_height);
    image->setWidth(scaling * img_width);
}

void ImageFrame::adjustImageToFrame() {
    if (!containsImage) {
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
    centerImage();
}
