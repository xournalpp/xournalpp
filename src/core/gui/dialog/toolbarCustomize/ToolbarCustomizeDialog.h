/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "gui/toolbarMenubar/model/ColorPalette.h"
#include "util/raii/GtkWindowUPtr.h"

class MainWindow;
class ToolbarDragDropHandler;
class GladeSearchpath;

class ToolbarCustomizeDialog {
public:
    ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win, ToolbarDragDropHandler* handler);
    ~ToolbarCustomizeDialog();

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    ToolbarDragDropHandler* handler;
    xoj::util::GtkWindowUPtr window;
    GtkNotebook* notebook;
    Palette palette;
};
