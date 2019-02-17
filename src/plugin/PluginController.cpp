#include "PluginController.h"
#include "Plugin.h"

#include "control/Control.h"
#include "gui/GladeSearchpath.h"

#include <config-features.h>


PluginController::PluginController(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(PluginController);

#ifdef ENABLE_PLUGINS
	loadPluginsFrom(control->getGladeSearchPath()->getFirstSearchPath() + "/../plugins/");
#endif
}

PluginController::~PluginController()
{
	XOJ_CHECK_TYPE(PluginController);

	for (Plugin* p : this->plugins)
	{
		delete p;
	}

	this->plugins.clear();

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

		Plugin* p = new Plugin(pluginFolder);
		if (!p->isValid())
		{
			delete p;
			continue;
		}

		this->plugins.push_back(p);
	}
	g_dir_close(dir);
#endif
}

