#include "ToolbarCustomizeDialog.h"
#include "../toolbarMenubar/model/ToolbarData.h"
#include "../toolbarMenubar/model/ToolbarModel.h"

#include "../../gui/MainWindow.h"
#include "../../gui/toolbarMenubar/ToolMenuHandler.h"
#include "../../gui/toolbarMenubar/AbstractToolItem.h"

#include <Util.h>

#include <config.h>
#include <glib/gi18n-lib.h>

static GdkAtom atomToolItem = gdk_atom_intern_static_string("application/xournal-ToolbarItem");

/**
 * Used for transport a Toolbar item
 */
class AbstractItemSelectionData {
public:
	AbstractItemSelectionData(AbstractToolItem * item, ToolbarCustomizeDialog * dlg) {
		this->item = item;
		this->dlg = dlg;
	}

	AbstractToolItem * item;
	ToolbarCustomizeDialog * dlg;
};


typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData {
	ToolbarCustomizeDialog * dlg;
	GdkPixbuf * icon;
	AbstractToolItem * item;
	GtkWidget * ebox;
};


void gtk_drag_dest_add_toolbar_targets(GtkWidget * target) {
	GtkTargetList * target_list;
	target_list = gtk_drag_dest_get_target_list(target);
	if (target_list) {
		gtk_target_list_ref(target_list);
	} else {
		target_list = gtk_target_list_new(NULL, 0);
	}

	// If not exist add, else do nothing
	if(!gtk_target_list_find(target_list, atomToolItem, NULL)) {
		gtk_target_list_add(target_list, atomToolItem, 0, NULL);
	}

	gtk_drag_dest_set_target_list(target, target_list);
	gtk_target_list_unref(target_list);
}

void gtk_drag_source_add_toolbar_targets(GtkWidget * widget) {
	GtkTargetList * target_list;

	target_list = gtk_drag_source_get_target_list(widget);
	if (target_list)
		gtk_target_list_ref(target_list);
	else
		target_list = gtk_target_list_new(NULL, 0);
	gtk_target_list_add(target_list, atomToolItem, 0, NULL);
	gtk_drag_source_set_target_list(widget, target_list);
	gtk_target_list_unref(target_list);
}

static const GtkTargetEntry dropTargetEntry = { "move-buffer", GTK_TARGET_SAME_APP, 1 };

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win) :
	GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

	this->win = win;
	this->currentDragItem = NULL;
	this->itemDatalist = NULL;
	this->itemSelectionData = NULL;

	rebuildIconview();

	//	GtkWidget * ebox = gtk_event_box_new();
	GtkWidget * target = get("viewport1");
	gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);

	gtk_drag_dest_add_toolbar_targets(target);

	g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);

	int len = 0;
	GtkWidget ** widgets = this->win->getToolbarWidgets(len);

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];
		gtk_drag_dest_set(w, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
		gtk_drag_dest_add_toolbar_targets(w);

		//		g_signal_connect (w, "drag_drop",
		//				G_CALLBACK (toolbar_drag_drop_cb), etoolbar);
		g_signal_connect(w, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
		g_signal_connect(w, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
		g_signal_connect(w, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

		// TODO: add context menu for editing
		//		g_signal_connect (w, "popup_context_menu",
		//				G_CALLBACK (popup_context_menu_cb), this);

	}
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	freeIconview();

	// We can only delete this list at the end, it would be better to delete this list after a refresh and after drag_end is called...
	for(GList * l = this->itemDatalist; l != NULL; l = l->next) {
		ToolItemDragData * data = (ToolItemDragData *)l->data;
		gdk_pixbuf_unref(data->icon);
		g_free(data);
	}

	g_list_free(this->itemDatalist);
	this->itemDatalist = NULL;


	int len = 0;
	GtkWidget ** widgets = this->win->getToolbarWidgets(len);

	for (int i = 0; i < len; i++) {
		GtkWidget * w = widgets[i];

		g_signal_handlers_disconnect_by_func(w, (gpointer)toolbarDragMotionCb, this);
		g_signal_handlers_disconnect_by_func(w, (gpointer)toolbarDragLeafeCb, this);
		g_signal_handlers_disconnect_by_func(w, (gpointer)toolbarDragDataReceivedCb, this);
	}

	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);
}

/**
 * Get a GDK Pixbuf from GTK widget image
 */
GdkPixbuf * ToolbarCustomizeDialog::getImagePixbuf(GtkImage * image) {
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

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data) {
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	data->dlg->currentDragItem = data->item;

	gtk_drag_set_icon_pixbuf(context, data->icon, -2, -2);
	gtk_widget_hide(data->ebox);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, ToolItemDragData * data) {
	XOJ_CHECK_TYPE_OBJ(data->dlg, ToolbarCustomizeDialog);
	data->dlg->currentDragItem = NULL;
	gtk_widget_show(data->ebox);
}

void ToolbarCustomizeDialog::toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data,
		guint info, guint time, AbstractItemSelectionData * item) {
	gtk_selection_data_set(selection_data, atomToolItem, 0, (const guchar *) item, sizeof(AbstractItemSelectionData));
}

/**
 * A tool item was dragged to the toolbar
 */
bool ToolbarCustomizeDialog::toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context,
		gint x, gint y, guint time, ToolbarCustomizeDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	GdkAtom target = gtk_drag_dest_find_target(GTK_WIDGET(toolbar), context, NULL);
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

void ToolbarCustomizeDialog::toolbarDragLeafeCb(GtkToolbar * toolbar, GdkDragContext * context,
		guint time, ToolbarCustomizeDialog * dlg) {
	XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);

	gtk_toolbar_set_drop_highlight_item(toolbar, NULL, -1);
}

void ToolbarCustomizeDialog::toolbarDragDataReceivedCb(GtkToolbar * toolbar, GdkDragContext * context,
		gint x, gint y, GtkSelectionData * data, guint info, guint time) {

	AbstractItemSelectionData * d = (AbstractItemSelectionData *)gtk_selection_data_get_data(data);

	XOJ_CHECK_TYPE_OBJ(d->dlg, ToolbarCustomizeDialog);


	int pos = gtk_toolbar_get_drop_index(toolbar, x, y);

	bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;
	GtkToolItem * it = d->item->createItem(horizontal);
	gtk_widget_show_all(GTK_WIDGET(it));
	gtk_toolbar_insert(toolbar, it, pos);

	d->item->setUsed(true);

	d->dlg->rebuildIconview();

	ToolbarData * tb = d->dlg->win->getSelectedToolbar();
	String name = d->dlg->win->getToolbarName(toolbar);

	tb->addItem(name, d->item->getId(), pos);

// TODO Implement toolbarDragDataReceivedCb
}

/**
 * A tool item was dragged to the dialog
 */
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

/**
 * Remove a toolbar item from the tool where it was
 */
void ToolbarCustomizeDialog::removeFromToolbar(AbstractToolItem * item) {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	// TODO implement
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


	for(GList * l = this->itemSelectionData; l != NULL; l = l->next) {
		delete (AbstractItemSelectionData *)l->data;
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
		gtk_drag_source_add_toolbar_targets(ebox);

		ToolItemDragData * data = g_new(ToolItemDragData, 1);
		data->dlg = this;
		data->icon = getImagePixbuf(GTK_IMAGE(icon));;
		data->item = item;
		data->ebox = ebox;

		this->itemDatalist = g_list_prepend(this->itemDatalist, data);

		g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), data);
		g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEnd), data);

		AbstractItemSelectionData * sd = new AbstractItemSelectionData(item, this);
		g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGet), sd);
		this->itemSelectionData = g_list_append(this->itemSelectionData, sd);

		int x = i % 3;
		int y = i / 3;
		gtk_table_attach(table, ebox, x, x + 1, y, y + 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);

		i++;
	}
}

/**
 * Displays the dialog
 */
void ToolbarCustomizeDialog::show() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	this->win->startToolbarEditMode();
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
	this->win->endToolbarEditMode();
}
