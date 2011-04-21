#include "ToolbarCustomizeDialog.h"
#include "../toolbarMenubar/model/ToolbarData.h"
#include "../toolbarMenubar/model/ToolbarModel.h"

#include "../../gui/MainWindow.h"
#include "../../gui/toolbarMenubar/ToolMenuHandler.h"
#include "../../gui/toolbarMenubar/AbstractToolItem.h"

#include "../../util/Util.h"

#include <config.h>
#include <glib/gi18n-lib.h>

static GdkAtom atomToolItem = gdk_atom_intern_static_string("application/xournal-ToolbarItem");

static GdkPixbuf *
get_image_pixbuf(GtkImage *image) {
	gchar * stock_id;
	GtkIconSize size;

	switch (gtk_image_get_storage_type(image)) {
	case GTK_IMAGE_PIXBUF:
		return (GdkPixbuf *) g_object_ref(gtk_image_get_pixbuf(image));
	case GTK_IMAGE_STOCK:
		gtk_image_get_stock(image, &stock_id, &size);
		return gtk_widget_render_icon(GTK_WIDGET (image), stock_id, size, NULL);
	default:
		g_warning("Image storage type %d not handled", gtk_image_get_storage_type(image));
		return NULL;
	}
}


typedef struct {
	AbstractToolItem * item;
} AbstractItemSelectionData;

void drag_data_get(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data, guint info, guint time, AbstractToolItem * item) {
	AbstractItemSelectionData * d = g_new(AbstractItemSelectionData, 1);
	d->item = item;
	gtk_selection_data_set(selection_data, atomToolItem, 0, (const guchar *) d, sizeof(AbstractItemSelectionData));
	g_free(d);
}

void gtk_drag_dest_add_toolbar_targets(GtkWidget * target) {
	GtkTargetList * target_list;
	target_list = gtk_drag_dest_get_target_list(target);
	if (target_list) {
		gtk_target_list_ref(target_list);
	} else {
		target_list = gtk_target_list_new(NULL, 0);
	}
	gtk_target_list_add_image_targets(target_list, 0, FALSE);

	gtk_target_list_add(target_list, atomToolItem, 0, NULL);

	gtk_drag_dest_set_target_list(target, target_list);
	gtk_target_list_unref(target_list);
}

static const GtkTargetEntry dropTargetEntry = { "move-buffer", GTK_TARGET_SAME_APP, 1 };

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win) :
	GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

	this->win = win;
	this->currentDragItem = NULL;

	rebuildIconview();

	//	GtkWidget * ebox = gtk_event_box_new();
	GtkWidget * target = get("viewport1");
	gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);

	gtk_drag_dest_add_toolbar_targets(target);

	g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);

	// TODO only once / remove after
	int len = 0;
	GtkWidget ** widgets = this->win->getToolbarWidgets(len);

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];
		gtk_drag_dest_set(w, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
		gtk_drag_dest_add_toolbar_targets(w);

		//		g_signal_connect (w, "drag_drop",
		//				G_CALLBACK (toolbar_drag_drop_cb), etoolbar);
		g_signal_connect(w, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);

		//		g_signal_connect (w, "drag_leave",
		//				G_CALLBACK (toolbar_drag_leave_cb), etoolbar);
		//
		//		g_signal_connect (w, "drag_data_received",
		//				G_CALLBACK (toolbar_drag_data_received_cb), etoolbar);

		//		g_signal_connect (w, "popup_context_menu",
		//				G_CALLBACK (popup_context_menu_cb), etoolbar);

	}
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);
}

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData {
	ToolbarCustomizeDialog * dlg;
	GdkPixbuf * icon;
	AbstractToolItem * item;
};

void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data) {
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	data->dlg->currentDragItem = data->item;

	gtk_drag_set_icon_pixbuf(context, data->icon, -2, -2);
}

bool ToolbarCustomizeDialog::toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context,
		gint x, gint y, guint time, ToolbarCustomizeDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	GdkAtom target = gtk_drag_dest_find_target(GTK_WIDGET (toolbar), context, NULL);
	if (target != atomToolItem) {
		gdk_drag_status(context, (GdkDragAction) 0, time);
		return false;
	}

	gint ipos = gtk_toolbar_get_drop_index(toolbar, x, y);
	GtkOrientation orientation = gtk_toolbar_get_orientation(toolbar);
	gtk_toolbar_set_drop_highlight_item(toolbar, dlg->currentDragItem->createItem(orientation == GTK_ORIENTATION_HORIZONTAL), ipos);

	gdk_drag_status(context, context->suggested_action, time);

	return true;
}

void ToolbarCustomizeDialog::dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext,
		gint x, gint y, GtkSelectionData * data, guint info,
		guint time, ToolbarCustomizeDialog * dlg) {

	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	if (gtk_selection_data_get_data_type(data) != atomToolItem) {
		gtk_drag_finish(dragContext, false, false, time);
		return;
	}

	AbstractItemSelectionData * d = NULL;
	d = (AbstractItemSelectionData *) gtk_selection_data_get_data(data);

	AbstractToolItem * item = d->item;

	if (item->isUsed()) {
		dlg->removeFromToolbar(item);
		dlg->rebuildIconview();
	}

	gtk_drag_finish(dragContext, true, false, time);
}

void ToolbarCustomizeDialog::removeFromToolbar(AbstractToolItem * item) {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

}

void ToolbarCustomizeDialog::freeIconview() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	GtkTable * table = GTK_TABLE(get("tbDefaultTools"));

	GList * children = gtk_container_get_children(GTK_CONTAINER(table));
	for (GList * l = children; l != NULL; l = l->next) {
		GtkWidget * w = (GtkWidget *) l->data;
		gtk_container_remove(GTK_CONTAINER(table), w);
	}

	g_list_free(children);
}

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
		gtk_container_add(GTK_CONTAINER (ebox), box);
		gtk_widget_show(ebox);

		if (!GTK_IS_IMAGE(icon)) {
			GdkPixbuf * pixbuf = Util::newPixbufFromWidget(icon);
			icon = gtk_image_new_from_pixbuf(pixbuf);
			gdk_pixbuf_unref(pixbuf);
		}
		gtk_widget_show(icon);

		gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

		/* make ebox a drag source */
		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &dropTargetEntry, 1, GDK_ACTION_MOVE);
		gtk_drag_source_add_image_targets(ebox);

		// TODO: free this data!! MEMORY LEAK!
		ToolItemDragData * data = g_new(ToolItemDragData, 1);
		data->dlg = this;
		data->icon = get_image_pixbuf(GTK_IMAGE(icon));;
		data->item = item;

		g_signal_connect (ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), item);
		g_signal_connect (ebox, "drag-data-get", G_CALLBACK(drag_data_get), item);

		int x = i % 3;
		int y = i / 3;
		gtk_table_attach(table, ebox, x, x + 1, y, y + 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);

		i++;
	}
}

void ToolbarCustomizeDialog::show() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	this->win->startToolbarEditMode();
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
	this->win->endToolbarEditMode();
}
