#include "ImageSizeSelectionView.h"

#include "control/tools/ImageSizeSelection.h"  // for ImageSizeSelection
#include "util/Rectangle.h"                    // for Rectangle
#include "util/raii/CairoWrappers.h"           // for cairo_save
#include "view/Repaintable.h"                  // for Repaintable

using namespace xoj::view;


ImageSizeSelectionView::ImageSizeSelectionView(const ImageSizeSelection* imageSizeSelection, Repaintable* parent,
                                               Color selectionColor):
        OverlayView(parent), imageSizeSelection(imageSizeSelection), selectionColor(selectionColor) {
    this->registerToPool(imageSizeSelection->getViewPool());
}

ImageSizeSelectionView::~ImageSizeSelectionView() noexcept = default;


void ImageSizeSelectionView::draw(cairo_t* cr) const {
    const xoj::util::Rectangle space = this->imageSizeSelection->getSelectedSpace();

    xoj::util::CairoSaveGuard const saveGuard(cr);  // cairo_save

    cairo_set_line_width(cr, BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    cairo_set_dash(cr, DASH_PATTERN.data(), DASH_PATTERN.size(), 0);
    Util::cairo_set_source_rgbi(cr, selectionColor);

    cairo_new_path(cr);

    // these lines form the outline of the space rectangle
    cairo_line_to(cr, space.x, space.y);
    cairo_line_to(cr, space.x, space.y + space.height);
    cairo_line_to(cr, space.x + space.width, space.y + space.height);
    cairo_line_to(cr, space.x + space.width, space.y);


    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, selectionColor, OPACITY_FOR_FILL);
    cairo_fill(cr);
}

auto ImageSizeSelectionView::isViewOf(const OverlayBase* overlay) const -> bool {
    return overlay == this->imageSizeSelection;
}

void ImageSizeSelectionView::on(ImageSizeSelectionView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void ImageSizeSelectionView::deleteOn(ImageSizeSelectionView::FinalizationRequest, Range rg) {
    rg.addPadding(BORDER_WIDTH_IN_PIXELS / this->parent->getZoom());
    this->parent->deleteOverlayView(this, rg);
}
