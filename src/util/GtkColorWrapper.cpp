#include "GtkColorWrapper.h"

GtkColorWrapper::GtkColorWrapper() : red(0), green(0), blue(0)
{
	XOJ_INIT_TYPE(GtkColorWrapper);
}

GtkColorWrapper::GtkColorWrapper(const GdkColor& color) : red(color.red), green(color.green), blue(color.blue)
{
	XOJ_INIT_TYPE(GtkColorWrapper);
}

GtkColorWrapper::GtkColorWrapper(const GdkRGBA& color) : red(color.red), green(color.green), blue(color.blue)
{
	XOJ_INIT_TYPE(GtkColorWrapper);
}

GtkColorWrapper::~GtkColorWrapper()
{
	XOJ_RELEASE_TYPE(GtkColorWrapper);
}

/**
 * Apply the color to a cairo interface with "cairo_set_source_rgb"
 */
void GtkColorWrapper::apply(cairo_t* cr)
{
	XOJ_CHECK_TYPE(GtkColorWrapper);
	cairo_set_source_rgb(cr, red / 65536.0, green / 65536.0, blue / 65536.0);
}

/**
 * Apply the color to a cairo interface with "cairo_set_source_rgba" and a specified alpha value
 */
void GtkColorWrapper::applyWithAlpha(cairo_t* cr, double alpha)
{
	XOJ_CHECK_TYPE(GtkColorWrapper);
	cairo_set_source_rgba(cr, red / 65536.0, green / 65536.0, blue / 65536.0, alpha);
}
