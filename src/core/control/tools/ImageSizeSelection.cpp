#include "ImageSizeSelection.h"

#include "util/Rectangle.h"  // for Rectangle

using xoj::util::Rectangle;

ImageSizeSelection::ImageSizeSelection(double x, double y): startX(x), startY(y), endX(x), endY(y) {}

void ImageSizeSelection::updatePosition(double x, double y) {
    this->endX = x;
    this->endY = y;
}

auto ImageSizeSelection::getSelectedSpace() -> Rectangle<double> {
    const double width = this->startX < this->endX ? this->endX - this->startX : this->startX - this->endX;
    const double height = this->startY < this->endY ? this->endY - this->startY : this->startY - this->endY;
    const double x = this->startX < this->endX ? this->startX : this->endX;
    const double y = this->startY < this->endY ? this->startY : this->endY;
    return {x, y, width, height};
}
