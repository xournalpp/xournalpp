#include "ToolbarCustomizeDialog.h"
#include "../../toolbarMenubar/model/ToolbarData.h"
#include "../../toolbarMenubar/model/ToolbarModel.h"

#include "../../../gui/MainWindow.h"
#include "../../../gui/toolbarMenubar/ToolMenuHandler.h"
#include "../../../gui/toolbarMenubar/AbstractToolItem.h"

#include "ToolbarDragDropHandler.h"
#include "ToolbarDragDropHelper.h"

#include <Util.h>

#include <config.h>
#include <glib/gi18n-lib.h>

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData {
	ToolbarCustomizeDialog * dlg;
	GdkPixbuf * icon;
	AbstractToolItem * item;
	GtkWidget * ebox;
};

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win,
		ToolbarDragDropHandler * handler) :
	GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

	this->win = win;
	this->handler = handler;
	this->itemDatalist = NULL;
	this->itemSelectionData = NULL;

	rebuildIconview();

	GtkWidget * target = get("viewport1");

	// prepare drag & drop
	gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
	ToolbarDragDropHelper::dragDestAddToolbar(target);

	g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	freeIconview();

	// We can only delete this list at the end, it would be better to delete this list after a refresh and after drag_end is called...
	for (GList * l = this->itemDatalist; l != NULL; l = l->next) {
		ToolItemDragData * data = (ToolItemDragData *) l->data;
		gdk_pixbuf_unref(data->icon);
		g_free(data);
	}

	g_list_free(this->itemDatalist);
	this->itemDatalist = NULL;

	this->cleanupToolbarsItemsDrag();

	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);
}

void ToolbarCustomizeDialog::cleanupToolbarsItemsDrag() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	//	int len = 0;
	//	GtkWidget ** widgets = this->win->getToolbarWidgets(len);
	//
	//	for (int i = 0; i < len; i++) {
	//		GtkToolbar * tb = GTK_TOOLBAR(widgets[i]);
	//
	//		int count = gtk_toolbar_get_n_items(tb);
	//		for (int j = 0; j < count; j++) {
	//			GtkToolItem * it = gtk_toolbar_get_nth_item(tb, j);
	//			AbstractToolItem * ti = xoj_tool_item_get_abstract_tool_item(it);
	//
	//			if(ti == NULL) {
	//				g_warning("ToolbarItem %i/%i is not an XOJ_TOOLBAR_ITEM!", i, j);
	//				continue;
	//			}
	//
	//			ti->enableDrag();
	//		}
	//	}

}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data) {
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	ToolbarAdapter::currentDragItem = data->item;

	gtk_drag_set_icon_pixbuf(context, data->icon, -2, -2);
	gtk_widget_hide(data->ebox);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data) {
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	ToolbarAdapter::currentDragItem = NULL;
	gtk_widget_show(data->ebox);
}

void ToolbarCustomizeDialog::toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context,
		GtkSelectionData * selection_data, guint info, guint time, ToolItemDragData * data) {

	g_return_if_fail(data != NULL);
	g_return_if_fail(data->item != NULL);

	data->item->setUsed(true);
	data->dlg->rebuildIconview();

	ToolItemDragDropData * it = ToolitemDragDrop::ToolItemDragDropData_new(data->item);

	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, (const guchar *) it,
			sizeof(ToolItemDragDropData));

	g_free(it);
}

/**
 * A tool item was dragged to the dialog
 */
void ToolbarCustomizeDialog::dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext, gint x, gint y,
		GtkSelectionData * data, guint info, guint time, ToolbarCustomizeDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	if (gtk_selection_data_get_data_type(data) != ToolbarDragDropHelper::atomToolItem) {
		gtk_drag_finish(dragContext, false, false, time);
		return;
	}

	ToolItemDragDropData * d = (ToolItemDragDropData *) gtk_selection_data_get_data(data);
	g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

	// TODO: !!!!!!!!!!!!!
	if (d->type == TOOL_ITEM_ITEM) {
//		dlg->removeFromToolbar(item, d->toolbar, d->id);
		d->item->setUsed(false);
		dlg->rebuildIconview();
	}

	gtk_drag_finish(dragContext, true, false, time);
}

/**
 * Remove a toolbar item from the tool where it was
 */
void ToolbarCustomizeDialog::removeFromToolbar(AbstractToolItem * item, String toolbarName, int id) {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	ToolbarData * d = this->win->getSelectedToolbar();
	if (d->removeItemByID(toolbarName, id)) {
		printf("Removed tool item %s from Toolbar %s ID %i\n", item->getId().c_str(), toolbarName.c_str(), id);
	} else {
		printf("Could not removed tool item %s from Toolbar %s Position %i\n", item->getId().c_str(), toolbarName.c_str(), id);
	}

	this->win->reloadToolbars();
}

/**
 * clear the icon list
 */
void ToolbarCustomizeDialog::freeIconview() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	GtkTable * table = GTK_TABLE(get("tbDefaultTools"));

	GList * children = gtk_container_get_children(GTK_CONTAINER(table));
	for (GList * l = children; l != NULL; l = l->next) {
		GtkWidget * w = (GtkWidget *) l->data;
		gtk_container_remove(GTK_CONTAINER(table), w);
	}

	g_list_free(children);

	for (GList * l = this->itemSelectionData; l != NULL; l = l->next) {
		delete (AbstractItemSelectionData *) l->data;
	}
	g_list_free(this->itemSelectionData);
	this->itemSelectionData = NULL;
}

/**
 * builds up the icon list
 */
void ToolbarCustomizeDialog::rebuildIconview() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	freeIconview();

	GtkTable * table = GTK_TABLE(get("tbDefaultTools"));

	ListIterator<AbstractToolItem *> it = this->win->getToolMenuHandler()->getToolItems();

	int i = 0;
	while (it.hasNext()) {
		AbstractToolItem * item = it.next();

		if (item->isUsed()) {
			continue;
		}

		String name = item->getToolDisplayName();
		GtkWidget * icon = item->getNewToolIcon();
		g_return_if_fail(icon != NULL);

		GtkWidget * box = gtk_vbox_new(false, 3);
		gtk_widget_show(box);

		GtkWidget * label = gtk_label_new(name.c_str());
		gtk_widget_show(label);
		gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

		GtkWidget * ebox = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(ebox), box);
		gtk_widget_show(ebox);

		if (!GTK_IS_IMAGE(icon)) {
			GdkPixbuf * pixbuf = Util::newPixbufFromWidget(icon);
			icon = gtk_image_new_from_pixbuf(pixbuf);
			gdk_pixbuf_unref(pixbuf);
		}
		gtk_widget_show(icon);

		gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

		// make ebox a drag source
		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

		ToolItemDragData * data = g_new(ToolItemDragData, 1);
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

void ToolbarCustomizeDialog::windowResponseCb(GtkDialog * dialog, int response, ToolbarCustomizeDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	gtk_widget_hide(GTK_WIDGET(dialog));

	dlg->handler->toolbarConfigDialogClosed();
}

/**
 * Displays the dialog
 */
void ToolbarCustomizeDialog::show() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	g_signal_connect(this->window, "response", G_CALLBACK(windowResponseCb), this);

	gtk_window_set_transient_for(GTK_WINDOW(this->window), GTK_WINDOW(this->win->getWindow()));

	gtk_widget_show_all(this->window);

}
