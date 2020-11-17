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
    auto searchPath = control->getGladeSearchPath()->getFirstSearchPath();
    loadPluginsFrom((searchPath /= "../plugins").lexically_normal());
#endif
}

void PluginController::loadPluginsFrom(fs::path const& path) {
#ifdef ENABLE_PLUGINS
    Settings* settings = control->getSettings();
    std::vector<std::string> pluginEnabled = StringUtils::split(settings->getPluginEnabled(), ',');
    std::vector<std::string> pluginDisabled = StringUtils::split(settings->getPluginDisabled(), ',');

    try {
        for (auto const& f: fs::directory_iterator(path)) {
            const auto& pluginPath = f.path();
            auto plugin = std::make_unique<Plugin>(control, pluginPath.filename().string(), pluginPath);
            if (!plugin->isValid()) {
                g_warning("Error loading plugin \"%s\"", f.path().string().c_str());
                continue;
            }

            if (plugin->isDefaultEnabled()) {
                plugin->setEnabled(!(std::find(pluginDisabled.begin(), pluginDisabled.end(), plugin->getName()) !=
                                     pluginDisabled.end()));
            } else {
                plugin->setEnabled(std::find(pluginEnabled.begin(), pluginEnabled.end(), plugin->getName()) !=
                                   pluginEnabled.end());
            }

            plugin->loadScript();

            this->plugins.emplace_back(std::move(plugin));
        }
    } catch (fs::filesystem_error const& e) {
        g_warning("Could not open plugin dir: \"%s\"", path.string().c_str());
        return;
    }
#endif
}

void PluginController::registerToolbar() {
#ifdef ENABLE_PLUGINS
    for (auto&& p: this->plugins) {
        p->registerToolbar();
    }
#endif
}

void PluginController::showPluginManager() const {
    PluginDialog dlg(control->getGladeSearchPath(), control->getSettings());
    dlg.loadPluginList(this);
    dlg.show(control->getGtkWindow());
}

void PluginController::registerMenu() {
#ifdef ENABLE_PLUGINS
    GtkWidget* menuPlugin = control->getWindow()->get("menuPlugin");
    for (auto&& p: this->plugins) {
        p->registerMenu(control->getGtkWindow(), menuPlugin);
    }
    gtk_widget_show_all(menuPlugin);

#else
    // If plugins are disabled - disable menu also
    GtkWidget* menuitemPlugin = control->getWindow()->get("menuitemPlugin");
    gtk_widget_hide(menuitemPlugin);
#endif
}

auto PluginController::getPlugins() const -> std::vector<Plugin*> {
    std::vector<Plugin*> pl;
    pl.reserve(plugins.size());
    std::transform(begin(plugins), end(plugins), std::back_inserter(pl), [](auto&& plugin) { return plugin.get(); });
    return pl;
}
