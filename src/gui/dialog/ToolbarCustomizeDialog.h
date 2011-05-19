/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARCUSTOMIZEDIALOG_H__
#define __TOOLBARCUSTOMIZEDIALOG_H__

#include "../GladeGui.h"
#include <XournalType.h>

class MainWindow;
class AbstractToolItem;
class AbstractItemSelectionData;

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData;

class ToolbarCustomizeDialog: public GladeGui {
public:
	ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win);
	virtual ~ToolbarCustomizeDialog();

public:
	virtual void show();

private:
	static void dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext, gint x, gint y, GtkSelectionData * data, guint info, guint time,
			ToolbarCustomizeDialog * dlg);
	static bool toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y, guint time, ToolbarCustomizeDialog * dlg);
	static void toolbarDragLeafeCb(GtkToolbar * toolbar, GdkDragContext * context, guint time, ToolbarCustomizeDialog * dlg);
	static void toolbarDragDataReceivedCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y, GtkSelectionData * data, guint info, guint time);

	static void toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data);
	static void toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data);
	static void toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data, guint info,
			guint time, AbstractItemSelectionData * item);

	static GdkPixbuf * getImagePixbuf(GtkImage * image);

	void removeFromToolbar(AbstractToolItem * item);
	void freeIconview();
	void rebuildIconview();

private:
	XOJ_TYPE_ATTRIB;
	AbstractToolItem * currentDragItem;

	GList * itemDatalist;

	MainWindow * win;

	GList * itemSelectionData;
};

#endif /* __TOOLBARCUSTOMIZEDIALOG_H__ */
