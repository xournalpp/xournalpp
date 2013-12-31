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

#include "CustomizeableColorList.h"

class MainWindow;
class AbstractToolItem;
class ToolbarDragDropHandler;

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData;

class ToolbarCustomizeDialog: public GladeGui
{
public:
	ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
	                       ToolbarDragDropHandler* handler);
	virtual ~ToolbarCustomizeDialog();

public:
	virtual void show(GtkWindow* parent);

	void rebuildIconview();
	void rebuildColorIcons();

private:
	static void dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext,
	                             gint x, gint y,
	                             GtkSelectionData* data, guint info, guint time, ToolbarCustomizeDialog* dlg);
	static void toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context,
	                               guint time,
	                               ToolbarCustomizeDialog* dlg);
	static void toolbarDragDataReceivedCb(GtkToolbar* toolbar,
	                                      GdkDragContext* context, gint x, gint y,
	                                      GtkSelectionData* data, guint info, guint time, ToolbarCustomizeDialog* dlg);

	static void toolitemDragBegin(GtkWidget* widget, GdkDragContext* context,
	                              ToolItemDragData* data);
	static void toolitemDragEnd(GtkWidget* widget, GdkDragContext* context,
	                            ToolItemDragData* data);
	static void toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context,
	                                GtkSelectionData* selection_data,
	                                guint info, guint time, ToolItemDragData* data);

	static void toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context,
	                                   void* data);
	static void toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context,
	                                 ToolbarCustomizeDialog* dlg);
	static void toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context,
	                                     GtkSelectionData* selection_data, guint info, guint time, void* data);

	static void toolitemDragBeginSeparator(GtkWidget* widget,
	                                       GdkDragContext* context, void* unused);
	static void toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context,
	                                     void* unused);
	static void toolitemDragDataGetSeparator(GtkWidget* widget,
	                                         GdkDragContext* context,
	                                         GtkSelectionData* selection_data, guint info, guint time, void* unused);

	void freeIconview();
	void freeColorIconview();

private:
	static void windowResponseCb(GtkDialog* dialog, int response,
	                             ToolbarCustomizeDialog* dlg);

private:
	XOJ_TYPE_ATTRIB;

	CustomizeableColorList* colorList;

	GList* itemDatalist;

	MainWindow* win;

	ToolbarDragDropHandler* handler;
};

#endif /* __TOOLBARCUSTOMIZEDIALOG_H__ */
