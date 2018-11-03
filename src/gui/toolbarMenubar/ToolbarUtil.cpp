#include "ToolbarUtil.h"

ToolbarUtil::ToolbarUtil() { }

ToolbarUtil::~ToolbarUtil() { }

GtkWidget* ToolbarUtil::newSepeartorImage()
{
	GtkWidget* separator = gtk_vseparator_new();
	GdkPixbuf* pixbuf = ToolbarUtil::newPixbufFromWidget(separator);
	gtk_widget_unref(separator);
	GtkWidget * w = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	return w;
}

void ToolbarUtil::fakeExposeWidget(GtkWidget* widget, GdkPixmap* pixmap)
{
	GdkWindow* tmp_window;
	GdkEventExpose event;

	event.type = GDK_EXPOSE;
	event.window = pixmap;
	event.send_event = FALSE;
	event.area = widget->allocation;
	event.region = NULL;
	event.count = 0;

	tmp_window = widget->window;
	widget->window = pixmap;
	gtk_widget_send_expose(widget, (GdkEvent*) &event);
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
GdkPixbuf* ToolbarUtil::newPixbufFromWidget(GtkWidget* widget, int iconSize)
{
	GtkWidget* window;
	GdkPixbuf* pixbuf;
	GtkRequisition requisition;
	GtkAllocation allocation;
	GdkPixmap* pixmap;
	GdkVisual* visual;
	gint icon_width;
	gint icon_height;

	icon_height = icon_width = iconSize;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_widget_realize(window);
	gtk_widget_show(widget);
	gtk_widget_realize(widget);
	gtk_widget_map(widget);

	/* Gtk will never set the width or height of a window to 0. So setting the width to
	 * 0 and than getting it will provide us with the minimum width needed to render
	 * the icon correctly, without any additional window background noise.
	 * This is needed mostly for pixmap based themes.
	 */
	gtk_window_set_default_size(GTK_WINDOW(window), icon_width, icon_height);
	gtk_window_get_size(GTK_WINDOW(window), &icon_width, &icon_height);

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
	gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), gtk_widget_get_colormap(window));

	/* Draw the window */
	gtk_widget_ensure_style(window);
	g_assert(window->style);
	g_assert(window->style->font_desc);

	fakeExposeWidget(window, pixmap);
	fakeExposeWidget(widget, pixmap);

	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, icon_width, icon_height);
	gdk_pixbuf_get_from_drawable(pixbuf, pixmap, NULL, 0, 0, 0, 0, icon_width, icon_height);

	gtk_widget_destroy(window);

	return pixbuf;
}
