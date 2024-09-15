#include "ToolbarSeparatorImage.h"

#include <cairo.h>        // for cairo_create, cairo_destroy, cairo_image_su...
#include <gdk/gdk.h>      // for gdk_pixbuf_get_from_surface
#include <glib-object.h>  // for g_object_unref
#include <util/Color.h>   // for Colors::red, Colors::gray
#include <util/Util.h>    // for cairo_set_source_rgbi

constexpr double LINE_WIDTH = 5.;
constexpr double MARGIN = 3.;
constexpr double TAIL_SIZE = 5.;

static auto newGdkPixbuf(SeparatorType separator) -> xoj::util::GObjectSPtr<GdkPixbuf> {
    constexpr auto width = 30;
    constexpr auto height = 30;
    cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(crImage);

    Util::cairo_set_source_rgbi(cr, Colors::red);
    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_move_to(cr, width / 2, 0);
    cairo_line_to(cr, width / 2, height);
    cairo_stroke(cr);
    if (separator == SeparatorType::SPACER) {  // add double arrow tails
        Util::cairo_set_source_rgbi(cr, Colors::gray);

        cairo_move_to(cr, MARGIN + TAIL_SIZE, height / 2 - TAIL_SIZE);
        cairo_line_to(cr, MARGIN, height / 2);
        cairo_line_to(cr, MARGIN + TAIL_SIZE, height / 2 + TAIL_SIZE);

        cairo_move_to(cr, width - MARGIN - TAIL_SIZE, height / 2 - TAIL_SIZE);
        cairo_line_to(cr, width - MARGIN, height / 2);
        cairo_line_to(cr, width - MARGIN - TAIL_SIZE, height / 2 + TAIL_SIZE);

        cairo_stroke(cr);
    }
    cairo_destroy(cr);
    GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface(crImage, 0, 0, width, height);
    cairo_surface_destroy(crImage);
    return xoj::util::GObjectSPtr<GdkPixbuf>(pixbuf, xoj::util::adopt);
}

auto ToolbarSeparatorImage::newImage(SeparatorType separator) -> GtkWidget* {
    return gtk_image_new_from_pixbuf(newGdkPixbuf(separator).get());
}

auto ToolbarSeparatorImage::newGdkPaintable(SeparatorType separator) -> xoj::util::GObjectSPtr<GdkPaintable> {
    return xoj::util::GObjectSPtr<GdkPaintable>(
            GDK_PAINTABLE(gdk_texture_new_for_pixbuf(newGdkPixbuf(separator).get())), xoj::util::adopt);
}
