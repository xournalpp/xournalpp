/*
 * Xournal++
 *
 * Toolbar drag and drop helpers
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARDRAGDROPHELPER_H__
#define __TOOLBARDRAGDROPHELPER_H__

#include <gtk/gtk.h>

class ToolbarDragDropHelper {
private:
	ToolbarDragDropHelper();
	virtual ~ToolbarDragDropHelper();

public:
	static void dragDestAddToolbar(GtkWidget * target);
	static void dragSourceAddToolbar(GtkWidget * widget);

public:
	static GdkAtom atomToolItem;
};

#endif /* __TOOLBARDRAGDROPHELPER_H__ */
