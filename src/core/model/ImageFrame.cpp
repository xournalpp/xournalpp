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

void ImageFrame::drawPartialImage(cairo_t* cr, double xIgnoreP, double yIgnoreP, double xDrawP, double yDrawP,
                                  double alphaForIgnore) const {
    if (this->containsImage) {
        auto elementView = xoj::view::ElementView::createFromElement(image);

        auto* imageView = dynamic_cast<xoj::view::ImageView*>(elementView.get());
        imageView->drawPartial(cr, xIgnoreP, yIgnoreP, xDrawP, yDrawP, alphaForIgnore);
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
        // todo p0mm have selection tool only see the partial image
        this->snappedBounds = this->getVisiblePartOfImage();
        // Rectangle<double>(this->x - 2, this->y -2 , this->width +2, this->height + 2);
    } else {
        this->snappedBounds = Rectangle<double>(this->x, this->y, this->width, this->height);
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

auto ImageFrame::getVisiblePartOfImage() const -> xoj::util::Rectangle<double> {
    if (!containsImage || image == nullptr) {
        return {0.0, 0.0, 0.0, 0.0};
    }

    const double vImgX = std::max(x, image->getX());
    const double vImgY = std::max(y, image->getY());

    double vImgH = image->getElementHeight();
    double vImgW = image->getElementWidth();

    const double difTopH = y - image->getY();
    if (difTopH > 0.0) {
        vImgH -= difTopH;
    }

    const double difBottomH = image->getElementHeight() + image->getY() - y - height;
    if (difBottomH > 0.0) {
        vImgH -= difBottomH;
    }

    const double difLeftW = x - image->getX();
    if (difLeftW > 0.0) {
        vImgW -= difLeftW;
    }

    const double difRightW = image->getElementWidth() + image->getX() - x - width;
    if (difRightW > 0.0) {
        vImgW -= difRightW;
    }

    if (vImgH < 0 || vImgW < 0) {
        return {0.0, 0.0, 0.0, 0.0};
    }

    return {vImgX, vImgY, vImgW, vImgH};
}

void ImageFrame::moveOnlyFrame(double x, double y, double width, double height) {
    this->x += x;
    this->width += width;
    this->y += y;
    this->height += height;

    // make sure there is still some image visible
    auto visPost = getVisiblePartOfImage();
    if (visPost.height == 0.0 && visPost.width == 0.0 && visPost.x == 0.0 && visPost.y == 0.0) {
        this->x -= x;
        this->width -= width;
        this->y -= y;
        this->height -= height;
    }
}

void ImageFrame::moveOnlyImage(double x, double y) {
    this->image->setX(image->getX() + x);
    this->image->setY(image->getY() + y);

    // make sure there is still some image visible
    auto visPost = getVisiblePartOfImage();
    if (visPost.height == 0.0 && visPost.width == 0.0 && visPost.x == 0.0 && visPost.y == 0.0) {
        this->image->setX(image->getX() - x);
        this->image->setY(image->getY() - y);
    }
}
void ImageFrame::setCouldBeEdited(bool could) {
    this->sizeCalculated = false;
    this->editable = could;
}

auto ImageFrame::inEditing() const -> bool { return editing; }

void ImageFrame::setInEditing(bool edit) {
    this->sizeCalculated = false;
    this->editing = edit;
}
