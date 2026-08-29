#include "ImageView.h"

#include <cairo.h>  // for cairo_image_surface_get_height, cairo_image...

#include "model/Image.h"  // for Image
#include "util/Matrix.h"  // for Matrix
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

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    image->getTransformation().transformCairo(cr);

    cairo_set_source_surface(cr, img, 0, 0);
    // make images translucent when highlighting elements with audio, as they can not have audio
    if (ctx.fadeOutNonAudio) {
        cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
    } else {
        cairo_paint(cr);
    }

    cairo_restore(cr);
}
