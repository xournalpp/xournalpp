#include "DeviceClassConfigGui.h"

#include <string>  // for allocator, char_traits

#include <glib.h>  // for g_ascii_strtoll, g_strdu...

#include "control/DeviceListHelper.h"        // for InputDevice
#include "control/settings/Settings.h"       // for Settings
#include "control/settings/SettingsEnums.h"  // for InputDeviceTypeOption
#include "gui/Builder.h"                     // for Builder
#include "util/Assert.h"                     // for xoj_assert
#include "util/gtk4_helper.h"                // for gtk_box_append

class GladeSearchpath;

constexpr auto UI_FILE = "settingsDeviceClassConfig.glade";
constexpr auto UI_WIDGET_NAME = "deviceClassBox";

DeviceClassConfigGui::DeviceClassConfigGui(GladeSearchpath* gladeSearchPath, GtkBox* box, Settings* settings,
                                           bool showApplyBtn):
        settings(settings) {
    Builder builder(gladeSearchPath, UI_FILE);
    gtk_box_append(box, builder.get(UI_WIDGET_NAME));  // box takes ownership of it all!

    this->labelDevice = builder.get("labelDevice");
    this->cbDeviceClass = builder.get("cbDeviceClass");
    auto* applyBtn = builder.get("btnApply");

    gtk_widget_set_visible(applyBtn, showApplyBtn);
    if (showApplyBtn) {
        g_signal_connect(applyBtn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer d) {
                             static_cast<DeviceClassConfigGui*>(d)->saveSettings();
                         }),
                         this);
    }
}

DeviceClassConfigGui::~DeviceClassConfigGui() = default;

void DeviceClassConfigGui::setDevice(const InputDevice& d) {
    this->device = d;
    gtk_label_set_text(GTK_LABEL(this->labelDevice), (device.getName() + " (" + device.getType() + ")").c_str());

    loadSettings();
}

void DeviceClassConfigGui::loadSettings() {
    // Get device class of device if available or
    InputDeviceTypeOption deviceType =
            this->settings->getDeviceClassForDevice(this->device.getName(), this->device.getSource());
    // Use the ID of each option in case the combo box options get rearranged in the future
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(this->cbDeviceClass),
                                std::to_string(static_cast<int>(deviceType)).c_str());
}

void DeviceClassConfigGui::saveSettings() {
    const gchar* deviceClassId = gtk_combo_box_get_active_id(GTK_COMBO_BOX(this->cbDeviceClass));
    xoj_assert(deviceClassId != nullptr);
    auto deviceClass = static_cast<InputDeviceTypeOption>(g_ascii_strtoll(deviceClassId, nullptr, 10));
    this->settings->setDeviceClassForDevice(this->device.getName(), this->device.getSource(), deviceClass);
}
