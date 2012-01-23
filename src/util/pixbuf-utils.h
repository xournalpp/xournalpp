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

GdkPixbuf *
xoj_pixbuf_get_from_surface  (cairo_surface_t *surface,
                              gint             src_x,
                              gint             src_y,
                              gint             width,
                              gint             height);

#endif //__PIXBUF_UTILS__
