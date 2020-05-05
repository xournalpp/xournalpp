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

#include <gtkmm.h>
#include <lager/context.hpp>
#include <lager/reader.hpp>

#include "xournalpp/Xournalpp.h"

class MainWindow: public Gtk::Window {
public:
    MainWindow(XournalppStore store);
    ~MainWindow();

    auto getGtkWindow() -> Gtk::Window*;

private:
    // State
    XournalppStore store;

    // Gtk stuff
    Gtk::Window* window = nullptr;
};
