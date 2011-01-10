/*
 * Xournal++
 *
 * Pixbuf utils
 *
 * Copied from F-Spot
 */

/* f-pixbuf-utils.c
 *
 * Copyright (C) 2001, 2002, 2003 The Free Software Foundation, Inc.
 * Copyright (C) 2003 Ettore Perazzoli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Paolo Bacchilega <paolo.bacch@tin.it>
 *
 * Adapted by Ettore Perazzoli <ettore@perazzoli.org>
 */

/* Some bits are based upon the GIMP source code, the original copyright
 * note follows:
 *
 * The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 */

#include <config.h>

#include "pixbuf-utils.h"

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <gdk/gdk.h>

const cairo_user_data_key_t pixel_key = { 0 };
const cairo_user_data_key_t format_key = { 0 };

cairo_surface_t *
f_image_surface_create(cairo_format_t format, int width, int height) {
	int size;
	cairo_surface_t *surface;
	unsigned char *pixels;

	switch (format) {
	case CAIRO_FORMAT_ARGB32:
	case CAIRO_FORMAT_RGB24:
		size = 4;
		break;
	case CAIRO_FORMAT_A8:
		size = 8;
		break;
	case CAIRO_FORMAT_A1:
		size = 1;
		break;
	}

	pixels = (unsigned char *)g_malloc(width * height * size);
	surface = cairo_image_surface_create_for_data(pixels, format, width, height, width * size);

	cairo_surface_set_user_data(surface, &pixel_key, pixels, g_free);
	cairo_surface_set_user_data(surface, &format_key, GINT_TO_POINTER (format), NULL);

	return surface;
}

void *
f_image_surface_get_data(cairo_surface_t *surface) {
	return cairo_surface_get_user_data(surface, &pixel_key);
}

cairo_format_t f_image_surface_get_format(cairo_surface_t *surface) {
	return (cairo_format_t)GPOINTER_TO_INT (cairo_surface_get_user_data (surface, &format_key));
}

int f_image_surface_get_width(cairo_surface_t *surface) {
	return cairo_image_surface_get_width(surface);
}

int f_image_surface_get_height(cairo_surface_t *surface) {
	return cairo_image_surface_get_height(surface);
}

/* Public functions.  */

cairo_surface_t *
f_pixbuf_to_cairo_surface(GdkPixbuf *pixbuf) {
	gint width = gdk_pixbuf_get_width(pixbuf);
	gint height = gdk_pixbuf_get_height(pixbuf);
	guchar *gdk_pixels = gdk_pixbuf_get_pixels(pixbuf);
	int gdk_rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	guchar *cairo_pixels;
	cairo_format_t format;
	cairo_surface_t *surface;
	int j;

	if (n_channels == 3)
		format = CAIRO_FORMAT_RGB24;
	else
		format = CAIRO_FORMAT_ARGB32;

	surface = f_image_surface_create(format, width, height);
	cairo_pixels = (guchar *) f_image_surface_get_data(surface);

	for (j = height; j; j--) {
		guchar *p = gdk_pixels;
		guchar *q = cairo_pixels;

		if (n_channels == 3) {
			guchar *end = p + 3 * width;

			while (p < end) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
				q[0] = p[2];
				q[1] = p[1];
				q[2] = p[0];
#else	  
				q[1] = p[0];
				q[2] = p[1];
				q[3] = p[2];
#endif
				p += 3;
				q += 4;
			}
		} else {
			guchar *end = p + 4 * width;
			guint t1, t2, t3;

#define MULT(d,c,a,t) G_STMT_START { t = c * a + 0x7f; d = ((t >> 8) + t) >> 8; } G_STMT_END

			while (p < end) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
				MULT(q[0], p[2], p[3], t1);
				MULT(q[1], p[1], p[3], t2);
				MULT(q[2], p[0], p[3], t3);
				q[3] = p[3];
#else	  
				q[0] = p[3];
				MULT(q[1], p[0], p[3], t1);
				MULT(q[2], p[1], p[3], t2);
				MULT(q[3], p[2], p[3], t3);
#endif

				p += 4;
				q += 4;
			}

#undef MULT
		}

		gdk_pixels += gdk_rowstride;
		cairo_pixels += 4 * width;
	}

	return surface;
}

GdkPixbuf *
f_pixbuf_from_cairo_surface(cairo_surface_t *source) {
	gint width = cairo_image_surface_get_width(source);
	gint height = cairo_image_surface_get_height(source);
	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);

	guchar *gdk_pixels = gdk_pixbuf_get_pixels(pixbuf);
	int gdk_rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	cairo_format_t format;
	cairo_surface_t *surface;
	cairo_t *ctx;
	static const cairo_user_data_key_t key = { 0 };
	int j;

	format = f_image_surface_get_format(source);
	surface = cairo_image_surface_create_for_data(gdk_pixels, format, width, height, gdk_rowstride);
	ctx = cairo_create(surface);
	cairo_set_source_surface(ctx, source, 0, 0);
	if (format == CAIRO_FORMAT_ARGB32)
		cairo_mask_surface(ctx, source, 0, 0);
	else
		cairo_paint(ctx);

	for (j = height; j; j--) {
		guchar *p = gdk_pixels;
		guchar *end = p + 4 * width;
		guchar tmp;

		while (p < end) {
			tmp = p[0];
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			p[0] = p[2];
			p[2] = tmp;
#else	  
			p[0] = p[1];
			p[1] = p[2];
			p[2] = p[3];
			p[3] = tmp;
#endif
			p += 4;
		}

		gdk_pixels += gdk_rowstride;
	}

	cairo_destroy(ctx);
	cairo_surface_destroy(surface);
	return pixbuf;
}
