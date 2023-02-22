#include "PluginController.h"

#include <cassert>
#include <memory>

#include "control/Control.h"
#include "gui/MainWindow.h"

#include "config-features.h"

#ifdef ENABLE_PLUGINS
#include <algorithm>
#include <tuple>
#include <utility>

#include "control/settings/Settings.h"
#include "gui/GladeSearchpath.h"
#include "gui/dialog/PluginDialog.h"
#include "util/PathUtil.h"
#include "util/StringUtils.h"

#include "Plugin.h"

namespace {

/**
 * @brief This Comparator defines the ordering of Plugins in the Plugin-vector, so that all Plugins are sorted first by
 * their name, then by their path. Since the name is determined by the parent directory of the Plugin's path, this
 * also ensures that only Plugins with unique paths are kept in the vector.
 */
struct PluginComparator {
    using KeyType = std::tuple<std::string const&, fs::path const&>;
    static auto get_key(Plugin const& plugin) -> KeyType { return {plugin.getName(), plugin.getPath()}; }

    [[maybe_unused]] auto operator()(std::unique_ptr<Plugin> const& lhs, std::unique_ptr<Plugin> const& rhs) const;
    [[maybe_unused]] auto operator()(std::unique_ptr<Plugin> const& lhs, KeyType const& rhs) const;
    [[maybe_unused]] auto operator()(KeyType const& lhs, std::unique_ptr<Plugin> const& rhs) const;

    [[maybe_unused]] auto operator()(Plugin const& lhs, Plugin const& rhs) const { return get_key(lhs) < get_key(rhs); }
    [[maybe_unused]] auto operator()(Plugin const& lhs, KeyType const& rhs) const { return get_key(lhs) < rhs; }
    [[maybe_unused]] auto operator()(KeyType const& lhs, Plugin const& rhs) const { return lhs < get_key(rhs); }
};

auto PluginComparator::operator()(std::unique_ptr<Plugin> const& lhs, std::unique_ptr<Plugin> const& rhs) const {
    return PluginComparator{}(*lhs, *rhs);
}

auto PluginComparator::operator()(std::unique_ptr<Plugin> const& lhs, KeyType const& rhs) const {
    return PluginComparator{}(*lhs, rhs);
}

auto PluginComparator::operator()(KeyType const& lhs, std::unique_ptr<Plugin> const& rhs) const {
    return PluginComparator{}(lhs, *rhs);
}

auto load_available_plugins_from(fs::path const& path, Control* control) -> std::vector<std::unique_ptr<Plugin>> {
    if (!fs::is_directory(path)) {
        g_info("Skipping Plugin path, it is not a directory: %s", path.string().c_str());
        return {};
    }
    g_info("Loading plugins from: %s", path.string().c_str());

    std::vector<std::unique_ptr<Plugin>> returner;
    try {
        for (auto const& f: fs::directory_iterator(path)) {
            const auto& pluginPath = f.path();
            try {
                auto plugin = std::make_unique<Plugin>(control, pluginPath.filename().string(), pluginPath);
                if (!plugin || !plugin->isValid()) {
                    g_warning("Error loading plugin \"%s\"", f.path().string().c_str());
                    continue;
                }
                plugin->setEnabled(plugin->isDefaultEnabled());
                returner.emplace_back(std::move(plugin));
            } catch (const std::exception& e) {
                g_warning("Error loading plugin \"%s\": %s", f.path().string().c_str(), e.what());
            }
        }
    } catch (const fs::filesystem_error& e) {
        g_warning("Could not open plugin dir: \"%s\": %s", path.string().c_str(), e.what());
    }
    return returner;
}

auto emplace_sorted_if_not_exists(std::vector<std::unique_ptr<Plugin>>& plugins, std::unique_ptr<Plugin> plugin)
        -> void {
    assert(std::is_sorted(begin(plugins), end(plugins), PluginComparator{}));
    auto iter = std::equal_range(begin(plugins), std::end(plugins), plugin, PluginComparator{});
    if (iter.first != iter.second) {
        return;
    }
    plugins.emplace(iter.first, std::move(plugin));
    assert(std::is_sorted(begin(plugins), end(plugins), PluginComparator{}));
}
}  // namespace
#endif


PluginController::PluginController(Control* control): control(control) {
#ifdef ENABLE_PLUGINS
    // Todo(fabian) move those search paths into PathUtils
    auto searchPath = control->getGladeSearchPath()->getFirstSearchPath();
    auto searchPaths = {fs::weakly_canonical(searchPath /= "../plugins"),  //
                        Util::getConfigSubfolder("plugins")};

    for (auto&& path: searchPaths) {
        auto loaded_plugins = load_available_plugins_from(path, control);
        this->plugins.reserve(loaded_plugins.size() + this->plugins.size());
        for (auto&& plugin: loaded_plugins) {
            emplace_sorted_if_not_exists(this->plugins, std::move(plugin));
        }
    }

    Settings* settings = control->getSettings();
    // enable plugins
    std::vector<std::string> pluginEnabled = StringUtils::split(settings->getPluginEnabled(), ',');
    for (auto&& plugin_path: pluginEnabled) {
        auto path = fs::path(plugin_path);
        auto name = path.filename().string();
        auto iter = std::equal_range(begin(plugins), end(plugins), std::tie(name, path), PluginComparator{});
        if (iter.first != iter.second) {
            iter.first->get()->setEnabled(true);
        }
    }
    // disable plugins
    std::vector<std::string> pluginDisabled = StringUtils::split(settings->getPluginDisabled(), ',');
    for (auto&& plugin_path: pluginDisabled) {
        auto path = fs::path(plugin_path);
        auto name = path.filename().string();
        auto iter = std::equal_range(begin(plugins), end(plugins), std::tie(name, path), PluginComparator{});
        if (iter.first != iter.second) {
            iter.first->get()->setEnabled(false);
        }
    }

    for (auto&& plugin: plugins) {
        if (plugin->isEnabled()) {
            plugin->loadScript();
        }
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
#ifdef ENABLE_PLUGINS
    PluginDialog dlg(control->getGladeSearchPath(), control->getSettings());
    dlg.loadPluginList(this);
    dlg.show(control->getGtkWindow());
#endif
}

auto PluginController::createMenuSections(GtkApplicationWindow* win) -> std::vector<GMenuModel*> {
#ifdef ENABLE_PLUGINS
    size_t id = 0;
    std::vector<GMenuModel*> sections;
    for (auto&& p: this->plugins) {
        id = p->populateMenuSection(win, id);
        auto* section = p->getMenuSection();
        if (section) {
            sections.emplace_back(section);
        }
    }
    return sections;
#else
    return {};
#endif
}

void PluginController::registerToolButtons(ToolMenuHandler* toolMenuHandler) {
#ifdef ENABLE_PLUGINS
    for (auto&& p: this->plugins) {
        p->registerToolButton(toolMenuHandler);
    }
#endif
}

auto PluginController::getPlugins() const -> std::vector<Plugin*> {
#ifdef ENABLE_PLUGINS
    std::vector<Plugin*> pl;
    pl.reserve(plugins.size());
    std::transform(begin(plugins), end(plugins), std::back_inserter(pl), [](auto&& plugin) { return plugin.get(); });
    return pl;
#else
    return {};
#endif
}
