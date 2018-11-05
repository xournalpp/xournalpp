#include "ToolbarUtil.h"

#include <math.h>

ToolbarUtil::ToolbarUtil() { }

ToolbarUtil::~ToolbarUtil() { }


GdkPixbuf* gdk_pixbuf_new_from_surface(cairo_surface_t* surface)
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

			// blue
			p[2] = *sb++;

			// green
			p[1] = *sb++;

			// red
			p[0] = *sb++;

			// alpha
			p[3] = *sb++;
		}
	}

	return pixbuf;
}

/**
 * Create a new GtkImage with preview color
 */
GtkWidget* ToolbarUtil::newColorIcon(int color, int size, bool circle)
{
	cairo_surface_t* surface = newColorIconSurface(color, size, circle);
	GtkWidget* widget = gtk_image_new_from_surface(surface);
	cairo_surface_destroy(surface);

	return widget;
}

/**
 * Create a new cairo_surface_t* with preview color
 */
cairo_surface_t* ToolbarUtil::newColorIconSurface(int color, int size, bool circle)
{
	cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
	cairo_t* cr = cairo_create(crBuffer);
	cairo_set_source_rgba(cr, 1, 1, 1, 0);
	cairo_fill(cr);

	double r = ((color & 0xff0000) >> 16) / 255.0;
	double g = ((color & 0xff00) >> 8) / 255.0;
	double b = ((color & 0xff)) / 255.0;
	cairo_set_source_rgb(cr, r, g, b);

	int x = 0;
	int y = 0;
	int width = size;

	double radius = size / 2;

	if (circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);

	if (circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}

	cairo_set_line_width(cr, 0.8);
	cairo_stroke(cr);
	cairo_destroy(cr);

	return crBuffer;
}

/**
 * Create a new GdkPixbuf* with preview color
 */
GdkPixbuf* ToolbarUtil::newColorIconPixbuf(int color, int size, bool circle)
{
	cairo_surface_t* surface = newColorIconSurface(color, size, circle);
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_surface(surface);
	cairo_surface_destroy(surface);

	return pixbuf;
}

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

