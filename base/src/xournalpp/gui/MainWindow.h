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

#include "xournalpp/Xournalpp.h"

class MainWindow {
public:
    MainWindow(XournalppStore store);

    auto show() -> void;

private:
    XournalppStore store;

    GtkWindow* window = nullptr;
};
