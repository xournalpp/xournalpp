#include "ImageSizeSelectionView.h"

#include "control/tools/ImageSizeSelection.h"  // for ImageSizeSelection
#include "util/Rectangle.h"                    // for Rectangle
#include "util/raii/CairoWrappers.h"           // for cairo_save
#include "view/Repaintable.h"                  // for Repaintable


using xoj::util::Rectangle;

using namespace xoj::view;


ImageSizeSelectionView::ImageSizeSelectionView(const ImageSizeSelection* imageSizeSelection, Repaintable* parent):
        OverlayView(parent), imageSizeSelection(imageSizeSelection) {
    this->registerToPool(imageSizeSelection->getViewPool());
}

ImageSizeSelectionView::~ImageSizeSelectionView() noexcept = default;


void ImageSizeSelectionView::draw(cairo_t* cr) const {
    const Rectangle space = this->imageSizeSelection->getSelectedSpace();

    xoj::util::CairoSaveGuard const saveGuard(cr);  // cairo_save

    cairo_set_line_width(cr, 1);
    Util::cairo_set_source_rgbi(cr, ColorU8{114, 159, 207, 255});

    cairo_new_path(cr);

    cairo_line_to(cr, space.x, space.y);
    cairo_line_to(cr, space.x, space.y + space.height);
    cairo_line_to(cr, space.x + space.width, space.y + space.height);
    cairo_line_to(cr, space.x + space.width, space.y);


    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, ColorU8{114, 159, 207}, 0.2);
    cairo_fill(cr);
}

auto ImageSizeSelectionView::isViewOf(const OverlayBase* overlay) const -> bool {
    return overlay == this->imageSizeSelection;
}

void ImageSizeSelectionView::on(ImageSizeSelectionView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(1 / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void ImageSizeSelectionView::deleteOn(ImageSizeSelectionView::FinalizationRequest, Range rg) {
    rg.addPadding(1 / this->parent->getZoom());
    this->parent->deleteOverlayView(this, rg);
}
