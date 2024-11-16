/*
 * Xournal++
 *
 * Control to callibrate the zoom to fit the display DPI
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#include "ZoomCallib.h"

#include <algorithm>  // for min
#include <string>

#include <cairo.h>    // for cairo_set_source_rgb, cairo_fill, cairo_move_to
#include <gdk/gdk.h>  // for GdkWindowAttr, gdk_window_move_resize, gdk_wind...

#include "util/raii/GObjectSPtr.h"

G_DEFINE_TYPE(ZoomCallib, zoomcallib, GTK_TYPE_WIDGET);  // NOLINT // @suppress("Unused static function")

static void zoomcallib_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                               int* minimum_baseline, int* natural_baseline);

// static void zoomcallib_size_allocate(GtkWidget* widget, int width,
//                                      int height,
//                                      int baseline);
// static void zoomcallib_realize(GtkWidget* widget);
static void zoomcallib_snapshot(GtkWidget* widget, GtkSnapshot* sn);
// static void zoomcallib_destroy(GtkObject* object);

void zoomcallib_set_val(ZoomCallib* callib, gint val) {
    callib->val = val;

    if (gtk_widget_is_drawable(GTK_WIDGET(callib))) {
        gtk_widget_queue_draw(GTK_WIDGET(callib));
    }
}

auto zoomcallib_new() -> GtkWidget* { return GTK_WIDGET(g_object_new(zoomcallib_get_type(), nullptr)); }

static void zoomcallib_class_init(ZoomCallibClass* klass) {
    GtkWidgetClass* widget_class = nullptr;

    widget_class = reinterpret_cast<GtkWidgetClass*>(klass);

    // widget_class->realize = zoomcallib_realize;
    // widget_class->size_allocate = zoomcallib_size_allocate;

    widget_class->snapshot = zoomcallib_snapshot;
    widget_class->measure = zoomcallib_measure;
}

static void zoomcallib_init(ZoomCallib* zc) { zc->val = 72; }

static void zoomcallib_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                               int* minimum_baseline, int* natural_baseline) {
    *minimum_baseline = *natural_baseline = -1;  // No baseline
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        *minimum = *natural = 200;
    } else {
        *minimum = *natural = 75;
    }
}

// static void zoomcallib_size_allocate(GtkWidget* widget,   int width,
//                                      int height,
//                                      int baseline) {
//     g_return_if_fail(widget != nullptr);
//     g_return_if_fail(IS_ZOOM_CALLIB(widget));
//
//     gtk_widget_set_allocation(widget, allocation);
//
//     if (gtk_widget_get_realized(widget)) {
//         gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width,
//                                allocation->height);
//     }
// }

// static void zoomcallib_realize(GtkWidget* widget) {
//     GdkWindowAttr attributes;
//     GtkAllocation allocation;
//
//     g_return_if_fail(widget != nullptr);
//     g_return_if_fail(IS_ZOOM_CALLIB(widget));
//
//     gtk_widget_set_realized(widget, true);
//
//     attributes.window_type = GDK_WINDOW_CHILD;
//
//     gtk_widget_get_allocation(widget, &allocation);
//
//     attributes.x = allocation.x;
//     attributes.y = allocation.y;
//     attributes.width = allocation.width;
//     attributes.height = allocation.height;
//
//     attributes.wclass = GDK_INPUT_OUTPUT;
//     attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
//
//     gint attributes_mask = GDK_WA_X | GDK_WA_Y;
//
//     gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes,
//     attributes_mask));
//
//     gdk_window_set_user_data(gtk_widget_get_window(widget), widget);
// }

static void zoomcallib_snapshot(GtkWidget* widget, GtkSnapshot* sn) {
    if (!IS_ZOOM_CALLIB(widget)) {
        g_message("zoomcallib_snapshot without a ZoomCallib");
    }

    cairo_text_extents_t extents;

    int w = gtk_widget_get_width(widget);
    int h = gtk_widget_get_height(widget);
    float hafCm = (static_cast<float>(ZOOM_CALLIB(widget)->val) / 2.54f) / 2.f;


    float height = static_cast<float>(std::min(50, h - 10));

    GdkRGBA black, grey, white;
    gdk_rgba_parse(&black, "black");  // TODO use Util::Color + dark theme stuff
    gdk_rgba_parse(&grey, "grey");    //
    gdk_rgba_parse(&white, "white");  //

    // TODO Find a better way to paint background
    auto rect = GRAPHENE_RECT_INIT(0, 0, static_cast<float>(w), static_cast<float>(h));
    gtk_snapshot_append_color(sn, &white, &rect);


    xoj::util::GObjectSPtr<PangoContext> c(pango_font_map_create_context(pango_cairo_font_map_get_default()),
                                           xoj::util::adopt);
    xoj::util::GObjectSPtr<PangoLayout> layout(pango_layout_new(c.get()), xoj::util::adopt);

    // cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    // cairo_set_font_size(cr, 13);

    auto cx = [x_base = 2.f, hafCm](int i) -> float { return x_base + static_cast<float>(i) * hafCm; };
    for (int i = 0; cx(i) < static_cast<float>(w); ++i) {
        float x = cx(i);

        float y = 0;
        GdkRGBA* c;
        if (i % 2 == 0) {
            c = &black;
            y = height - 3;
        } else {
            c = &grey;
            y = height - 17;
        }
        auto rect = GRAPHENE_RECT_INIT(x, 2.f + height - y, 1.f, y);
        gtk_snapshot_append_color(sn, &black, &rect);

        if (i % 2 == 0 && i != 0 && x < static_cast<float>(w - 20)) {
            std::string txt = std::to_string(i / 2);
            pango_layout_set_text(layout.get(), txt.c_str(), static_cast<int>(txt.length()));
            PangoRectangle extents;
            pango_layout_get_extents(layout.get(), nullptr, &extents);

            gtk_snapshot_save(sn);
            auto pt = GRAPHENE_POINT_INIT(x - .5f * extents.width, h - y - 3.f);
            gtk_snapshot_translate(sn, &pt);
            gtk_snapshot_append_layout(sn, layout.get(), &black);

            gtk_snapshot_restore(sn);
        }
    }
}
