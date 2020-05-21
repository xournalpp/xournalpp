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

#include <gtk-4.0/gtk/gtk.h>
#include <lager/context.hpp>
#include <lager/reader.hpp>

#include "xournalpp/Xournalpp.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app, XournalppStore store);

    auto show() -> void;

private:
    // State
    XournalppStore store;

    // Gtk components
    GtkWindow* window = nullptr;
};
