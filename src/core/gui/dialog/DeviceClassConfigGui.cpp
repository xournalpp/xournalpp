#include "DeviceClassConfigGui.h"

#include <config.h>

#include "control/DeviceListHelper.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "util/Util.h"
#include "util/i18n.h"

#include "SettingsDialog.h"

DeviceClassConfigGui::DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings,
                                           const InputDevice& device):
        GladeGui(gladeSearchPath, "settingsDeviceClassConfig.glade", "offscreenwindow"),
        settings(settings),
        device(device) {
    GtkWidget* mainGrid = get("deviceClassGrid");
    gtk_window_set_child(GTK_WINDOW(getWindow()), nullptr);
    gtk_box_append(GTK_BOX(w), mainGrid);
    gtk_widget_show(mainGrid);

    this->labelDevice = get("labelDevice");
    this->cbDeviceClass = get("cbDeviceClass");
    gtk_label_set_text(GTK_LABEL(this->labelDevice), (device.getName() + " (" + device.getType() + ")").c_str());

    loadSettings();
}

DeviceClassConfigGui::~DeviceClassConfigGui() = default;

void DeviceClassConfigGui::loadSettings() {
    // Get device class of device if available or
    InputDeviceTypeOption deviceType =
            this->settings->getDeviceClassForDevice(this->device.getName(), this->device.getSource());
    // Use the ID of each option in case the combo box options get rearranged in the future
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(this->cbDeviceClass),
                                g_strdup_printf("%i", static_cast<int>(deviceType)));
}

void DeviceClassConfigGui::show(GtkWindow* parent) {
    // Not implemented! This is not a dialog!
}

void DeviceClassConfigGui::saveSettings() {
    const gchar* deviceClassId = gtk_combo_box_get_active_id(GTK_COMBO_BOX(this->cbDeviceClass));
    g_assert(deviceClassId != nullptr);
    auto deviceClass = static_cast<InputDeviceTypeOption>(g_ascii_strtoll(deviceClassId, nullptr, 10));
    this->settings->setDeviceClassForDevice(this->device.getName(), this->device.getSource(), deviceClass);
}
