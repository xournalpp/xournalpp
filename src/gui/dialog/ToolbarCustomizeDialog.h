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
#include "../../util/XournalType.h"

class MainWindow;
class AbstractToolItem;

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData;

class ToolbarCustomizeDialog : public GladeGui {
public:
	ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win);
	virtual ~ToolbarCustomizeDialog();

public:
	virtual void show();

private:
	static void dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext,
			gint x, gint y, GtkSelectionData * data, guint info, guint time, ToolbarCustomizeDialog * dlg);
	static bool toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context,
			gint x, gint y, guint time, ToolbarCustomizeDialog * dlg);
	static void toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data);

	void removeFromToolbar(AbstractToolItem * item);
	void freeIconview();
	void rebuildIconview();

private:
	XOJ_TYPE_ATTRIB;
	AbstractToolItem * currentDragItem;

	MainWindow * win;
};

#endif /* __TOOLBARCUSTOMIZEDIALOG_H__ */
