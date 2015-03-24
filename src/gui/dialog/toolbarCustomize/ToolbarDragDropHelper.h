/*
 * Xournal++
 *
 * Toolbar drag and drop helpers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __TOOLBARDRAGDROPHELPER_H__
#define __TOOLBARDRAGDROPHELPER_H__

#include <gtk/gtk.h>

class ToolbarDragDropHelper
{
private:
	ToolbarDragDropHelper();
	virtual ~ToolbarDragDropHelper();

public:
	static void dragDestAddToolbar(GtkWidget* target);
	static void dragSourceAddToolbar(GtkWidget* widget);

	static GdkPixbuf* getImagePixbuf(GtkImage* image);
	static GdkPixbuf* getColorImage(int color);

public:
	static GdkAtom atomToolItem;
	static GtkTargetEntry dropTargetEntry;
};

#endif /* __TOOLBARDRAGDROPHELPER_H__ */
