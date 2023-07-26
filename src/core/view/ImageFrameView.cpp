#include "ImageFrameView.h"

#include <array>  // for std::array

#include "model/ImageFrame.h"         // for ImageFrame
#include "util/raii/CairoWrappers.h"  // for cairo_save

using namespace xoj::view;

ImageFrameView::ImageFrameView(const ImageFrame* imgFrame): imageFrame(imgFrame) {}

ImageFrameView::~ImageFrameView() = default;

void ImageFrameView::draw(const xoj::view::Context& ctx) const {
    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    if (imageFrame->couldBeEdited()) {
        drawFrame(ctx, WHITE);
        if (imageFrame->hasImage()) {
            drawImage(ctx, 0.0);
        }
        drawFrameHandles(ctx, WHITE);
    } else {
        if (imageFrame->hasImage()) {
            drawImage(ctx, 0.0);
        } else {
            drawFrame(ctx, WHITE);
            drawFrameHandles(ctx, BLACK);
        }
    }
}

void ImageFrameView::drawFrame(const Context& ctx, Color color) const {
    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    cairo_set_line_width(ctx.cr, 1 / ZOOM);

    Util::cairo_set_source_rgbi(ctx.cr, BLACK);
    std::array<double, 2> dashes = {6.0, 4.0};
    cairo_set_dash(ctx.cr, dashes.data(), dashes.size(), 0.0);


    cairo_new_path(ctx.cr);

    cairo_line_to(ctx.cr, imageFrame->getX(), imageFrame->getY());
    cairo_line_to(ctx.cr, imageFrame->getX(), imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(ctx.cr, imageFrame->getX() + imageFrame->getElementWidth(),
                  imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(ctx.cr, imageFrame->getX() + imageFrame->getElementWidth(), imageFrame->getY());


    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, WHITE, 0.2);
    cairo_fill(ctx.cr);
}

void ImageFrameView::drawImage(const Context& ctx, const double alphaForIgnore) const {
    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    auto [image_x, image_y, image_width, image_height] = imageFrame->getImagePosition();

    double xIgnoreP = 0.0;
    double yIgnoreP = 0.0;
    double xDrawP = 1.0;
    double yDrawP = 1.0;

    if (imageFrame->getX() > image_x) {
        const double dif = imageFrame->getX() - image_x;
        xIgnoreP = dif / image_width;
    }

    if (imageFrame->getY() > image_y) {
        const double dif = imageFrame->getY() - image_y;
        yIgnoreP = dif / image_height;
    }

    if (imageFrame->getX() + imageFrame->getElementWidth() < image_x + image_width) {
        const double dif = image_x + image_width - imageFrame->getX() - imageFrame->getElementWidth();
        xDrawP = 1 - (dif / image_width);
    }

    if (imageFrame->getY() + imageFrame->getElementHeight() < image_y + image_height) {
        const double dif = image_y + image_height - imageFrame->getY() - imageFrame->getElementHeight();
        yDrawP = 1 - (dif / image_height);
    }

    imageFrame->drawPartialImage(ctx, xIgnoreP, yIgnoreP, xDrawP, yDrawP, alphaForIgnore);
}

void ImageFrameView::drawFrameHandles(const Context& ctx, Color color) const {
    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    cairo_set_line_width(ctx.cr, 1 / ZOOM);

    const double fx = imageFrame->getX();
    const double fy = imageFrame->getY();
    const double fw = imageFrame->getElementWidth();
    const double fh = imageFrame->getElementHeight();

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // draw Handles for the corners

    // top left
    cairo_new_path(ctx.cr);
    cairo_line_to(ctx.cr, fx - 1, fy - 1);
    cairo_line_to(ctx.cr, fx + 5, fy - 1);
    cairo_line_to(ctx.cr, fx + 5, fy + 2);
    cairo_line_to(ctx.cr, fx + 2, fy + 2);
    cairo_line_to(ctx.cr, fx + 2, fy + 5);
    cairo_line_to(ctx.cr, fx - 1, fy + 5);
    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // top right
    cairo_new_path(ctx.cr);
    cairo_line_to(ctx.cr, fx + fw + 1, fy - 1);
    cairo_line_to(ctx.cr, fx + fw - 5, fy - 1);
    cairo_line_to(ctx.cr, fx + fw - 5, fy + 2);
    cairo_line_to(ctx.cr, fx + fw - 2, fy + 2);
    cairo_line_to(ctx.cr, fx + fw - 2, fy + 5);
    cairo_line_to(ctx.cr, fx + fw + 1, fy + 5);
    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // bottom left
    cairo_new_path(ctx.cr);
    cairo_line_to(ctx.cr, fx - 1, fy + fh + 1);
    cairo_line_to(ctx.cr, fx + 5, fy + fh + 1);
    cairo_line_to(ctx.cr, fx + 5, fy + fh - 2);
    cairo_line_to(ctx.cr, fx + 2, fy + fh - 2);
    cairo_line_to(ctx.cr, fx + 2, fy + fh - 5);
    cairo_line_to(ctx.cr, fx - 1, fy + fh - 5);
    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // top right
    cairo_new_path(ctx.cr);
    cairo_line_to(ctx.cr, fx + fw + 1, fy + fh + 1);
    cairo_line_to(ctx.cr, fx + fw - 5, fy + fh + 1);
    cairo_line_to(ctx.cr, fx + fw - 5, fy + fh - 2);
    cairo_line_to(ctx.cr, fx + fw - 2, fy + fh - 2);
    cairo_line_to(ctx.cr, fx + fw - 2, fy + fh - 5);
    cairo_line_to(ctx.cr, fx + fw + 1, fy + fh - 5);
    cairo_close_path(ctx.cr);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // draw Handles for the sides

    // top
    cairo_rectangle(ctx.cr, fx + fw / 2 - 2.5, fy - 1, 5, 3);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // bottom
    cairo_rectangle(ctx.cr, fx + fw / 2 - 2.5, fy + fh - 2, 5, 3);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // left
    cairo_rectangle(ctx.cr, fx - 1, fy + fh / 2 - 2.5, 3, 5);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);

    Util::cairo_set_source_rgbi(ctx.cr, color);

    // right
    cairo_rectangle(ctx.cr, fx + fw - 2, fy + fh / 2 - 2.5, 3, 5);

    cairo_stroke_preserve(ctx.cr);
    Util::cairo_set_source_rgbi(ctx.cr, BLACK, 1);
    cairo_fill(ctx.cr);
}

void ImageFrameView::setZoomForDrawing(double zoom) const { ZOOM = zoom; }
