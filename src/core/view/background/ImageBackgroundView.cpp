#include "ImageBackgroundView.h"

#include <gdk-pixbuf/gdk-pixbuf.h>  // for gdk_pixbuf_get_height
#include <gdk/gdk.h>                // for gdk_cairo_set_source_pixbuf

#include "model/BackgroundImage.h"           // for BackgroundImage
#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

ImageBackgroundView::ImageBackgroundView(double pageWidth, double pageHeight, const BackgroundImage& image):
        BackgroundView(pageWidth, pageHeight), image(image) {}

void ImageBackgroundView::draw(cairo_t* cr) const {
    const GdkPixbuf* pixbuff = this->image.getPixbuf();
    if (pixbuff) {
        cairo_matrix_t matrix = {0};
        cairo_get_matrix(cr, &matrix);

        int width = gdk_pixbuf_get_width(pixbuff);
        int height = gdk_pixbuf_get_height(pixbuff);

        double sx = this->pageWidth / width;
        double sy = this->pageHeight / height;

        cairo_scale(cr, sx, sy);

        gdk_cairo_set_source_pixbuf(cr, pixbuff, 0, 0);
        cairo_paint(cr);

        cairo_set_matrix(cr, &matrix);
    }
}
