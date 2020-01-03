#include "DeviceClassConfigGui.h"

#include <config.h>

#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "util/DeviceListHelper.h"

#include "SettingsDialog.h"
#include "Util.h"
#include "i18n.h"

DeviceClassConfigGui::DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings,
                                           const InputDevice& device):
        GladeGui(gladeSearchPath, "settingsDeviceClassConfig.glade", "offscreenwindow"),
        settings(settings),
        device(device) {
    GtkWidget* mainGrid = get("deviceClassGrid");
    gtk_container_remove(GTK_CONTAINER(getWindow()), mainGrid);
    gtk_box_pack_end(GTK_BOX(w), mainGrid, true, true, 0);
    gtk_widget_show_all(mainGrid);

    this->labelDevice = get("labelDevice");
    this->cbDeviceClass = get("cbDeviceClass");
    gtk_label_set_text(GTK_LABEL(this->labelDevice), (device.getName() + " (" + device.getType() + ")").c_str());

    loadSettings();
}

DeviceClassConfigGui::~DeviceClassConfigGui() = default;

void DeviceClassConfigGui::loadSettings() {
    // Get device class of device if available or
    int deviceType = this->settings->getDeviceClassForDevice(this->device.getName(), this->device.getSource());
    gtk_combo_box_set_active(GTK_COMBO_BOX(this->cbDeviceClass), deviceType);
}

void DeviceClassConfigGui::show(GtkWindow* parent) {
    // Not implemented! This is not a dialog!
}

void DeviceClassConfigGui::saveSettings() {
    int deviceClass = gtk_combo_box_get_active(GTK_COMBO_BOX(this->cbDeviceClass));
    this->settings->setDeviceClassForDevice(this->device.getName(), this->device.getSource(), deviceClass);
}
