#include "ToolbarSeparatorImage.h"

auto ToolbarSeparatorImage::newSepeartorImage() -> GtkWidget* {
    cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 30, 30);
    cairo_t* cr = cairo_create(crImage);

    cairo_set_source_rgb(cr, 255, 0, 0);
    cairo_set_line_width(cr, 5);
    cairo_move_to(cr, 15, 0);
    cairo_line_to(cr, 15, 30);
    cairo_stroke(cr);
    cairo_destroy(cr);

    GtkWidget* w = gtk_image_new_from_surface(crImage);
    cairo_surface_destroy(crImage);
    return w;
}
