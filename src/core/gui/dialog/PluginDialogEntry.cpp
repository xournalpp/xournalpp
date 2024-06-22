#include "PluginDialogEntry.h"

#ifdef ENABLE_PLUGINS

#include "gui/Builder.h"
#include "plugin/Plugin.h"
#include "util/i18n.h"

#include "filesystem.h"  // for path

class GladeSearchpath;

constexpr auto UI_FILE = "pluginEntry.glade";
constexpr auto UI_WIDGET_NAME = "pluginMainBox";

PluginDialogEntry::PluginDialogEntry(Plugin* plugin, GladeSearchpath* gladeSearchPath, GtkBox* box): plugin(plugin) {
    Builder builder(gladeSearchPath, UI_FILE);
    gtk_box_append(box, builder.get(UI_WIDGET_NAME));
    stateButton = GTK_CHECK_BUTTON(builder.get("cbEnabled"));

    gtk_label_set_text(GTK_LABEL(builder.get("pluginName")), plugin->getName().c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbAuthor")), plugin->getAuthor().c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbVersion")), plugin->getVersion().c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbDescription")), plugin->getDescription().c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbPath")), plugin->getPath().u8string().c_str());
    gtk_label_set_text(GTK_LABEL(builder.get("lbDefaultText")),
                       plugin->isDefaultEnabled() ? _("default enabled") : _("default disabled"));
    gtk_check_button_set_active(stateButton, plugin->isEnabled());
}

void PluginDialogEntry::saveSettings(std::string& pluginEnabled, std::string& pluginDisabled) {
    bool state = gtk_check_button_get_active(stateButton);

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
}

#endif
