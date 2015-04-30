#include "ToolbarCustomizeDialog.h"

#include "CustomizeableColorList.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/widgets/SelectColor.h"
#include "ToolbarDragDropHandler.h"
#include "ToolbarDragDropHelper.h"
#include "ToolItemDragCurrentData.h"

#include <config.h>
#include <Util.h>

#include <glib/gi18n-lib.h>

typedef struct _ToolItemDragData ToolItemDragData;

struct _ToolItemDragData
{
	ToolbarCustomizeDialog* dlg;
	GdkPixbuf* icon;
	AbstractToolItem* item;
	GtkWidget* ebox;
};

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
											   ToolbarDragDropHandler* handler) :
GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar")
{
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

	this->win = win;
	this->handler = handler;
	this->itemDatalist = NULL;
	this->colorList = new CustomizeableColorList();

	rebuildIconview();
	rebuildColorIcons();

	GtkWidget* target = get("viewport1");

	// prepare drag & drop
	gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
	ToolbarDragDropHelper::dragDestAddToolbar(target);

	g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);

	// init separator
	GtkWidget* tbSeparator = get("tbSeparator");

	GtkWidget* icon = Util::newSepeartorImage();
	GtkWidget* box = gtk_vbox_new(false, 3);
	gtk_widget_show(box);

	GtkWidget* label = gtk_label_new(_("Separator"));
	gtk_widget_show(label);
	gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

	GtkWidget* ebox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(ebox), box);
	gtk_widget_show(ebox);

	gtk_widget_show(icon);

	gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

	// make ebox a drag source
	gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
	ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

	g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBeginSeparator), NULL);
	g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEndSeparator), NULL);

	g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGetSeparator), NULL);

	gtk_table_attach(GTK_TABLE(tbSeparator), ebox, 0, 1, 0, 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog()
{
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	freeIconview();
	freeColorIconview();

	// We can only delete this list at the end, it would be better to delete this list
	// after a refresh and after drag_end is called...
	for (GList * l = this->itemDatalist; l != NULL; l = l->next)
	{
		ToolItemDragData * data = (ToolItemDragData *) l->data;
		g_object_unref(data->icon);
		g_free(data);
	}

	g_list_free(this->itemDatalist);
	this->itemDatalist = NULL;
	delete this->colorList;
	this->colorList = NULL;

	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);
}

void ToolbarCustomizeDialog::toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* unused)
{
	ToolItemDragCurrentData::setData(TOOL_ITEM_SEPARATOR, -1, NULL);

	GtkWidget* icon = Util::newSepeartorImage();
	gtk_drag_set_icon_pixbuf(context, ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon)), -2, -2);
	gtk_widget_unref(icon);
}

void ToolbarCustomizeDialog::toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused)
{
	ToolItemDragCurrentData::clearData();
}

void ToolbarCustomizeDialog::toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
														  GtkSelectionData* selection_data, guint info,
														  guint time, void* unused)
{

	ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(NULL);
	it->type = TOOL_ITEM_SEPARATOR;

	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
						   (const guchar*) it, sizeof(ToolItemDragDropData));

	g_free(it);
}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data)
{
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);

	ToolItemDragCurrentData::setData(TOOL_ITEM_ITEM, -1, data->item);

	gtk_drag_set_icon_pixbuf(context, data->icon, -2, -2);
	gtk_widget_hide(data->ebox);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data)
{
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	ToolItemDragCurrentData::clearData();
	gtk_widget_show(data->ebox);
}

void ToolbarCustomizeDialog::toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context,
												 GtkSelectionData* selection_data, guint info, guint time,
												 ToolItemDragData* data)
{

	g_return_if_fail(data != NULL);
	g_return_if_fail(data->item != NULL);

	data->item->setUsed(true);
	data->dlg->rebuildIconview();

	ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(data->item);

	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
						   (const guchar*) it, sizeof(ToolItemDragDropData));

	g_free(it);
}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context, void* data)
{
	int color = GPOINTER_TO_INT(data);
	ToolItemDragCurrentData::setDataColor(-1, color);

	GdkPixbuf* image = ToolbarDragDropHelper::getColorImage(color);

	gtk_drag_set_icon_pixbuf(context, image, -2, -2);

	g_object_unref(image);
	gtk_widget_hide(widget);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context, ToolbarCustomizeDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	ToolItemDragCurrentData::clearData();
	gtk_widget_show(widget);

	dlg->rebuildColorIcons();
}

void ToolbarCustomizeDialog::toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context,
													  GtkSelectionData* selection_data, guint info,
													  guint time, void* data)
{

	int color = GPOINTER_TO_INT(data);

	ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(NULL);
	it->color = color;
	it->type = TOOL_ITEM_COLOR;

	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
						   (const guchar*) it, sizeof(ToolItemDragDropData));

	g_free(it);
}

/**
 * A tool item was dragged to the dialog
 */
void ToolbarCustomizeDialog::dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y,
											  GtkSelectionData* data, guint info, guint time, ToolbarCustomizeDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	if (gtk_selection_data_get_data_type(data) != ToolbarDragDropHelper::atomToolItem)
	{
		gtk_drag_finish(dragContext, false, false, time);
		return;
	}

	ToolItemDragDropData* d = (ToolItemDragDropData*) gtk_selection_data_get_data(data);
	g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

	if (d->type == TOOL_ITEM_ITEM)
	{
		d->item->setUsed(false);
		dlg->rebuildIconview();
	}
	else if (d->type == TOOL_ITEM_SEPARATOR)
	{
		// simple ignore the separator
	}
	else if (d->type == TOOL_ITEM_COLOR)
	{
		dlg->win->getToolMenuHandler()->removeColorToolItem(d->item);
		dlg->rebuildColorIcons();
	}
	else
	{
		g_warning("ToolbarCustomizeDialog::dragDataReceived unhandled type: %i", d->type);
	}

	gtk_drag_finish(dragContext, true, false, time);
}

/**
 * clear the icon list
 */
void ToolbarCustomizeDialog::freeIconview()
{
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	GtkTable* table = GTK_TABLE(get("tbDefaultTools"));

	GList* children = gtk_container_get_children(GTK_CONTAINER(table));
	for (GList* l = children; l != NULL; l = l->next)
	{
		GtkWidget* w = (GtkWidget*) l->data;
		gtk_container_remove(GTK_CONTAINER(table), w);
	}

	g_list_free(children);
}

/**
 * builds up the icon list
 */
void ToolbarCustomizeDialog::rebuildIconview()
{
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	freeIconview();

	GtkTable* table = GTK_TABLE(get("tbDefaultTools"));

	int i = 0;
	for (AbstractToolItem* item : *this->win->getToolMenuHandler()->getToolItems())
	{
		if (item->isUsed())
		{
			continue;
		}

		string name = item->getToolDisplayName();
		GtkWidget* icon = item->getNewToolIcon();
		g_return_if_fail(icon != NULL);

		GtkWidget* box = gtk_vbox_new(false, 3);
		gtk_widget_show(box);

		GtkWidget* label = gtk_label_new(name.c_str());
		gtk_widget_show(label);
		gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

		GtkWidget* ebox = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(ebox), box);
		gtk_widget_show(ebox);

		gtk_widget_show(icon);

		gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

		// make ebox a drag source
		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

		ToolItemDragData* data = g_new(ToolItemDragData, 1);
		data->dlg = this;
		data->icon = ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon));
		data->item = item;
		data->ebox = ebox;

		this->itemDatalist = g_list_prepend(this->itemDatalist, data);

		g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), data);
		g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEnd), data);

		g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGet), data);

		int x = i % 3;
		int y = i / 3;
		gtk_table_attach(table, ebox, x, x + 1, y, y + 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);

		i++;
	}
}

/**
 * clear the icon list
 */
void ToolbarCustomizeDialog::freeColorIconview()
{
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	GtkTable* table = GTK_TABLE(get("tbColor"));

	GList* children = gtk_container_get_children(GTK_CONTAINER(table));
	for (GList* l = children; l != NULL; l = l->next)
	{
		GtkWidget* w = (GtkWidget*) l->data;
		gtk_container_remove(GTK_CONTAINER(table), w);
	}

	g_list_free(children);
}

void ToolbarCustomizeDialog::rebuildColorIcons()
{
	GtkTable* table = GTK_TABLE(get("tbColor"));
	g_return_if_fail(table != NULL);

	freeColorIconview();

	ToolMenuHandler* tmh = this->win->getToolMenuHandler();

	int i = 0;
	for (XojColor* color : *this->colorList->getPredefinedColors())
	{
		if (tmh->isColorInUse(color->getColor()))
		{
			continue;
		}

		GtkWidget* icon = selectcolor_new(color->getColor());
		selectcolor_set_size(icon, 16);
		selectcolor_set_circle(icon, true);

		GtkWidget* box = gtk_vbox_new(false, 3);
		gtk_widget_show(box);

		GtkWidget* label = gtk_label_new(color->getName().c_str());
		gtk_widget_show(label);
		gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

		GtkWidget* ebox = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(ebox), box);
		gtk_widget_show(ebox);

		gtk_widget_show(icon);

		gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

		// make ebox a drag source
		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

		g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemColorDragBegin), GINT_TO_POINTER(color->getColor()));
		g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemColorDragEnd), this);
		g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemColorDragDataGet), GINT_TO_POINTER(color->getColor()));

		int x = i % 5;
		int y = i / 5;
		i++;
		gtk_table_attach(table, ebox, x, x + 1, y, y + 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);
	}

	gtk_widget_show_all(GTK_WIDGET(table));
}

void ToolbarCustomizeDialog::windowResponseCb(GtkDialog* dialog, int response, ToolbarCustomizeDialog* dlg)
{
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	gtk_widget_hide(GTK_WIDGET(dialog));

	dlg->handler->toolbarConfigDialogClosed();
}

/**
 * Displays the dialog
 */
void ToolbarCustomizeDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	g_signal_connect(this->window, "response", G_CALLBACK(windowResponseCb), this);

	gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);

	gtk_widget_show_all(this->window);
}
