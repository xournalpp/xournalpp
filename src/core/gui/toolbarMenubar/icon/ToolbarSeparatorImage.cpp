#include "ToolbarSeparatorImage.h"

#include <cairo.h>        // for cairo_create, cairo_destroy, cairo_image_su...
#include <gdk/gdk.h>      // for gdk_pixbuf_get_from_surface
#include <glib-object.h>  // for g_object_unref

auto ToolbarSeparatorImage::newImage() -> GtkWidget* {
    GdkPixbuf* pixbuf = ToolbarSeparatorImage::getNewToolPixbuf();
    GtkWidget* w = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    return w;
}

auto ToolbarSeparatorImage::getNewToolPixbuf() -> GdkPixbuf* {
    constexpr auto width = 30;
    constexpr auto height = 30;
    cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(crImage);

    cairo_set_source_rgb(cr, 255, 0, 0);
    cairo_set_line_width(cr, 5);
    cairo_move_to(cr, width / 2, 0);
    cairo_line_to(cr, width / 2, height);
    cairo_stroke(cr);
    cairo_destroy(cr);
    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(crImage, 0, 0, width, height);
    cairo_surface_destroy(crImage);
    return pixbuf;
}
