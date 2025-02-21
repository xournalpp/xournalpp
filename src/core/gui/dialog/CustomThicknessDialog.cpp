#include "CustomThicknessDialog.h"

#include "gui/Builder.h"

constexpr auto UI_FILE = "customThickness.glade";
constexpr auto UI_DIALOG_NAME = "customThicknessDialog";

// static inline void buildLabel(Builder& builder, OpacityFeature opacityFeature) {
//     // Used to set the label of the dialog in the form of:
//     // <b>{toolOptionsDesc}</b>\n
//     // Select opacity for: {opacityFeatureDesc}
//     std::string toolOptionsDesc;
//     std::string selectOpacityFor = _("Select opacity for: ");
//     std::string opacityFeatureDesc;
//
//     switch (opacityFeature) {
//         case OPACITY_FILL_HIGHLIGHTER:
//             toolOptionsDesc = _("Highlighter Options");
//             opacityFeatureDesc = _("Fill color");
//             break;
//         case OPACITY_FILL_PEN:
//             toolOptionsDesc = _("Pen Options");
//             opacityFeatureDesc = _("Fill color");
//             break;
//         case OPACITY_SELECT_PDF_TEXT_MARKER:
//             toolOptionsDesc = _("PDF Text Options");
//             opacityFeatureDesc = _("PDF Text Marker");
//             break;
//         default:
//             g_warning("No opacityFeature description set for '%s'", opacityFeatureToString(opacityFeature).c_str());
//             Stacktrace::printStacktrace();
//             break;
//     }
//     gtk_label_set_label(GTK_LABEL(builder.get("label1")),
//                         FC(_F("<b>{1}</b>\n{2}{3}") % toolOptionsDesc % selectOpacityFor % opacityFeatureDesc));
// }

xoj::popup::CustomThicknessDialog::CustomThicknessDialog(GladeSearchpath* gladeSearchPath, double thickness,
                                                         CustomToolSizeFeature feature,
                                                         std::function<void(double, CustomToolSizeFeature)> callback):
        customToolSizeFeature(feature), callback(callback) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    // buildLabel(builder, customToolSizeFeature);
    sizeRange = GTK_RANGE(builder.get("scaleSize"));

    gtk_range_set_value(sizeRange, thickness);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect(builder.get("btOk"), "clicked", G_CALLBACK(+[](GtkButton*, CustomThicknessDialog* self) {
                         self->callback(gtk_range_get_value(self->sizeRange), self->customToolSizeFeature);
                         gtk_window_close(self->window.get());
                     }),
                     this);

#if GTK_MAJOR_VERSION == 3
    // Widgets are visible by default in gtk4
    gtk_widget_show_all(builder.get("dialog-main-box"));
#endif
}

xoj::popup::CustomThicknessDialog::~CustomThicknessDialog() = default;
