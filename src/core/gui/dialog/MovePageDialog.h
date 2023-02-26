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
    MovePageDialog(GladeSearchpath* gladeSearchPath, size_t currentPage, size_t maxPage);
    ~MovePageDialog() override;

public:
    void show(GtkWindow* parent) override;

    // returns the selected page or -1 if closed
    size_t getSelectedPageFrom() const;
    size_t getSelectedPageTo() const;

private:
    size_t selectedPageFrom = 0;
    size_t selectedPageTo = 0;
};
