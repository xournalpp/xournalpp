#include "PluginDialog.h"

#ifdef ENABLE_PLUGINS

#include <algorithm>
#include <string>  // for string

#include "control/settings/Settings.h"     // for Settings
#include "gui/Builder.h"
#include "gui/dialog/PluginDialogEntry.h"  // for PluginDialogEntry
#include "plugin/PluginController.h"       // for PluginController

class GladeSearchpath;
class Plugin;

constexpr auto UI_FILE = "plugin.ui";
constexpr auto UI_DIALOG_NAME = "pluginDialog";

PluginDialog::PluginDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
                           const std::vector<std::unique_ptr<Plugin>>& pluginList):
        settings(settings) {
    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));

    GtkBox* pluginBox = GTK_BOX(builder.get("pluginBox"));
    std::transform(pluginList.begin(), pluginList.end(), std::back_inserter(this->plugins),
                   [&](auto& p) { return std::make_unique<PluginDialogEntry>(p.get(), gladeSearchPath, pluginBox); });

    g_signal_connect_swapped(builder.get("btCancel"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
    g_signal_connect_swapped(builder.get("btOk"), "clicked", G_CALLBACK(+[](PluginDialog* self) {
                                 self->saveSettings();
                                 gtk_window_close(self->window.get());
                             }),
                             this);
}

void PluginDialog::saveSettings() {
    std::string pluginEnabled;
    std::string pluginDisabled;

    // Save plugin settings
    for (auto&& bcg: this->plugins) { bcg->saveSettings(pluginEnabled, pluginDisabled); }

    settings->setPluginEnabled(pluginEnabled);
    settings->setPluginDisabled(pluginDisabled);
}

#endif
