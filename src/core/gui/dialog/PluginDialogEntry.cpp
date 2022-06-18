#include "PluginDialogEntry.h"

#include "plugin/Plugin.h"  // for Plugin
#include "util/i18n.h"      // for _

#include "config-features.h"  // for ENABLE_PLUGINS
#include "filesystem.h"       // for path

class GladeSearchpath;


PluginDialogEntry::PluginDialogEntry(Plugin* plugin, GladeSearchpath* gladeSearchPath, GtkWidget* w):
        GladeGui(gladeSearchPath, "pluginEntry.glade", "offscreenwindow"), plugin(plugin) {
    GtkWidget* pluginMainBox = get("pluginMainBox");
    gtk_container_remove(GTK_CONTAINER(getWindow()), pluginMainBox);
    gtk_container_add(GTK_CONTAINER(w), pluginMainBox);
    gtk_widget_show_all(pluginMainBox);

    loadSettings();
}

void PluginDialogEntry::loadSettings() {
#ifdef ENABLE_PLUGINS
    gtk_label_set_text(GTK_LABEL(get("pluginName")), plugin->getName().c_str());
    gtk_label_set_text(GTK_LABEL(get("lbAuthor")), plugin->getAuthor().c_str());
    gtk_label_set_text(GTK_LABEL(get("lbVersion")), plugin->getVersion().c_str());
    gtk_label_set_text(GTK_LABEL(get("lbDescription")), plugin->getDescription().c_str());
    gtk_label_set_text(GTK_LABEL(get("lbPath")), plugin->getPath().u8string().c_str());
    gtk_label_set_text(GTK_LABEL(get("lbDefaultText")),
                       plugin->isDefaultEnabled() ? _("default enabled") : _("default disabled"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbEnabled")), plugin->isEnabled());
#endif
}

void PluginDialogEntry::show(GtkWindow* parent) {
    // Not implemented! This is not a dialog!
}

void PluginDialogEntry::saveSettings(std::string& pluginEnabled, std::string& pluginDisabled) {
#ifdef ENABLE_PLUGINS
    bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbEnabled")));

    if (state == plugin->isDefaultEnabled()) {
        return;
    }

    if (state) {
        if (!pluginEnabled.empty()) {
            pluginEnabled += ",";
        }
        pluginEnabled += plugin->getPath().u8string();
    } else {
        if (!pluginDisabled.empty()) {
            pluginDisabled += ",";
        }
        pluginDisabled += plugin->getPath().u8string();
    }
#endif
}
