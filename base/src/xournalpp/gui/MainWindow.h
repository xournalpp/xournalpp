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

#include <gtk/gtk.h>
#include <lager/context.hpp>
#include <lager/reader.hpp>
#include <xournalpp/gui/widgets/XournalWidget.h>

#include "xournalpp/Xournalpp.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app, XournalppStore store);

    auto show() -> void;

private:
    // State
    XournalppStore store;

    // View components
    XournalWidget xournal;


    // Gtk components
    GtkWindow* window = nullptr;
};
