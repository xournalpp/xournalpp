#include "ColorSelectImage.h"

#include <pixbuf-utils.h>

#include <cmath>

ColorSelectImage::ColorSelectImage(int color, bool circle)
 : color(color),
   circle(circle)
{
	widget = gtk_drawing_area_new();
	gtk_widget_set_size_request(widget, 16, 16);

	g_signal_connect(widget, "draw", G_CALLBACK(
		+[](GtkWidget *widget, cairo_t* cr, ColorSelectImage* self)
		{ self->drawWidget(cr); }), this);
}

ColorSelectImage::~ColorSelectImage()
{
}

/**
 * Draw the widget
 */
void ColorSelectImage::drawWidget(cairo_t* cr)
{
	IconConfig config;
	config.color = color;
	config.size = size;
	config.state = COLOR_ICON_STATE_ENABLED;
	config.circle = circle;
	config.state = state;
	config.width = gtk_widget_get_allocated_width(widget);
	config.height = gtk_widget_get_allocated_height(widget);

	drawWidget(cr, config);
}

/**
 * @return The widget which is drawn
 */
GtkWidget* ColorSelectImage::getWidget()
{
	return widget;
}

/**
 * Color of the icon
 */
void ColorSelectImage::setColor(int color)
{
	this->color = color;
	gtk_widget_queue_draw(widget);
}

/**
 * Set State of the Icon
 */
void ColorSelectImage::setState(ColorIconState state)
{
	this->state = state;
	gtk_widget_queue_draw(widget);
}

/**
 * Create a new GtkImage with preview color
 */
GtkWidget* ColorSelectImage::newColorIcon(int color, int size, bool circle)
{
	cairo_surface_t* surface = newColorIconSurface(color, size, circle);
	GtkWidget* w = gtk_image_new_from_surface(surface);
	cairo_surface_destroy(surface);

	return w;
}

/**
 * Draw the widget
 */
void ColorSelectImage::drawWidget(cairo_t* cr, const IconConfig& config)
{
	float alpha = 1.0;
	if (config.state == COLOR_ICON_STATE_DISABLED)
	{
		alpha = 0.5;
	}

	// Fill transparent
	cairo_set_source_rgba(cr, 1, 1, 1, 0);
	cairo_fill(cr);

	int y = (config.height - config.size) / 2;

	double r = ((config.color & 0xff0000) >> 16) / 255.0;
	double g = ((config.color & 0xff00) >> 8) / 255.0;
	double b = ((config.color & 0xff)) / 255.0;
	cairo_set_source_rgba(cr, r, g, b, alpha);

	int x = 0;
	int width = config.size;

	double radius = config.size / 2.0;

	if (config.circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0, 0, 0, alpha);

	if (config.circle)
	{
		cairo_arc(cr, radius + x, radius + y, radius - 1, 0, 2 * M_PI);
	}
	else
	{
		cairo_rectangle(cr, x + 1, y + 1, width - 2, width - 2);
	}

	cairo_set_line_width(cr, 0.8);
	cairo_stroke(cr);


	if (config.state == COLOR_ICON_STATE_PEN)
	{
		// Pencil cursor from cursor drawing, a little shrinked, so that it fits to the color item
		cairo_move_to(cr, x, y + 16);
		cairo_line_to(cr, x, y + 16 - 4);
		cairo_line_to(cr, x + 13, y + 16 - 16);
		cairo_line_to(cr, x + 16, y + 16 - 14);
		cairo_line_to(cr, x + 4, y + 16);

		cairo_close_path(cr);

		cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
		cairo_fill_preserve(cr);

		cairo_set_source_rgba(cr, 0, 0, 0, 0.7);
		cairo_set_line_width(cr, 0.8);
		cairo_stroke(cr);
	}
}

/**
 * Create a new cairo_surface_t* with preview color
 */
cairo_surface_t* ColorSelectImage::newColorIconSurface(int color, int size, bool circle)
{
	cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
	cairo_t* cr = cairo_create(crBuffer);

	IconConfig config;
	config.color = color;
	config.size = size;
	config.state = COLOR_ICON_STATE_ENABLED;
	config.circle = circle;

	drawWidget(cr, config);
	cairo_destroy(cr);

	return crBuffer;
}

/**
 * Create a new GdkPixbuf* with preview color
 */
GdkPixbuf* ColorSelectImage::newColorIconPixbuf(int color, int size, bool circle)
{
	cairo_surface_t* surface = newColorIconSurface(color, size, circle);
	GdkPixbuf* pixbuf = xoj_pixbuf_get_from_surface(surface, 0, 0, cairo_image_surface_get_width(surface), cairo_image_surface_get_height(surface));
	cairo_surface_destroy(surface);

	return pixbuf;
}


