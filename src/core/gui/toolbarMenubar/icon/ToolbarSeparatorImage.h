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

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget

/**
 * Menuitem handler
 */
namespace ToolbarSeparatorImage {

/**
 * @brief Create Seperator Widget
 * This is used in the toolbar for spacing between items.
 *
 * @return GtkWidget* Separator
 */
GtkWidget* newImage();

/**
 * @brief Create Separator Pixbuf
 * This is used in the toolbar customization to drag the separator
 * from the Customization dialog to the toolbar and vice versa.
 *
 * @return GdkPixbuf* Seperator
 */
GdkPixbuf* getNewToolPixbuf();
};  // namespace ToolbarSeparatorImage
