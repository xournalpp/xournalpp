/*
 * Xournal++
 *
 * Pixbuf utils
 *
 * Copied from F-Spot, part copied from GTK3
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

#include <pixbuf-utils.h>

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



/**
 * Source GTK 3
 */

static cairo_format_t
gdk_cairo_format_for_content (cairo_content_t content)
{
  switch (content)
    {
    case CAIRO_CONTENT_COLOR:
      return CAIRO_FORMAT_RGB24;
    case CAIRO_CONTENT_ALPHA:
      return CAIRO_FORMAT_A8;
    case CAIRO_CONTENT_COLOR_ALPHA:
    default:
      return CAIRO_FORMAT_ARGB32;
    }
}

static cairo_surface_t *
gdk_cairo_surface_coerce_to_image (cairo_surface_t *surface,
                                   cairo_content_t  content,
                                   int              src_x,
                                   int              src_y,
                                   int              width,
                                   int              height)
{
  cairo_surface_t *copy;
  cairo_t *cr;

  copy = cairo_image_surface_create (gdk_cairo_format_for_content (content),
                                     width,
                                     height);

  cr = cairo_create (copy);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface (cr, surface, -src_x, -src_y);
  cairo_paint (cr);
  cairo_destroy (cr);

  return copy;
}

static void
convert_alpha (guchar *dest_data,
               int     dest_stride,
               guchar *src_data,
               int     src_stride,
               int     src_x,
               int     src_y,
               int     width,
               int     height)
{
  int x, y;

  src_data += src_stride * src_y + src_x * 4;

  for (y = 0; y < height; y++) {
    guint32 *src = (guint32 *) src_data;

    for (x = 0; x < width; x++) {
      guint alpha = src[x] >> 24;

      if (alpha == 0)
        {
          dest_data[x * 4 + 0] = 0;
          dest_data[x * 4 + 1] = 0;
          dest_data[x * 4 + 2] = 0;
        }
      else
        {
          dest_data[x * 4 + 0] = (((src[x] & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
          dest_data[x * 4 + 1] = (((src[x] & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
          dest_data[x * 4 + 2] = (((src[x] & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
        }
      dest_data[x * 4 + 3] = alpha;
    }

    src_data += src_stride;
    dest_data += dest_stride;
  }
}

static void
convert_no_alpha (guchar *dest_data,
                  int     dest_stride,
                  guchar *src_data,
                  int     src_stride,
                  int     src_x,
                  int     src_y,
                  int     width,
                  int     height)
{
  int x, y;

  src_data += src_stride * src_y + src_x * 4;

  for (y = 0; y < height; y++) {
    guint32 *src = (guint32 *) src_data;

    for (x = 0; x < width; x++) {
      dest_data[x * 3 + 0] = src[x] >> 16;
      dest_data[x * 3 + 1] = src[x] >>  8;
      dest_data[x * 3 + 2] = src[x];
    }

    src_data += src_stride;
    dest_data += dest_stride;
  }
}

/**
 * gdk_pixbuf_get_from_surface:
 * @surface: surface to copy from
 * @src_x: Source X coordinate within @surface
 * @src_y: Source Y coordinate within @surface
 * @width: Width in pixels of region to get
 * @height: Height in pixels of region to get
 *
 * Transfers image data from a #cairo_surface_t and converts it to an RGB(A)
 * representation inside a #GdkPixbuf. This allows you to efficiently read
 * individual pixels from cairo surfaces. For #GdkWindows, use
 * gdk_pixbuf_get_from_window() instead.
 *
 * This function will create an RGB pixbuf with 8 bits per channel.
 * The pixbuf will contain an alpha channel if the @surface contains one.
 *
 * Return value: (transfer full): A newly-created pixbuf with a reference
 *     count of 1, or %NULL on error
 */
GdkPixbuf *
xoj_pixbuf_get_from_surface  (cairo_surface_t *surface,
                              gint             src_x,
                              gint             src_y,
                              gint             width,
                              gint             height)
{
  cairo_content_t content;
  GdkPixbuf *dest;

  /* General sanity checks */
  g_return_val_if_fail (surface != NULL, NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);

  content = (cairo_content_t)(cairo_surface_get_content (surface) | CAIRO_CONTENT_COLOR);
  dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                         !!(content & CAIRO_CONTENT_ALPHA),
                         8,
                         width, height);

  surface = gdk_cairo_surface_coerce_to_image (surface, content,
                                               src_x, src_y,
                                               width, height);
  cairo_surface_flush (surface);
  if (cairo_surface_status (surface) || dest == NULL)
    {
      cairo_surface_destroy (surface);
      return NULL;
    }

  if (gdk_pixbuf_get_has_alpha (dest))
    convert_alpha (gdk_pixbuf_get_pixels (dest),
                   gdk_pixbuf_get_rowstride (dest),
                   cairo_image_surface_get_data (surface),
                   cairo_image_surface_get_stride (surface),
                   0, 0,
                   width, height);
  else
    convert_no_alpha (gdk_pixbuf_get_pixels (dest),
                      gdk_pixbuf_get_rowstride (dest),
                      cairo_image_surface_get_data (surface),
                      cairo_image_surface_get_stride (surface),
                      0, 0,
                      width, height);

  cairo_surface_destroy (surface);
  return dest;
}

