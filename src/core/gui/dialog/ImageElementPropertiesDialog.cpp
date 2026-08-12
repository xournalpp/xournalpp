#include "ImageElementPropertiesDialog.h"

#include <utility>

#include <glib-object.h>

#include "control/settings/Settings.h"
#include "gui/Builder.h"
#include "model/FormatDefinitions.h"

class GladeSearchpath;

constexpr auto UI_FILE = "imageProperties.glade";
constexpr auto UI_DIALOG_NAME = "imagePropertiesDialog";

using namespace xoj::popup;

ImageElementPropertiesDialog::ImageElementPropertiesDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
                                                           double width, double height,
                                                           std::function<void(double, double)> callback):
        settings(settings), callback(std::move(callback)) {
    Builder builder(gladeSearchPath, UI_FILE);
    this->window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    this->widthSpin = GTK_SPIN_BUTTON(builder.get("spinWidth"));
    this->heightSpin = GTK_SPIN_BUTTON(builder.get("spinHeight"));
    this->keepAspectButton = GTK_CHECK_BUTTON(builder.get("cbKeepAspectRatio"));

    const int unitIndex = settings->getSizeUnitIndex();
    this->scale = XOJ_UNITS[unitIndex].scale;
    this->aspectRatio = height > 0 ? width / height : 1.0;
    this->initialWidth = width / this->scale;
    this->initialHeight = height / this->scale;

    gtk_label_set_text(GTK_LABEL(builder.get("lbUnitValue")), XOJ_UNITS[unitIndex].name);
    setDimensions(this->initialWidth, this->initialHeight);

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](ImageElementPropertiesDialog* self) {
                                 gtk_spin_button_update(self->widthSpin);
                                 gtk_spin_button_update(self->heightSpin);
                                 if (gtk_check_button_get_active(self->keepAspectButton) && self->aspectRatio > 0) {
                                     if (self->lastEditedDimension ==
                                         ImageElementPropertiesDialog::LastEditedDimension::Width) {
                                         self->applyAspectRatioFromWidth();
                                     } else {
                                         self->applyAspectRatioFromHeight();
                                     }
                                 }
                                 double width = gtk_spin_button_get_value(self->widthSpin) * self->scale;
                                 double height = gtk_spin_button_get_value(self->heightSpin) * self->scale;
                                 self->callback(width, height);
                                 gtk_window_close(self->window.get());
                             }),
                             this);

    g_signal_connect(this->widthSpin, "value-changed", G_CALLBACK(+[](GtkSpinButton* spin, gpointer userData) {
                         auto* self = static_cast<ImageElementPropertiesDialog*>(userData);
                         if (self->ignoreValueChange) {
                             return;
                         }
                         self->lastEditedDimension = ImageElementPropertiesDialog::LastEditedDimension::Width;
                     }),
                     this);
    g_signal_connect(this->heightSpin, "value-changed", G_CALLBACK(+[](GtkSpinButton* spin, gpointer userData) {
                         auto* self = static_cast<ImageElementPropertiesDialog*>(userData);
                         if (self->ignoreValueChange) {
                             return;
                         }
                         self->lastEditedDimension = ImageElementPropertiesDialog::LastEditedDimension::Height;
                     }),
                     this);
    g_signal_connect(this->widthSpin, "focus-out-event", G_CALLBACK(+[](GtkWidget*, GdkEventFocus*, gpointer userData) {
                         auto* self = static_cast<ImageElementPropertiesDialog*>(userData);
                         gtk_spin_button_update(self->widthSpin);
                         self->applyAspectRatioFromWidth();
                         return FALSE;
                     }),
                     this);
    g_signal_connect(this->heightSpin, "focus-out-event", G_CALLBACK(+[](GtkWidget*, GdkEventFocus*, gpointer userData) {
                         auto* self = static_cast<ImageElementPropertiesDialog*>(userData);
                         gtk_spin_button_update(self->heightSpin);
                         self->applyAspectRatioFromHeight();
                         return FALSE;
                     }),
                     this);
}

ImageElementPropertiesDialog::~ImageElementPropertiesDialog() = default;

void ImageElementPropertiesDialog::setDimensions(double width, double height) {
    this->ignoreValueChange = true;
    gtk_spin_button_set_value(this->widthSpin, width);
    gtk_spin_button_set_value(this->heightSpin, height);
    this->ignoreValueChange = false;
}

void ImageElementPropertiesDialog::applyAspectRatioFromWidth() {
    if (this->ignoreValueChange || !gtk_check_button_get_active(this->keepAspectButton) || this->aspectRatio <= 0) {
        return;
    }

    this->ignoreValueChange = true;
    gtk_spin_button_set_value(this->heightSpin, gtk_spin_button_get_value(this->widthSpin) / this->aspectRatio);
    this->ignoreValueChange = false;
}

void ImageElementPropertiesDialog::applyAspectRatioFromHeight() {
    if (this->ignoreValueChange || !gtk_check_button_get_active(this->keepAspectButton) || this->aspectRatio <= 0) {
        return;
    }

    this->ignoreValueChange = true;
    gtk_spin_button_set_value(this->widthSpin, gtk_spin_button_get_value(this->heightSpin) * this->aspectRatio);
    this->ignoreValueChange = false;
}
