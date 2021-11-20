/*
 * Xournal++
 *
 * Pixbuf utils
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

cairo_surface_t* f_pixbuf_to_cairo_surface(GdkPixbuf* pixbuf);

GdkPixbuf* xoj_pixbuf_get_from_surface(cairo_surface_t* surface, gint src_x, gint src_y, gint width, gint height);
