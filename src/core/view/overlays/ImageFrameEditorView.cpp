#include "ImageFrameEditorView.h"

#include "control/tools/ImageFrameEditor.h"  // for ImageFrameEditor
#include "util/Rectangle.h"                  // for Rectangle
#include "util/raii/CairoWrappers.h"         // for cairo_save
#include "view/Repaintable.h"                // for Repaintable


using xoj::util::Rectangle;

using namespace xoj::view;


ImageFrameEditorView::ImageFrameEditorView(const ImageFrameEditor* handler, Repaintable* parent):
        ToolView(parent), imageFrameEditor(handler) {
    this->registerToPool(imageFrameEditor->getViewPool());
}

ImageFrameEditorView::~ImageFrameEditorView() noexcept = default;

void ImageFrameEditorView::draw(cairo_t* cr) const {
    this->imageFrameEditor->drawImageFrame(cr, this->parent->getZoom());
}

auto ImageFrameEditorView::isViewOf(const OverlayBase* overlay) const -> bool {
    return overlay == this->imageFrameEditor;
}

void ImageFrameEditorView::on(ImageFrameEditorView::FlagDirtyRegionRequest, Range rg) {
    rg.addPadding(1 / this->parent->getZoom());
    this->parent->flagDirtyRegion(rg);
}

void ImageFrameEditorView::deleteOn(ImageFrameEditorView::FinalizationRequest, Range rg) {
    rg.addPadding(1 / this->parent->getZoom());
    this->parent->drawAndDeleteToolView(this, rg);
}
