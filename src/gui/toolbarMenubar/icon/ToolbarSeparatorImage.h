/*
 * Xournal++
 *
 * Toolbar icon for separator (only used for drag and drop and so)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

/**
 * Menuitem handler
 */
class ToolbarSeparatorImage {
private:
    ToolbarSeparatorImage();
    virtual ~ToolbarSeparatorImage();

public:
    static GtkWidget* newSepeartorImage();
};
