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

#include <gtkmm.h>
#include <lager/lens.hpp>
#include <xournalppui.c>

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

auto run(int argc, char* argv[]) -> int {
    auto resource = ui_get_resource();
    g_resources_register(resource);
    auto gtkapplication = Gtk::Application::create(argc, argv, "org.xournalpp.xournalpp");
    auto state = lager::make_store<Action>(AppState{Viewport{0, 0, 0.0, 0.0, 0.0}, Settings{Settings::INFINITE}},
                                           update, with_gtk_event_loop{});
    MainWindow mainWindow{std::move(state)};
    return gtkapplication->run(*mainWindow.getGtkWindow());
}
