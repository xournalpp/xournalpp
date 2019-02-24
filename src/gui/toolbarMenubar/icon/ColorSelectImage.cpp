#include "ColorSelectImage.h"

#include <pixbuf-utils.h>

#include <math.h>

ColorSelectImage::ColorSelectImage(int color, bool circle)
 : color(color),
   circle(circle)
{
	XOJ_INIT_TYPE(ColorSelectImage);

	widget = gtk_drawing_area_new();
	gtk_widget_set_size_request(widget, 16, 16);

	g_signal_connect(widget, "draw", G_CALLBACK(
		+[](GtkWidget *widget, cairo_t* cr, ColorSelectImage* self)
		{
			XOJ_CHECK_TYPE_OBJ(self, ColorSelectImage);
			self->drawWidget(cr);
		}), this);

}

ColorSelectImage::~ColorSelectImage()
{
	XOJ_CHECK_TYPE(ColorSelectImage);

	XOJ_RELEASE_TYPE(ColorSelectImage);
}

/**
 * Draw the widget
 */
void ColorSelectImage::drawWidget(cairo_t* cr)
{
	XOJ_CHECK_TYPE(ColorSelectImage);

	int y = (gtk_widget_get_allocated_height(widget) - size) / 2;
	drawWidget(cr, color, size, y, circle);
}

/**
 * @return The widget which is drawed
 */
GtkWidget* ColorSelectImage::getWidget()
{
	XOJ_CHECK_TYPE(ColorSelectImage);

	return widget;
}

/**
 * Color of the icon
 */
void ColorSelectImage::setColor(int color)
{
	XOJ_CHECK_TYPE(ColorSelectImage);

	this->color = color;
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
void ColorSelectImage::drawWidget(cairo_t* cr, int color, int size, int y, bool circle)
{
	cairo_set_source_rgba(cr, 1, 1, 1, 0);
	cairo_fill(cr);

	double r = ((color & 0xff0000) >> 16) / 255.0;
	double g = ((color & 0xff00) >> 8) / 255.0;
	double b = ((color & 0xff)) / 255.0;
	cairo_set_source_rgb(cr, r, g, b);

	int x = 0;
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
}

/**
 * Create a new cairo_surface_t* with preview color
 */
cairo_surface_t* ColorSelectImage::newColorIconSurface(int color, int size, bool circle)
{
	cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
	cairo_t* cr = cairo_create(crBuffer);
	drawWidget(cr, color, size, 0, circle);
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


