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

#include "util/raii/GObjectSPtr.h"

enum SeparatorType : bool { SEPARATOR, SPACER };

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
GtkWidget* newImage(SeparatorType separator);

/**
 * @brief Create Separator Pixbuf
 * This is used in the toolbar customization to drag the separator
 * from the Customization dialog to the toolbar and vice versa.
 *
 * @return Separator icon
 */
xoj::util::GObjectSPtr<GdkPaintable> newGdkPaintable(SeparatorType separator);
};  // namespace ToolbarSeparatorImage
