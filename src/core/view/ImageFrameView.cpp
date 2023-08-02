#include "ImageFrameView.h"

#include <array>  // for std::array

#include "model/ImageFrame.h"         // for ImageFrame
#include "util/raii/CairoWrappers.h"  // for cairo_save

using namespace xoj::view;

ImageFrameView::ImageFrameView(const ImageFrame* imgFrame): imageFrame(imgFrame) {}

ImageFrameView::~ImageFrameView() = default;

void ImageFrameView::draw(const xoj::view::Context& ctx) const {
    if (imageFrame->inEditing()) {
        return;  // drawing is handled by ImageFrameEditorView
    }

    xoj::util::CairoSaveGuard const saveGuard(ctx.cr);  // cairo_save

    if (imageFrame->couldBeEdited()) {
        drawFrame(ctx.cr, WHITE);
        if (imageFrame->hasImage()) {
            drawImage(ctx.cr, 0.0);
        }
        drawFrameHandles(ctx.cr, WHITE);
    } else {
        if (imageFrame->hasImage()) {
            drawImage(ctx.cr, 0.0);
        } else {
            drawFrame(ctx.cr, WHITE);
            drawFrameHandles(ctx.cr, BLACK);
        }
    }
}

void ImageFrameView::drawFrame(cairo_t* cr, Color color) const {
    xoj::util::CairoSaveGuard const saveGuard(cr);  // cairo_save

    cairo_set_line_width(cr, 1 / ZOOM);

    Util::cairo_set_source_rgbi(cr, BLACK);
    std::array<double, 2> dashes = {6.0, 4.0};
    cairo_set_dash(cr, dashes.data(), dashes.size(), 0.0);


    cairo_new_path(cr);

    cairo_line_to(cr, imageFrame->getX(), imageFrame->getY());
    cairo_line_to(cr, imageFrame->getX(), imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(cr, imageFrame->getX() + imageFrame->getElementWidth(),
                  imageFrame->getY() + imageFrame->getElementHeight());
    cairo_line_to(cr, imageFrame->getX() + imageFrame->getElementWidth(), imageFrame->getY());


    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, WHITE, 0.2);
    cairo_fill(cr);
}

void ImageFrameView::drawImage(cairo_t* cr, const double alphaForIgnore) const {
    xoj::util::CairoSaveGuard const saveGuard(cr);  // cairo_save

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

    imageFrame->drawPartialImage(cr, xIgnoreP, yIgnoreP, xDrawP, yDrawP, alphaForIgnore);
}

void ImageFrameView::drawFrameHandles(cairo_t* cr, Color color) const {
    xoj::util::CairoSaveGuard const saveGuard(cr);  // cairo_save

    cairo_set_line_width(cr, 1 / ZOOM);

    const double fx = imageFrame->getX();
    const double fy = imageFrame->getY();
    const double fw = imageFrame->getElementWidth();
    const double fh = imageFrame->getElementHeight();

    Util::cairo_set_source_rgbi(cr, color);

    // draw Handles for the corners

    // top left
    cairo_new_path(cr);
    cairo_line_to(cr, fx - 1, fy - 1);
    cairo_line_to(cr, fx + 5, fy - 1);
    cairo_line_to(cr, fx + 5, fy + 2);
    cairo_line_to(cr, fx + 2, fy + 2);
    cairo_line_to(cr, fx + 2, fy + 5);
    cairo_line_to(cr, fx - 1, fy + 5);
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // top right
    cairo_new_path(cr);
    cairo_line_to(cr, fx + fw + 1, fy - 1);
    cairo_line_to(cr, fx + fw - 5, fy - 1);
    cairo_line_to(cr, fx + fw - 5, fy + 2);
    cairo_line_to(cr, fx + fw - 2, fy + 2);
    cairo_line_to(cr, fx + fw - 2, fy + 5);
    cairo_line_to(cr, fx + fw + 1, fy + 5);
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // bottom left
    cairo_new_path(cr);
    cairo_line_to(cr, fx - 1, fy + fh + 1);
    cairo_line_to(cr, fx + 5, fy + fh + 1);
    cairo_line_to(cr, fx + 5, fy + fh - 2);
    cairo_line_to(cr, fx + 2, fy + fh - 2);
    cairo_line_to(cr, fx + 2, fy + fh - 5);
    cairo_line_to(cr, fx - 1, fy + fh - 5);
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // top right
    cairo_new_path(cr);
    cairo_line_to(cr, fx + fw + 1, fy + fh + 1);
    cairo_line_to(cr, fx + fw - 5, fy + fh + 1);
    cairo_line_to(cr, fx + fw - 5, fy + fh - 2);
    cairo_line_to(cr, fx + fw - 2, fy + fh - 2);
    cairo_line_to(cr, fx + fw - 2, fy + fh - 5);
    cairo_line_to(cr, fx + fw + 1, fy + fh - 5);
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // draw Handles for the sides

    // top
    cairo_rectangle(cr, fx + fw / 2 - 2.5, fy - 1, 5, 3);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // bottom
    cairo_rectangle(cr, fx + fw / 2 - 2.5, fy + fh - 2, 5, 3);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // left
    cairo_rectangle(cr, fx - 1, fy + fh / 2 - 2.5, 3, 5);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, color);

    // right
    cairo_rectangle(cr, fx + fw - 2, fy + fh / 2 - 2.5, 3, 5);

    cairo_stroke_preserve(cr);
    Util::cairo_set_source_rgbi(cr, BLACK, 1);
    cairo_fill(cr);
}

void ImageFrameView::setZoomForDrawing(double zoom) const { ZOOM = zoom; }
