/*
 * Xournal++
 *
 * Goto-Page dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

class GladeSearchpath;

class GotoDialog: public GladeGui {
public:
    GotoDialog(GladeSearchpath* gladeSearchPath, int maxPage);
    ~GotoDialog() override;

public:
    void show(GtkWindow* parent) override;

    // returns the selected page or -1 if closed
    int getSelectedPage() const;

private:
    int selectedPage = -1;
};
