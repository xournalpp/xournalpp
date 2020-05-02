/*
 * Xournal++
 *
 * The Main window
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include <string>

#include <Xournalpp.h>
#include <lager/context.hpp>
#include <lager/reader.hpp>
class MainWindow {
public:
    MainWindow(lager::reader<AppState> state, lager::context<Action> context);

private:
    lager::reader<AppState> state;
    lager::context<Action> context;
};
