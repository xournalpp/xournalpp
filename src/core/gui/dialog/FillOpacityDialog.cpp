#include "FillOpacityDialog.h"

#include <cmath>

#include <cairo.h>        // for cairo_set_operator, cairo_rectangle, cairo_...
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for gdouble

#include "gui/Builder.h"
#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"

class GladeSearchpath;

constexpr auto UI_FILE = "fillOpacity.glade";
constexpr auto UI_DIALOG_NAME = "fillOpacityDialog";

static int percentToByte(double percent) { return static_cast<int>(std::round(percent * 2.55)); }
static double byteToPercent(int byte) { return byte / 2.55; }

xoj::popup::FillOpacityDialog::FillOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha, FillType type,
                                                 std::function<void(int, FillType)> callback):
        fillType(type), callback(callback) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    previewImage = GTK_IMAGE(builder.get("imgPreview"));
    alphaRange = GTK_RANGE(builder.get("scaleAlpha"));

    gtk_range_set_value(alphaRange, byteToPercent(alpha));

    setPreviewImage(alpha);

    g_signal_connect(alphaRange, "change-value",
                     G_CALLBACK(+[](GtkRange* range, GtkScrollType scroll, gdouble value, FillOpacityDialog* self) {
                         self->setPreviewImage(percentToByte(value));
                         gtk_range_set_value(range, value);
                     }),
                     this);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect(builder.get("btOk"), "clicked", G_CALLBACK(+[](GtkButton*, FillOpacityDialog* self) {
                         self->callback(percentToByte(gtk_range_get_value(self->alphaRange)), self->fillType);
                         gtk_window_close(self->window.get());
                     }),
                     this);

#if GTK_MAJOR_VERSION == 3
    // Widgets are visible by default in gtk4
    gtk_widget_show_all(builder.get("dialog-main-box"));
#endif
}

xoj::popup::FillOpacityDialog::~FillOpacityDialog() = default;

const int PREVIEW_WIDTH = 70;
const int PREVIEW_HEIGTH = 50;
const int PREVIEW_BORDER = 10;

void xoj::popup::FillOpacityDialog::setPreviewImage(int alpha) {
    xoj::util::CairoSurfaceSPtr surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, PREVIEW_WIDTH, PREVIEW_HEIGTH),
                                        xoj::util::adopt);
    xoj::util::CairoSPtr cairo(cairo_create(surface.get()), xoj::util::adopt);
    cairo_t* cr = cairo.get();

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, 255, 255, 255);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, 0, 0x80 / 255.0, 0, alpha / 255.0);
    cairo_rectangle(cr, PREVIEW_BORDER, PREVIEW_BORDER, PREVIEW_WIDTH - PREVIEW_BORDER * 2,
                    PREVIEW_HEIGTH - PREVIEW_BORDER * 2);
    cairo_fill(cr);

    if (fillType == FILL_PEN) {
        cairo_set_line_width(cr, 5);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgb(cr, 0, 0x80 / 255.0, 0);
        cairo_rectangle(cr, PREVIEW_BORDER, PREVIEW_BORDER, PREVIEW_WIDTH - PREVIEW_BORDER * 2,
                        PREVIEW_HEIGTH - PREVIEW_BORDER * 2);
        cairo_stroke(cr);
    }

    xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(
            gdk_pixbuf_get_from_surface(surface.get(), 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGTH), xoj::util::adopt);
    gtk_image_set_from_pixbuf(previewImage, pixbuf.get());
}
