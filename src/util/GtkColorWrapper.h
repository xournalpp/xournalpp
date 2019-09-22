/*
 * Xournal++
 *
 * Helper functions to wrap GdkColor/GdkRGBA color types used by GTK2 / GTK3
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdk.h>
#include <XournalType.h>
#include <cairo.h>

class GtkColorWrapper
{
public:
	GtkColorWrapper();
	GtkColorWrapper(const int color);
	GtkColorWrapper(const GdkColor& color);
	GtkColorWrapper(const GdkRGBA& color);
	virtual ~GtkColorWrapper();

public:
	/**
	 * Apply the color to a cairo interface with "cairo_set_source_rgb"
	 */
	void apply(cairo_t* cr);

	/**
	 * Apply the color to a cairo interface with "cairo_set_source_rgba" and a specified alpha value
	 */
	void applyWithAlpha(cairo_t* cr, double alpha);

public:
	/**
	 * Color values, 0-65535
	 */
	guint16 red;
	guint16 green;
	guint16 blue;

private:
	};
