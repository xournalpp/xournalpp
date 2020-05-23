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

    static auto dragBegin(GtkGestureDrag* gesture, double start_x, double start_y, gpointer user_data) -> void;
    static auto dragEnd(GtkGestureDrag* gesture, double offset_x, double offset_y, gpointer user_data) -> void;
    static auto dragUpdate(GtkGestureDrag* gesture, double offset_x, double offset_y, gpointer user_data) -> void;

private:
    // State
    XournalppStore store;

    // Gtk components
    GtkWindow* window = nullptr;
};
