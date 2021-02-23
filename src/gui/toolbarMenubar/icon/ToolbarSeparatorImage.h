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
namespace ToolbarSeparatorImage {
/**
 * Returns: (transfer full)
 */
GdkPixbuf* newPixbuf();

/**
 * Returns: (transfer floating)
 */
GtkWidget* newImage();
}  // namespace ToolbarSeparatorImage
