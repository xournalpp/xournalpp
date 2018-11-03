#include "ToolbarUtil.h"

ToolbarUtil::ToolbarUtil() { }

ToolbarUtil::~ToolbarUtil() { }

#if !GTK3_ENABLED

GtkWidget* gtk_image_new_from_surface(cairo_surface_t* surface)
{
	int w = cairo_image_surface_get_width(surface);
	int h = cairo_image_surface_get_height(surface);
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
	unsigned char* sb = cairo_image_surface_get_data(surface);

	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	g_assert(gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
	g_assert(gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
	g_assert(gdk_pixbuf_get_has_alpha(pixbuf));
	g_assert(n_channels == 4);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			guchar * p = pixels + y * rowstride + x * n_channels;
			p[3] = *sb++;
			p[3] = 255; // TODO: Alpha
			p[0] = 255 - *sb++;
			p[1] = 255 - *sb++;
			p[2] = 255 - *sb++;
		}
	}

	GtkWidget* widget = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	return widget;
}

#endif


GtkWidget* ToolbarUtil::newSepeartorImage()
{
	cairo_surface_t* crImage = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 30, 30);
	cairo_t* cr = cairo_create(crImage);

	cairo_set_source_rgb(cr, 255, 0, 0);
	cairo_set_line_width(cr, 5);
	cairo_move_to(cr, 15, 0);
	cairo_line_to(cr, 15, 30);
	cairo_stroke(cr);
	cairo_destroy(cr);

	GtkWidget* w = gtk_image_new_from_surface(crImage);
	cairo_surface_destroy(crImage);
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
