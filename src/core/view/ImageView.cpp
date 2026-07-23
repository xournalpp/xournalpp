#include "ImageView.h"

#include <cairo.h>  // for cairo_image_surface_get_height, cairo_image...

#include "model/Image.h"  // for Image
#include "util/Point.h"   // for Point
#include "view/View.h"    // for Context, OPACITY_NO_AUDIO, view

using namespace xoj::view;

ImageView::ImageView(const Image* image): image(image) {}

ImageView::~ImageView() = default;

void ImageView::draw(const Context& ctx) const {
    cairo_t* cr = ctx.cr;

    cairo_save(cr);

    cairo_surface_t* img = image->getImage();
    if (!img) {
        g_warning("Image could not be rendered");
        return;
    }

    int width = cairo_image_surface_get_width(img);
    int height = cairo_image_surface_get_height(img);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    const auto& box = image->getBoundingBox();
    double xFactor = box.width / width;
    double yFactor = box.height / height;

    cairo_scale(cr, xFactor, yFactor);

    auto [x, y] = image->getOrigin();
    cairo_set_source_surface(cr, img, x / xFactor, y / yFactor);
    // make images translucent when highlighting elements with audio, as they can not have audio
    if (ctx.fadeOutNonAudio) {
        cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
    } else {
        cairo_paint(cr);
    }

    cairo_restore(cr);
}
