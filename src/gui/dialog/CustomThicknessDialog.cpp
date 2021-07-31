#include "CustomThicknessDialog.h"

#include <cmath>

static inline double translateFromScale(double value) { return pow(2, value); }

static inline double translateToScale(double value) { return log2(value); }

CustomThicknessDialog::CustomThicknessDialog(GladeSearchpath* gladeSearchPath, double thickness):
        GladeGui(gladeSearchPath, "customThickness.glade", "customThicknessDialog") {
    GtkWidget* scaleThickness = get("scaleThickness");

    gtk_scale_add_mark(reinterpret_cast<GtkScale*>(scaleThickness), translateToScale(0.1), GTK_POS_TOP, nullptr);
    gtk_scale_add_mark(reinterpret_cast<GtkScale*>(scaleThickness), translateToScale(1.0), GTK_POS_TOP, nullptr);
    gtk_scale_add_mark(reinterpret_cast<GtkScale*>(scaleThickness), translateToScale(10.0), GTK_POS_TOP, nullptr);
    gtk_scale_add_mark(reinterpret_cast<GtkScale*>(scaleThickness), translateToScale(100.0), GTK_POS_TOP, nullptr);

    gtk_range_set_value(GTK_RANGE(scaleThickness), translateToScale(thickness));

    g_signal_connect(scaleThickness, "change-value",
                     G_CALLBACK(+[](GtkRange* range, GtkScrollType scroll, gdouble value, CustomThicknessDialog* self) {
                         gtk_range_set_value(range, value);
                     }),
                     this);

    g_signal_connect(scaleThickness, "format-value", G_CALLBACK(+[](GtkScale* scale, gdouble value) {
                         return g_strdup_printf("%.2f", gtk_scale_get_digits(scale), translateFromScale(value));
                     }),
                     this);
}

CustomThicknessDialog::~CustomThicknessDialog() = default;

double CustomThicknessDialog::getResultThickness() const { return translateFromScale(resultThickness); }

void CustomThicknessDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int result = gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);

    // OK Button
    if (result == 1) {
        GtkWidget* scaleThickness = get("scaleThickness");
        resultThickness = gtk_range_get_value(GTK_RANGE(scaleThickness));
    } else {
        resultThickness = NAN;
    }
}
