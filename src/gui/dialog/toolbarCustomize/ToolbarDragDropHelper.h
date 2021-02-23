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

GdkPixbuf* getImagePixbuf(GtkImage* image);

extern const GdkAtom atomToolItem;
extern const GtkTargetEntry dropTargetEntry;
};  // namespace ToolbarDragDropHelper
