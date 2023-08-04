#include "ImageSizeSelection.h"

#include "util/Rectangle.h"  // for Rectangle


using xoj::util::Rectangle;

using namespace xoj::view;

ImageSizeSelection::ImageSizeSelection(double x, double y):
        startX(x),
        startY(y),
        endX(x),
        endY(y),
        viewPool(std::make_shared<xoj::util::DispatchPool<ImageSizeSelectionView>>()) {
    box.addPoint(x, y);
}

void ImageSizeSelection::updatePosition(double x, double y) {
    Range old(startX, startY);
    old.addPoint(endX, endY);
    old.addPoint(x, y);

    box = Range(startX, startY);
    this->endX = x;
    this->endY = y;
    box.addPoint(x, y);

    this->viewPool->dispatch(xoj::view::ImageSizeSelectionView::FLAG_DIRTY_REGION, old);
}

auto ImageSizeSelection::getSelectedSpace() const -> Rectangle<double> {
    const double width = this->startX < this->endX ? this->endX - this->startX : this->startX - this->endX;
    const double height = this->startY < this->endY ? this->endY - this->startY : this->startY - this->endY;
    const double x = this->startX < this->endX ? this->startX : this->endX;
    const double y = this->startY < this->endY ? this->startY : this->endY;
    return {x, y, width, height};
}

void ImageSizeSelection::finalize() {
    this->viewPool->dispatchAndClear(xoj::view::ImageSizeSelectionView::FINALIZATION_REQUEST, this->box);
}
