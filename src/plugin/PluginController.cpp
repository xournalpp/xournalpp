#include "PluginController.h"
#include "Plugin.h"

#include "control/Control.h"
#include "gui/GladeSearchpath.h"

#include <StringUtils.h>

#include <config-features.h>


PluginController::PluginController(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(PluginController);
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
	XOJ_CHECK_TYPE(PluginController);
#ifdef ENABLE_PLUGINS

	for (Plugin* p : this->plugins)
	{
		delete p;
	}

	this->plugins.clear();

#endif
	XOJ_RELEASE_TYPE(PluginController);
}

/**
 * Load all plugins within this folder
 *
 * @param path The path which contains the plugin folders
 */
void PluginController::loadPluginsFrom(string path)
{
	XOJ_CHECK_TYPE(PluginController);
#ifdef ENABLE_PLUGINS

	GError* error = NULL;
	GDir* dir = g_dir_open(path.c_str(), 0, &error);
	if (error != NULL)
	{
		g_warning("Could not open plugin dir: «%s»", path.c_str());
		g_error_free(error);
		return;
	}

	const gchar* file;
	while ((file = g_dir_read_name(dir)) != NULL)
	{
		string pluginFolder = path;
		pluginFolder += "/";
		pluginFolder += file;

		Plugin* p = new Plugin(file, pluginFolder);
		if (!p->isValid())
		{
			g_warning("Error loading plugin «%s»", file);
			delete p;
			continue;
		}

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
	XOJ_CHECK_TYPE(PluginController);

#ifdef ENABLE_PLUGINS
	for (Plugin* p : this->plugins)
	{
		p->registerToolbar();
	}
#endif
}

/**
 * Register menu stuff
 */
void PluginController::registerMenu()
{
	XOJ_CHECK_TYPE(PluginController);

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


