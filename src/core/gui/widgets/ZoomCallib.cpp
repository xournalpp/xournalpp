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

G_DEFINE_TYPE(ZoomCallib, zoomcallib, GTK_TYPE_WIDGET);  // NOLINT // @suppress("Unused static function")

static void zoomcallib_get_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width);
static void zoomcallib_get_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height);
static void zoomcallib_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                               int* minimum_baseline, int* natural_baseline);

static void zoomcallib_size_allocate(GtkWidget* widget, int width, int height, int baseline);
static void zoomcallib_realize(GtkWidget* widget);
static void zoomcallib_snapshot(GtkWidget* widget, GtkSnapshot* snapshot);
// TODO (gtk4): replace all cairo drawings with render node based drawing

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
    widget_class->size_allocate = zoomcallib_size_allocate;

    widget_class->snapshot = zoomcallib_snapshot;
    widget_class->measure = zoomcallib_measure;
}

static void zoomcallib_init(ZoomCallib* zc) { zc->val = 72; }

static void zoomcallib_get_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width) {
    *minimal_width = *natural_width = 200;
}

static void zoomcallib_get_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height) {
    *minimal_height = *natural_height = 75;
}

static void zoomcallib_measure(GtkWidget* widget, GtkOrientation orientation, int for_size, int* minimum, int* natural,
                               int* minimum_baseline, int* natural_baseline) {
    auto setter = [](auto* ptr, auto val) {
        if (ptr) {
            *ptr = val;
        }
    };

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        setter(minimum, 75);
        setter(natural, 75);
        setter(minimum_baseline, -1);
        setter(natural_baseline, -1);
    } else {
        setter(minimum, 200);
        setter(natural, 200);
        setter(minimum_baseline, -1);
        setter(natural_baseline, -1);
    }
}

static void zoomcallib_size_allocate(GtkWidget* widget, int width, int height, int baseline) {}

// static void zoomcallib_realize(GtkWidget*) {} /*  */

// https://blog.gtk.org/2020/04/24/custom-widgets-in-gtk-4-drawing/
static void zoomcallib_draw_func(ZoomCallib* widget, cairo_t* cr, int width, int height, gpointer data) {

    cairo_text_extents_t extents;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    gdouble hafCm = (widget->val / 2.54) / 2;

    // Todo (low-prio): find out whats this for:
    auto height2 = height < 50 ? std::max(height - 10, 0) : 50;

    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 13);

    auto cx = [x_base = 2, hafCm](int i) { return x_base + i * hafCm; };
    for (int i = 0; cx(i) < width; ++i) {
        gdouble x = cx(i);

        int y = 0;
        if (i % 2 == 0) {
            cairo_set_source_rgb(cr, 0, 0, 0);
            y = height2 - 3;
        } else {
            cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
            y = height2 - 17;
        }

        cairo_rectangle(cr, x, 2 + height - y, 1, y);

        cairo_fill(cr);

        if (i % 2 == 0 && i != 0 && x < width - 20) {
            cairo_set_source_rgb(cr, 0, 0, 0);

            char* txt = g_strdup_printf("%i", i / 2);
            cairo_text_extents(cr, txt, &extents);

            cairo_move_to(cr, x - extents.width / 2, height - y - 3);

            cairo_show_text(cr, txt);
            g_free(txt);
        }
    }
}

void zoomcallib_snapshot(GtkWidget* widget, GtkSnapshot* snapshot) {
    auto width = gtk_widget_get_width(widget);
    auto height = gtk_widget_get_height(widget);

    graphene_rect_t rect{GRAPHENE_RECT_INIT(0, 0, width, height)};
    auto* cairo = gtk_snapshot_append_cairo(snapshot, &rect);

    zoomcallib_draw_func(ZOOM_CALLIB(widget), cairo, width, height, nullptr);
}
