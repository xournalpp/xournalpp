#include "ToolbarCustomizeDialog.h"
#include "../toolbarMenubar/model/ToolbarData.h"
#include "../toolbarMenubar/model/ToolbarModel.h"

#include "../../gui/MainWindow.h"
#include "../../gui/toolbarMenubar/ToolMenuHandler.h"
#include "../../gui/toolbarMenubar/AbstractToolItem.h"

#include <config.h>
#include <glib/gi18n-lib.h>

static GdkPixbuf *
new_pixbuf_from_widget(GtkWidget * widget);

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

static void drag_begin(GtkWidget * widget, GdkDragContext * context, gpointer data) {
	GdkPixbuf * pixbuf = get_image_pixbuf(GTK_IMAGE (data));
	gtk_drag_set_icon_pixbuf(context, pixbuf, -2, -2);
	g_object_unref(pixbuf);
}

void drag_data_get(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data, guint info, guint time, AbstractToolItem * item) {
	String id = item->getId();

	gtk_selection_data_set(selection_data, atomToolItem, 0, (const guchar *) id.c_str(), id.size() + 1);
}

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath * gladeSearchPath, MainWindow * win) :
	GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
	XOJ_INIT_TYPE(ToolbarCustomizeDialog);

	this->win = win;

	static const GtkTargetEntry dropTargetEntry = { "move-buffer", GTK_TARGET_SAME_APP, 1 };

	char buffer[512];

	GtkTable * table = GTK_TABLE(get("tbDefaultTools"));

	ListIterator<AbstractToolItem *> it = win->getToolMenuHandler()->getToolItems();

	int i = 0;
	while (it.hasNext()) {
		AbstractToolItem * item = it.next();
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
			GdkPixbuf * pixbuf = new_pixbuf_from_widget(icon);
			icon = gtk_image_new_from_pixbuf(pixbuf);
			gdk_pixbuf_unref(pixbuf);
		}
		gtk_widget_show(icon);

		gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

		/* make ebox a drag source */
		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &dropTargetEntry, 1, GDK_ACTION_MOVE);
		gtk_drag_source_add_image_targets(ebox);
		g_signal_connect (ebox, "drag-begin",
				G_CALLBACK (drag_begin), icon);
		g_signal_connect (ebox, "drag-data-get",
				G_CALLBACK (drag_data_get), item);

		//			/* accept drops on ebox */
		//			gtk_drag_dest_set(ebox, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
		//			gtk_drag_dest_add_image_targets(ebox);
		//			g_signal_connect (ebox, "drag-data-received",
		//					G_CALLBACK (drag_data_received), image);


		int x = i % 3;
		int y = i / 3;
		gtk_table_attach(table, ebox, x, x + 1, y, y + 1, (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 5);

		i++;
	}
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
	XOJ_RELEASE_TYPE(ToolbarCustomizeDialog);
}

static void fake_expose_widget(GtkWidget * widget, GdkPixmap *pixmap) {
	GdkWindow *tmp_window;
	GdkEventExpose event;

	event.type = GDK_EXPOSE;
	event.window = pixmap;
	event.send_event = FALSE;
	event.area = widget->allocation;
	event.region = NULL;
	event.count = 0;

	tmp_window = widget->window;
	widget->window = pixmap;
	gtk_widget_send_expose(widget, (GdkEvent *) &event);
	widget->window = tmp_window;
}

/*
 * Source egg-editable-toolbar.c from evince
 *
 * We should probably experiment some more with this.
 * Right now the rendered icon is pretty good for most
 * themes. However, the icon is slightly large for themes
 * with large toolbar icons.
 */
static GdkPixbuf *
new_pixbuf_from_widget(GtkWidget * widget) {
	GtkWidget *window;
	GdkPixbuf *pixbuf;
	GtkRequisition requisition;
	GtkAllocation allocation;
	GdkPixmap *pixmap;
	GdkVisual *visual;
	gint icon_width;
	gint icon_height;
	GdkScreen *screen;

	icon_height = icon_width = 24;

	screen = gtk_widget_get_screen(widget);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_add(GTK_CONTAINER (window), widget);
	gtk_widget_realize(window);
	gtk_widget_show(widget);
	gtk_widget_realize(widget);
	gtk_widget_map(widget);

	/* Gtk will never set the width or height of a window to 0. So setting the width to
	 * 0 and than getting it will provide us with the minimum width needed to render
	 * the icon correctly, without any additional window background noise.
	 * This is needed mostly for pixmap based themes.
	 */
	gtk_window_set_default_size(GTK_WINDOW (window), icon_width, icon_height);
	gtk_window_get_size(GTK_WINDOW (window), &icon_width, &icon_height);

	gtk_widget_size_request(window, &requisition);
	allocation.x = 0;
	allocation.y = 0;
	allocation.width = icon_width;
	allocation.height = icon_height;
	gtk_widget_size_allocate(window, &allocation);
	gtk_widget_size_request(window, &requisition);

	/* Create a pixmap */
	visual = gtk_widget_get_visual(window);
	pixmap = gdk_pixmap_new(NULL, icon_width, icon_height, visual->depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE (pixmap), gtk_widget_get_colormap(window));

	/* Draw the window */
	gtk_widget_ensure_style(window);
	g_assert(window->style);
	g_assert(window->style->font_desc);

	fake_expose_widget(window, pixmap);
	fake_expose_widget(widget, pixmap);

	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, icon_width, icon_height);
	gdk_pixbuf_get_from_drawable(pixbuf, pixmap, NULL, 0, 0, 0, 0, icon_width, icon_height);

	gtk_widget_destroy(window);

	return pixbuf;
}

void ToolbarCustomizeDialog::show() {
	XOJ_CHECK_TYPE(ToolbarCustomizeDialog);

	this->win->startToolbarEditMode();
	gtk_dialog_run(GTK_DIALOG(this->window));
	gtk_widget_hide(this->window);
	this->win->endToolbarEditMode();
}
