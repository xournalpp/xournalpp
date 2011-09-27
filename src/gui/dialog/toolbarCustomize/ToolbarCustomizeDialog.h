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

#include "../../GladeGui.h"
#include <XournalType.h>
#include <String.h>

class MainWindow;
class AbstractToolItem;
class AbstractItemSelectionData;

class ToolbarDragDropHandler;

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData;

class ToolbarCustomizeDialog: public GladeGui {
public:
	ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win, ToolbarDragDropHandler * handler);
	virtual ~ToolbarCustomizeDialog();

public:
	virtual void show();
	void rebuildIconview();

private:
	static void dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext, gint x, gint y,
			GtkSelectionData * data, guint info, guint time, ToolbarCustomizeDialog * dlg);
	static void toolbarDragLeafeCb(GtkToolbar * toolbar, GdkDragContext * context, guint time,
			ToolbarCustomizeDialog * dlg);
	static void toolbarDragDataReceivedCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y,
			GtkSelectionData * data, guint info, guint time, ToolbarCustomizeDialog * dlg);

	static void toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data);
	static void toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data);
	static void toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data,
			guint info, guint time, AbstractItemSelectionData * item);

	void removeFromToolbar(AbstractToolItem * item, String toolbarName, int pos);
	void freeIconview();

	void cleanupToolbarsItemsDrag();

private:
	static void windowResponseCb(GtkDialog * dialog, int response, ToolbarCustomizeDialog * dlg);

private:
	XOJ_TYPE_ATTRIB;
	ToolbarDragDropHandler * tbhandler;

	GList * itemDatalist;

	MainWindow * win;

	ToolbarDragDropHandler * handler;

	GList * itemSelectionData;
};

#endif /* __TOOLBARCUSTOMIZEDIALOG_H__ */
