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

#include "Xournalpp.h"

#include <lager/event_loop/manual.hpp>
#include <lager/store.hpp>

using XournalppResult = std::pair<AppState, lager::effect<Action>>;

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

auto main(int argc, char* argv[]) -> int {
    auto store = lager::make_store<Action>(AppState{}, update, lager::with_manual_event_loop{});
    auto reader = static_cast<lager::reader<AppState>>(store);
    /*
     * TODO initialize Widget tree and pass reader as state to all child widgets
     * if a child widget only needs part of the state, use following:
     * lager::reader<Viewport> viewportReader = reader[&AppState::viewport];
     * viewportReader->x allows access to members
     */

    return 0;
}