#include "DeviceListDialog.h"

#include "control/settings/Settings.h"
#include "gui/Builder.h"
#include "util/gtk4_helper.h"  // for gtk_box_append, ...

#include "DeviceClassConfigGui.h"

constexpr auto UI_FILE_NAME = "deviceList.glade";
constexpr auto UI_MAIN_ID = "deviceListDialog";
constexpr auto UI_CONTENT_BOX = "contentBox";

DeviceListDialog::DeviceListDialog(GladeSearchpath* gladeSearchPath, Settings* settings): settings(settings) {

    Builder builder(gladeSearchPath, UI_FILE_NAME);
    window.reset(GTK_WINDOW(builder.get(UI_MAIN_ID)));

    auto deviceList = DeviceListHelper::getDeviceList(settings);
    auto* box = GTK_BOX(builder.get(UI_CONTENT_BOX));
    for (const InputDevice& inputDevice: deviceList) {
        auto& entry = this->deviceClassConfigs.emplace_back(gladeSearchPath, box, settings);
        entry.setDevice(inputDevice);
    }

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](DeviceListDialog* self) {
                                 self->saveSettings();
                                 gtk_window_close(self->window.get());
                             }),
                             this);
}

void DeviceListDialog::saveSettings() {
    this->settings->transactionStart();
    for (auto& deviceClassConfigGui: this->deviceClassConfigs) {
        deviceClassConfigGui.saveSettings();
    }
    this->settings->transactionEnd();
}
