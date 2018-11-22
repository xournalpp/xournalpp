#include "ToolbarUtil.h"

#include <pixbuf-utils.h>

#include <math.h>

ToolbarUtil::ToolbarUtil() { }

ToolbarUtil::~ToolbarUtil() { }

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

	double radius = size / 2.0;

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
	GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(surface, 0, 0, cairo_image_surface_get_width(surface), cairo_image_surface_get_height(surface));
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

