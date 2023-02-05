/*
 * Xournal++
 *
 * Move-Page dialog
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

class MovePageDialog: public GladeGui {
public:
    MovePageDialog(GladeSearchpath* gladeSearchPath, int currentPage, int maxPage);
    ~MovePageDialog() override;

public:
    void show(GtkWindow* parent) override;

    // returns the selected page or -1 if closed
    int getSelectedPageFrom() const;
    int getSelectedPageTo() const;

private:
    int selectedPageFrom = -1;
    int selectedPageTo = -1;
};
