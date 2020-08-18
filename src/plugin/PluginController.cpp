#include "PluginController.h"

#include <algorithm>

#include <config-features.h>

#include "control/Control.h"
#include "gui/GladeSearchpath.h"
#include "gui/dialog/PluginDialog.h"

#include "Plugin.h"
#include "StringUtils.h"


PluginController::PluginController(Control* control): control(control) {
#ifdef ENABLE_PLUGINS
    // Use string instead of fs::path due to string operations below
    string path = control->getGladeSearchPath()->getFirstSearchPath().string();
    if (StringUtils::endsWith(path, "ui")) {
        path = path.substr(0, path.length() - 2) + "plugins";
    } else {
        path += "/../plugins";
    }
    loadPluginsFrom(path);
#endif
}

PluginController::~PluginController()
#ifdef ENABLE_PLUGINS
{


    for (Plugin* p: this->plugins) {
        delete p;
    }

    this->plugins.clear();
}
#else
        = default;
#endif


/**
 * Load all plugins within this folder
 *
 * @param path The path which contains the plugin folders
 */
void PluginController::loadPluginsFrom(fs::path const& path) {
#ifdef ENABLE_PLUGINS
    Settings* settings = control->getSettings();
    vector<string> pluginEnabled = StringUtils::split(settings->getPluginEnabled(), ',');
    vector<string> pluginDisabled = StringUtils::split(settings->getPluginDisabled(), ',');

    try {
        for (auto const& f: fs::directory_iterator(path)) {
            auto pluginFolder = path / f.path();

            Plugin* p = new Plugin(control, f.path().string(), pluginFolder.string());
            if (!p->isValid()) {
                g_warning("Error loading plugin «%s»", f.path().string().c_str());
                delete p;
                continue;
            }

            if (p->isDefaultEnabled()) {
                p->setEnabled(!(std::find(pluginDisabled.begin(), pluginDisabled.end(), p->getName()) !=
                                pluginDisabled.end()));
            } else {
                p->setEnabled(std::find(pluginEnabled.begin(), pluginEnabled.end(), p->getName()) !=
                              pluginEnabled.end());
            }

            p->loadScript();

            this->plugins.push_back(p);
        }
    } catch (fs::filesystem_error const& e) {
        g_warning("Could not open plugin dir: «%s»", path.string().c_str());
        return;
    }
#endif
}

/**
 * Register toolbar item and all other UI stuff
 */
void PluginController::registerToolbar() {
#ifdef ENABLE_PLUGINS
    for (Plugin* p: this->plugins) {
        p->registerToolbar();
    }
#endif
}

/**
 * Show Plugin manager Dialog
 */
void PluginController::showPluginManager() {
    PluginDialog dlg(control->getGladeSearchPath(), control->getSettings());
    dlg.loadPluginList(this);
    dlg.show(control->getGtkWindow());
}

/**
 * Register menu stuff
 */
void PluginController::registerMenu() {
#ifdef ENABLE_PLUGINS
    GtkWidget* menuPlugin = control->getWindow()->get("menuPlugin");
    for (Plugin* p: this->plugins) {
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
auto PluginController::getPlugins() -> vector<Plugin*>& { return plugins; }
