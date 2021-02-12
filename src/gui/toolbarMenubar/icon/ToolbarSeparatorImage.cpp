#include "ToolbarSeparatorImage.h"

auto ToolbarSeparatorImage::newSeperatorImage() -> GtkWidget* {
    cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 30, 16);
    cairo_t* cr = cairo_create(crImage);

    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 15, 1);
    cairo_line_to(cr, 15, 15);
    cairo_stroke(cr);
    cairo_destroy(cr);

    GtkWidget* w = gtk_image_new_from_surface(crImage);
    cairo_surface_destroy(crImage);
    return w;
}

auto ToolbarSeparatorImage::getNewToolPixbuf() -> GdkPixbuf* {
    cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 30, 16);
    cairo_t* cr = cairo_create(crImage);

    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 15, 1);
    cairo_line_to(cr, 15, 15);
    cairo_stroke(cr);
    cairo_destroy(cr);

    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(crImage, 0, 0, cairo_image_surface_get_width(crImage),
                                                    cairo_image_surface_get_height(crImage));
    cairo_surface_destroy(crImage);

    return pixbuf;
}
