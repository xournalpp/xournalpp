/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <cstdint>

#include <config-test.h>
#include <gio/gio.h>      // for GApplication, G_APPLICATION
#include <glib-object.h>  // for G_CALLBACK, g_signal_con...
#include <glib.h>
#include <gtest/gtest.h>

#include "control/Control.h"
#include "control/jobs/XournalScheduler.h"  // for XournalScheduler
#include "gui/GladeSearchpath.h"            // for GladeSearchpath

extern "C" {
#include <lauxlib.h>  // for luaL_Reg, luaL_newstate, luaL_requiref
#include <lua.h>      // for lua_getglobal, lua_getfield, lua_setf...
#include <lualib.h>   // for luaL_openlibs
}

#include "plugin/luapi_application.h"  // for luaopen_app


struct Data {
    Data() = default;
    Data(Data&&) = delete;
    Data(Data const&) = delete;
    auto operator=(Data&&) -> Data = delete;
    auto operator=(Data const&) -> Data = delete;

    ~Data() {}

    std::unique_ptr<GladeSearchpath> gladePath;
    std::unique_ptr<Control> control;
    std::unique_ptr<MainWindow> win;
};

void on_activate(GtkApplication* app, Data* data) {
    auto uiPath = fs::path(PROJECT_SOURCE_DIR) / "ui";
    data->gladePath = std::make_unique<GladeSearchpath>();
    data->gladePath->addSearchDirectory(uiPath);

    data->control = std::make_unique<Control>(G_APPLICATION(app), data->gladePath.get(), true);
    data->win = std::make_unique<MainWindow>(data->gladePath.get(), data->control.get(), app);
    data->control->initWindow(data->win.get());
    data->control->getScheduler()->start();
    Util::execInUiThread([=]() { data->control->getWindow()->getXournal()->layoutPages(); });
    gtk_application_add_window(app, GTK_WINDOW(data->win->getWindow()));

    data->win->show(nullptr);
    data->control->addDefaultPage("");

    auto pluginPath = fs::path(PROJECT_SOURCE_DIR) / "test" / "unit_tests" / "lua";
    auto plugin = std::make_unique<Plugin>(data->control.get(), pluginPath.filename().string(), pluginPath);
    ASSERT_TRUE(plugin->isValid());
    plugin->loadScript();

    g_application_quit(G_APPLICATION(app));
}

void on_shutdown(GApplication*, Data* data) { data->control->getScheduler()->stop(); }

TEST(LuaTest, testPage) {
    Data data;
    GtkApplication* app = gtk_application_new("com.github.xournalpp.xournalpp", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(&on_activate), &data);
    g_signal_connect(app, "shutdown", G_CALLBACK(&on_shutdown), &data);
    g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
}
