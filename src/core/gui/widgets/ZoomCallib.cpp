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

#include <cairo.h>    // for cairo_set_source_rgb, cairo_fill, cairo_move_to
#include <gdk/gdk.h>  // for GdkWindowAttr, gdk_window_move_resize, gdk_wind...

G_DEFINE_TYPE(ZoomCallib, zoomcallib, GTK_TYPE_WIDGET);  // NOLINT // @suppress("Unused static function")

static void zoomcallib_get_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width);
static void zoomcallib_get_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height);

static void zoomcallib_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static void zoomcallib_realize(GtkWidget* widget);
static auto zoomcallib_draw(GtkWidget* widget, cairo_t* cr) -> gboolean;
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

    widget_class->realize = zoomcallib_realize;
    widget_class->size_allocate = zoomcallib_size_allocate;

    widget_class->draw = zoomcallib_draw;
    widget_class->get_preferred_width = zoomcallib_get_preferred_width;
    widget_class->get_preferred_height = zoomcallib_get_preferred_height;
}

static void zoomcallib_init(ZoomCallib* zc) { zc->val = 72; }

static void zoomcallib_get_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width) {
    *minimal_width = *natural_width = 200;
}

static void zoomcallib_get_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height) {
    *minimal_height = *natural_height = 75;
}

static void zoomcallib_size_allocate(GtkWidget* widget, GtkAllocation* allocation) {
    g_return_if_fail(widget != nullptr);
    g_return_if_fail(IS_ZOOM_CALLIB(widget));
    g_return_if_fail(allocation != nullptr);

    gtk_widget_set_allocation(widget, allocation);

    if (gtk_widget_get_realized(widget)) {
        gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y, allocation->width,
                               allocation->height);
    }
}

static void zoomcallib_realize(GtkWidget* widget) {
    GdkWindowAttr attributes;
    guint attributes_mask = 0;
    GtkAllocation allocation;

    g_return_if_fail(widget != nullptr);
    g_return_if_fail(IS_ZOOM_CALLIB(widget));

    gtk_widget_set_realized(widget, true);

    attributes.window_type = GDK_WINDOW_CHILD;

    gtk_widget_get_allocation(widget, &allocation);

    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;

    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y;

    gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));

    gdk_window_set_user_data(gtk_widget_get_window(widget), widget);
}

static auto zoomcallib_draw(GtkWidget* widget, cairo_t* cr) -> gboolean {
    if (!IS_ZOOM_CALLIB(widget)) {
        g_message("zoomcallib_draw without a ZoomCallib");
    }

    cairo_text_extents_t extents;
    GtkAllocation allocation;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    gtk_widget_get_allocation(widget, &allocation);

    gdouble hafCm = (ZOOM_CALLIB(widget)->val / 2.54) / 2;

    int h = allocation.height;
    int height = 50;
    if (h < height) {
        height = allocation.height - 10;
    }


    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);

    auto cx = [x_base = 2, hafCm](int i) { return x_base + i * hafCm; };
    for (int i = 0; cx(i) < allocation.width; ++i) {
        gdouble x = cx(i);

        int y = 0;
        if (i % 2 == 0) {
            cairo_set_source_rgb(cr, 0, 0, 0);
            y = height - 3;
        } else {
            cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
            y = height - 17;
        }

        cairo_rectangle(cr, x, 2 + h - y, 1, y);

        cairo_fill(cr);

        if (i % 2 == 0 && i != 0 && x < allocation.width - 20) {
            cairo_set_source_rgb(cr, 0, 0, 0);

            char* txt = g_strdup_printf("%i", i / 2);
            cairo_text_extents(cr, txt, &extents);

            cairo_move_to(cr, x - extents.width / 2, h - y - 3);

            cairo_show_text(cr, txt);
            g_free(txt);
        }
    }

    return true;
}
