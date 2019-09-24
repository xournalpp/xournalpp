#include "PluginController.h"
#include "Plugin.h"

#include "control/Control.h"
#include "gui/dialog/PluginDialog.h"
#include "gui/GladeSearchpath.h"

#include <StringUtils.h>

#include <config-features.h>

#include <algorithm>


PluginController::PluginController(Control* control)
 : control(control)
{
#ifdef ENABLE_PLUGINS
	string path = control->getGladeSearchPath()->getFirstSearchPath();
	if (StringUtils::endsWith(path, "ui"))
	{
		path = path.substr(0, path.length() - 2) + "plugins";
	}
	else
	{
		path += "/../plugins";
	}
	loadPluginsFrom(path);
#endif
}

PluginController::~PluginController()
{
#ifdef ENABLE_PLUGINS

	for (Plugin* p : this->plugins)
	{
		delete p;
	}

	this->plugins.clear();

#endif
}

/**
 * Load all plugins within this folder
 *
 * @param path The path which contains the plugin folders
 */
void PluginController::loadPluginsFrom(string path)
{
#ifdef ENABLE_PLUGINS

	GError* error = nullptr;
	GDir* dir = g_dir_open(path.c_str(), 0, &error);
	if (error != nullptr)
	{
		g_warning("Could not open plugin dir: «%s»", path.c_str());
		g_error_free(error);
		return;
	}

	Settings* settings = control->getSettings();
	vector<string> pluginEnabled = StringUtils::split(settings->getPluginEnabled(), ',');
	vector<string> pluginDisabled = StringUtils::split(settings->getPluginDisabled(), ',');

	const gchar* file;
	while ((file = g_dir_read_name(dir)) != nullptr)
	{
		string pluginFolder = path;
		pluginFolder += "/";
		pluginFolder += file;

		Plugin* p = new Plugin(control, file, pluginFolder);
		if (!p->isValid())
		{
			g_warning("Error loading plugin «%s»", file);
			delete p;
			continue;
		}

		if (p->isDefaultEnabled())
		{
			p->setEnabled(!(std::find(pluginDisabled.begin(), pluginDisabled.end(), p->getName()) != pluginDisabled.end()));
		}
		else
		{
			p->setEnabled(std::find(pluginEnabled.begin(), pluginEnabled.end(), p->getName()) != pluginEnabled.end());
		}

		p->loadScript();

		this->plugins.push_back(p);
	}
	g_dir_close(dir);
#endif
}

/**
 * Register toolbar item and all other UI stuff
 */
void PluginController::registerToolbar()
{
#ifdef ENABLE_PLUGINS
	for (Plugin* p : this->plugins)
	{
		p->registerToolbar();
	}
#endif
}

/**
 * Show Plugin manager Dialog
 */
void PluginController::showPluginManager()
{
	PluginDialog dlg(control->getGladeSearchPath(), control->getSettings());
	dlg.loadPluginList(this);
	dlg.show(control->getGtkWindow());
}

/**
 * Register menu stuff
 */
void PluginController::registerMenu()
{
#ifdef ENABLE_PLUGINS
	GtkWidget* menuPlugin = control->getWindow()->get("menuPlugin");
	for (Plugin* p : this->plugins)
	{
		p->registerMenu(control->getGtkWindow(), menuPlugin);
	}

	gtk_widget_show_all(menuPlugin);

#else
	// If plugins are disabled - disable menu also
	GtkWidget* menuitemPlugin = control->getWindow()->get("menuitemPlugin");
	gtk_widget_hide(menuitemPlugin);
#endif
}

/**
 * Return the plugin list
 */
vector<Plugin*>& PluginController::getPlugins()
{
	return plugins;
}
