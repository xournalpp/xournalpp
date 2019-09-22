#include "PluginDialogEntry.h"

#include "plugin/Plugin.h"

#include <i18n.h>


PluginDialogEntry::PluginDialogEntry(Plugin* plugin, GladeSearchpath* gladeSearchPath, GtkWidget* w)
 : GladeGui(gladeSearchPath, "pluginEntry.glade", "offscreenwindow"),
   plugin(plugin)
{
	GtkWidget* pluginMainBox = get("pluginMainBox");
	gtk_container_remove(GTK_CONTAINER(getWindow()), pluginMainBox);
	gtk_container_add(GTK_CONTAINER(w), pluginMainBox);
	gtk_widget_show_all(pluginMainBox);

	loadSettings();
}

PluginDialogEntry::~PluginDialogEntry()
{
}

void PluginDialogEntry::loadSettings()
{
#ifdef ENABLE_PLUGINS
	gtk_label_set_text(GTK_LABEL(get("pluginName")), plugin->getName().c_str());
	gtk_label_set_text(GTK_LABEL(get("lbAuthor")), plugin->getAuthor().c_str());
	gtk_label_set_text(GTK_LABEL(get("lbVersion")), plugin->getVersion().c_str());
	gtk_label_set_text(GTK_LABEL(get("lbDescription")), plugin->getDescription().c_str());

	gtk_label_set_text(GTK_LABEL(get("lbDefaultText")), plugin->isDefaultEnabled() ? _("default enabled") : _("default disabled"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get("cbEnabled")), plugin->isEnabled());
#endif
}

void PluginDialogEntry::show(GtkWindow* parent)
{
	// Not implemented! This is not a dialog!
}

void PluginDialogEntry::saveSettings(string& pluginEnabled, string& pluginDisabled)
{
#ifdef ENABLE_PLUGINS
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get("cbEnabled")));

	if (state == plugin->isDefaultEnabled())
	{
		return;
	}

	if (state)
	{
		if (!pluginEnabled.empty())
		{
			pluginEnabled += ",";
		}
		pluginEnabled += plugin->getName();
	}
	else
	{
		if (!pluginDisabled.empty())
		{
			pluginDisabled += ",";
		}
		pluginDisabled += plugin->getName();
	}
#endif
}
