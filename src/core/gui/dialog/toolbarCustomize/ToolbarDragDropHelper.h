/*
 * Xournal++
 *
 * Toolbar drag and drop helpers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

namespace ToolbarDragDropHelper {
void dragDestAddToolbar(GtkWidget* target);
void dragSourceAddToolbar(GtkWidget* widget);

// auto gdk_context_set_icon_from_image(GdkDragContext* ctx, GtkWidget* image) -> bool;

extern const GValue atomToolItem;
// TODO (gtk4): Find replacement
// extern const GtkTargetEntry dropTargetEntry;
};  // namespace ToolbarDragDropHelper
