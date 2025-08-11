#include "SelectOpacityDialog.h"

#include <cairo.h>        // for cairo_set_operator, cairo_rectangle, cairo_...
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for gdouble

#include "gui/Builder.h"
#include "util/Stacktrace.h"
#include "util/i18n.h"
#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"  // for round_cast

class GladeSearchpath;

constexpr auto UI_FILE = "selectOpacity.ui";
constexpr auto UI_DIALOG_NAME = "selectOpacityDialog";

static int percentToByte(double percent) { return round_cast<int>(percent * 2.55); }
static double byteToPercent(int byte) { return byte / 2.55; }

static inline void buildLabel(Builder& builder, OpacityFeature opacityFeature) {
    // Used to set the label of the dialog in the form of:
    // <b>{toolOptionsDesc}</b>\n
    // Select opacity for: {opacityFeatureDesc}
    std::string toolOptionsDesc;
    std::string selectOpacityFor = _("Select opacity for: ");
    std::string opacityFeatureDesc;

    switch (opacityFeature) {
        case OPACITY_FILL_HIGHLIGHTER:
            toolOptionsDesc = _("Highlighter Options");
            opacityFeatureDesc = _("Fill color");
            break;
        case OPACITY_FILL_PEN:
            toolOptionsDesc = _("Pen Options");
            opacityFeatureDesc = _("Fill color");
            break;
        case OPACITY_SELECT_PDF_TEXT_MARKER:
            toolOptionsDesc = _("PDF Text Options");
            opacityFeatureDesc = _("PDF Text Marker");
            break;
        default:
            g_warning("No opacityFeature description set for '%s'", opacityFeatureToString(opacityFeature).c_str());
            Stacktrace::printStacktrace();
            break;
    }
    gtk_label_set_label(GTK_LABEL(builder.get("label1")),
                        FC(_F("<b>{1}</b>\n{2}{3}") % toolOptionsDesc % selectOpacityFor % opacityFeatureDesc));
}

xoj::popup::SelectOpacityDialog::SelectOpacityDialog(GladeSearchpath* gladeSearchPath, int alpha,
                                                     OpacityFeature feature,
                                                     std::function<void(int, OpacityFeature)> callback):
        opacityFeature(feature), callback(callback) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    buildLabel(builder, opacityFeature);
    previewImage = GTK_PICTURE(builder.get("imgPreview"));
    alphaRange = GTK_RANGE(builder.get("scaleAlpha"));

    gtk_range_set_value(alphaRange, byteToPercent(alpha));

    setPreviewImage(alpha);


    g_signal_connect(alphaRange, "value-changed", G_CALLBACK(+[](GtkRange* range, gpointer self) {
                         static_cast<SelectOpacityDialog*>(self)->setPreviewImage(
                                 percentToByte(round_cast<int>(gtk_range_get_value(range))));
                     }),
                     this);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect(builder.get("btOk"), "clicked", G_CALLBACK(+[](GtkButton*, SelectOpacityDialog* self) {
                         self->callback(percentToByte(gtk_range_get_value(self->alphaRange)), self->opacityFeature);
                         gtk_window_close(self->window.get());
                     }),
                     this);
}

xoj::popup::SelectOpacityDialog::~SelectOpacityDialog() = default;

const int PREVIEW_WIDTH = 70;
const int PREVIEW_HEIGTH = 50;
const int PREVIEW_BORDER = 10;

void xoj::popup::SelectOpacityDialog::setPreviewImage(int alpha) {
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

    if (opacityFeature == OPACITY_FILL_PEN) {
        cairo_set_line_width(cr, 5);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgb(cr, 0, 0x80 / 255.0, 0);
        cairo_rectangle(cr, PREVIEW_BORDER, PREVIEW_BORDER, PREVIEW_WIDTH - PREVIEW_BORDER * 2,
                        PREVIEW_HEIGTH - PREVIEW_BORDER * 2);
        cairo_stroke(cr);
    }

    xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(
            gdk_pixbuf_get_from_surface(surface.get(), 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGTH), xoj::util::adopt);
    gtk_picture_set_pixbuf(previewImage, pixbuf.get());
}
