#include "PluginDialog.h"

#include <string>  // for string

#include "control/settings/Settings.h"     // for Settings
#include "gui/dialog/PluginDialogEntry.h"  // for PluginDialogEntry
#include "plugin/PluginController.h"       // for PluginController

class GladeSearchpath;
class Plugin;

PluginDialog::PluginDialog(GladeSearchpath* gladeSearchPath, Settings* settings):
        GladeGui(gladeSearchPath, "plugin.glade", "pluginDialog"), settings(settings) {}

void PluginDialog::loadPluginList(PluginController const* pc) {
    GtkWidget* pluginBox = get("pluginBox");

    for (Plugin* p: pc->getPlugins()) {
        this->plugins.emplace_back(std::make_unique<PluginDialogEntry>(p, getGladeSearchPath(), pluginBox));
    }
}

void PluginDialog::saveSettings() {
    std::string pluginEnabled;
    std::string pluginDisabled;

    // Save plugin settings
    for (auto&& bcg: this->plugins) { bcg->saveSettings(pluginEnabled, pluginDisabled); }

    settings->setPluginEnabled(pluginEnabled);
    settings->setPluginDisabled(pluginDisabled);
}

void PluginDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    int returnCode = gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);

    if (returnCode == 2) {
        saveSettings();
    }
}
