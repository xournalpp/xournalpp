#include "CustomThicknessDialog.h"

CustomThicknessDialog::CustomThicknessDialog(GladeSearchpath* gladeSearchPath, int thickness):
        GladeGui(gladeSearchPath, "customThickness.glade", "customThicknessDialog") {
    GtkWidget* scaleThickness = get("scaleThickness");

    gtk_range_set_value(GTK_RANGE(scaleThickness), thickness);

    g_signal_connect(scaleThickness, "change-value",
                     G_CALLBACK(+[](GtkRange* range, GtkScrollType scroll, gdouble value, CustomThicknessDialog* self) {
                         gtk_range_set_value(range, value);
                     }),
                     this);
}

CustomThicknessDialog::~CustomThicknessDialog() = default;

double CustomThicknessDialog::getResultThickness() const { return static_cast<double>(resultThickness); }

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
