/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#include "xournalpp/Xournalpp.h"

#include <gtk/gtk.h>
#include <lager/lens.hpp>

#include "gui/MainWindow.h"
#include "util/gtk_event_loop.h"


auto update(AppState model, Action action) -> XournalppResult {
    // To add more update functions see https://sinusoid.es/lager/modularity.html
    // Consider doing this on the lowest layer of hierarchy possible
    return std::visit(lager::visitor{[&](const ViewportAction& viewportAction) -> XournalppResult {
                          auto [newViewport, effect] = viewportUpdate(model.viewport, viewportAction);
                          model.viewport = newViewport;
                          return {model, effect};
                      }},
                      action);
}

struct ApplicationData {
    std::optional<MainWindow> widgetTree;
};

static auto activateCb(GtkApplication* app, ApplicationData* user_data) -> void {
    auto state = lager::make_store<Action>(AppState{}, update, with_gtk_event_loop{});
    /*
     * TODO initialize Widget tree and pass reader and context to all child widgets
     * if a child widget only needs part of the state, use following:
     * lager::reader<Viewport> viewportReader = reader[&AppState::viewport];
     * context should also only use the most restricted action type possible
     * viewportReader->x allows access to members of Viewport
     */

    user_data->widgetTree = MainWindow{std::move(state)};
    user_data->widgetTree->show();
}

auto run(int argc, char* argv[]) -> int {
    auto gtkapplication = gtk_application_new("org.xournalpp.xournalpp", G_APPLICATION_FLAGS_NONE);
    auto appData = ApplicationData{};
    g_signal_connect(gtkapplication, "activate", G_CALLBACK(activateCb), &appData);
    auto status = g_application_run(G_APPLICATION(gtkapplication), argc, argv);
    g_object_unref(gtkapplication);

    return status;
}
