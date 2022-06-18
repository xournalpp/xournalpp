#include "DeviceClassConfigGui.h"

#include <string>  // for allocator, char_traits

#include <glib.h>  // for g_ascii_strtoll, g_strdu...

#include "control/DeviceListHelper.h"        // for InputDevice
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for InputDeviceTypeOption

class GladeSearchpath;

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
