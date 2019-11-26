#include "GtkColorWrapper.h"

GtkColorWrapper::GtkColorWrapper() : red(0), green(0), blue(0)
{
}

GtkColorWrapper::GtkColorWrapper(const uint32_t color)
 : red((color >> 8U) & 0xff00U)
 , green(color & 0xff00U)
 , blue(color << 8U)
{
}

GtkColorWrapper::GtkColorWrapper(const GdkColor& color)
 : red(color.red), green(color.green), blue(color.blue)
{
}

GtkColorWrapper::GtkColorWrapper(const GdkRGBA& color)
 : red(color.red * 63535), green(color.green * 63535), blue(color.blue * 63535)
{
}

GtkColorWrapper::~GtkColorWrapper() = default;

/**
 * Apply the color to a cairo interface with "cairo_set_source_rgb"
 */
void GtkColorWrapper::apply(cairo_t* cr) const
{
	cairo_set_source_rgb(cr, red / 65536.0, green / 65536.0, blue / 65536.0);
}

/**
 * Apply the color to a cairo interface with "cairo_set_source_rgba" and a specified alpha value
 */
void GtkColorWrapper::applyWithAlpha(cairo_t* cr, double alpha) const
{
	cairo_set_source_rgba(cr, red / 65536.0, green / 65536.0, blue / 65536.0, alpha);
}
