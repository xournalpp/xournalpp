#include "ImageFrameView.h"

#include "model/ImageFrame.h"         // for ImageFrame
#include "util/raii/CairoWrappers.h"  // for cairo_save

using namespace xoj::view;

ImageFrameView::ImageFrameView(const ImageFrame* imgFrame): imageFrame(imgFrame) {}

ImageFrameView::~ImageFrameView() = default;

void ImageFrameView::draw(const xoj::view::Context& ctx) const {
    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    // todo p0mm draw image first if it is just inside the frame
    if (imageFrame->hasImage()) {
        // todo p0mm test call to check out partial view
        imageFrame->drawPartialImage(ctx, 0.3, 0.5, 0.6, 0.75, 0.02);
    }

    cairo_set_line_width(ctx.cr, 1);
    Util::cairo_set_source_rgbi(ctx.cr, ColorU8{114, 159, 207, 255});


    cairo_new_path(ctx.cr);

    cairo_line_to(ctx.cr, imageFrame->getX(), imageFrame->getY());
    cairo_line_to(ctx.cr, imageFrame->getX(), imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(ctx.cr, imageFrame->getX() + imageFrame->getElementWidth(),
                  imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(ctx.cr, imageFrame->getX() + imageFrame->getElementWidth(), imageFrame->getY());


    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, ColorU8{114, 159, 207}, 0.2);
    cairo_fill(ctx.cr);

    // todo p0mm draw image last if it is beeing moved / resized
    /*
    if (imageFrame->hasImage()) {
        imageFrame->drawImage(ctx);
    }
    */
}
