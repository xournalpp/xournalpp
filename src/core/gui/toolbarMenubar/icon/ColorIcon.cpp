#include "ColorIcon.h"

#include <cmath>  // for M_PI

#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"

namespace ColorIcon {
/**
 * Create a new GtkImage with preview color
 */
auto newGtkImage(Color color, int size, bool circle, std::optional<Color> secondaryColor) -> GtkWidget* {
    xoj::util::GObjectSPtr<GdkPixbuf> img(newGdkPixbuf(color, size, circle, secondaryColor));
    GtkWidget* w = gtk_image_new_from_pixbuf(img.get());
    gtk_widget_show(w);
    return w;
}

/**
 * Create a new GdkPixbuf* with preview color
 */
auto newGdkPixbuf(Color color, int size, bool circle, std::optional<Color> secondaryColor)
        -> xoj::util::GObjectSPtr<GdkPixbuf> {
    xoj::util::CairoSurfaceSPtr buf(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size), xoj::util::adopt);
    xoj::util::CairoSPtr cr(cairo_create(buf.get()), xoj::util::adopt);

    Util::cairo_set_source_rgbi(cr.get(), color, 1);

    constexpr double PADDING = 1;

    if (circle) {
        const double mid = .5 * size;
        const double radius = mid - PADDING;
        cairo_arc(cr.get(), mid, mid, radius, 0, 2 * M_PI);
    } else {
        cairo_rectangle(cr.get(), PADDING, PADDING, size - 2 * PADDING, size - 2 * PADDING);
    }
    cairo_fill_preserve(cr.get());

    cairo_set_source_rgba(cr.get(), 0, 0, 0, 1);
    cairo_set_line_width(cr.get(), 1);
    cairo_stroke(cr.get());

    if (secondaryColor) {
        // Draw an indicator for the secondary color
        Util::cairo_set_source_rgbi(cr.get(), secondaryColor.value(), 1);

        const double indicatorMid = size - 2 * PADDING;
        const double indicatorRadius = (0.5 * size) - PADDING;
        // only draws the upper left quarter of the arc
        cairo_arc(cr.get(), indicatorMid, indicatorMid, indicatorRadius, M_PI, M_PI / 2);
        cairo_fill_preserve(cr.get());

        cairo_set_source_rgba(cr.get(), 0, 0, 0, 1);
        cairo_set_line_width(cr.get(), 1);
        cairo_stroke(cr.get());
    }

    return xoj::util::GObjectSPtr<GdkPixbuf>(gdk_pixbuf_get_from_surface(buf.get(), 0, 0, size, size),
                                             xoj::util::adopt);
}
};  // namespace ColorIcon
