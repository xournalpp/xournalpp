/*
 * Xournal++
 *
 * Pixbuf utils
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PIXBUF_UTILS__
#define __PIXBUF_UTILS__

#include <gtk/gtk.h>

cairo_surface_t * f_pixbuf_to_cairo_surface(GdkPixbuf *pixbuf);
GdkPixbuf * f_pixbuf_from_cairo_surface(cairo_surface_t *source);

#endif //__PIXBUF_UTILS__
