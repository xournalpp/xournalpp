/* poppler-page.cc: glib wrapper for poppler
 * Copyright (C) 2005, Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include <math.h>

#include <goo/GooList.h>
#include <splash/SplashBitmap.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>
#include <PageTransition.h>

#include "poppler.h"
#include "poppler-private.h"

enum
{
  PROP_0,
  PROP_LABEL
};

typedef struct _PopplerPageClass PopplerPageClass;
struct _PopplerPageClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerPage, poppler_page, G_TYPE_OBJECT)

PopplerPage *
_poppler_page_new (PopplerDocument *document, Page *page, int index)
{
  PopplerPage *poppler_page;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

  poppler_page = (PopplerPage *) g_object_new (POPPLER_TYPE_PAGE, NULL, NULL);
  poppler_page->document = (PopplerDocument *) g_object_ref (document);
  poppler_page->page = page;
  poppler_page->index = index;

  return poppler_page;
}

static void
poppler_page_finalize (GObject *object)
{
  PopplerPage *page = POPPLER_PAGE (object);

  g_object_unref (page->document);
  page->document = NULL;

  if (page->annots != NULL)
    delete page->annots;
#if defined (HAVE_CAIRO)
  if (page->text != NULL) 
    page->text->decRefCnt();
#else
  if (page->gfx != NULL)
    delete page->gfx;  
  if (page->text_dev != NULL)
    delete page->text_dev;
#endif
  /* page->page is owned by the document */
}

/**
 * poppler_page_get_size:
 * @page: A #PopplerPage
 * @width: return location for the width of @page
 * @height: return location for the height of @page
 * 
 * Gets the size of @page at the current scale and rotation.
 **/
void
poppler_page_get_size (PopplerPage *page,
		       double      *width,
		       double      *height)
{
  double page_width, page_height;
  int rotate;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  rotate = page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    page_height = page->page->getCropWidth ();
    page_width = page->page->getCropHeight ();
  } else {
    page_width = page->page->getCropWidth ();
    page_height = page->page->getCropHeight ();
  }

  if (width != NULL)
    *width = page_width;
  if (height != NULL)
    *height = page_height;
}

/**
 * poppler_page_get_index:
 * @page: a #PopplerPage
 * 
 * Returns the index of @page
 * 
 * Return value: index value of @page
 **/
int
poppler_page_get_index (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), 0);

  return page->index;
}

/**
 * poppler_page_get_duration:
 * @page: a #PopplerPage
 *
 * Returns the duration of @page
 *
 * Return value: duration in seconds of @page or -1.
 **/
double
poppler_page_get_duration (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), -1);

  return page->page->getDuration ();
}

/**
 * poppler_page_get_transition:
 * @page: a #PopplerPage
 *
 * Returns the transition effect of @page
 *
 * Return value: a #PopplerPageTransition or NULL.
 **/
PopplerPageTransition *
poppler_page_get_transition (PopplerPage *page)
{
  PageTransition *trans;
  PopplerPageTransition *transition;
  Object obj;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  trans = new PageTransition (page->page->getTrans (&obj));
  obj.free ();

  if (!trans->isOk ()) {
    delete trans;
    return NULL;
  }

  transition = poppler_page_transition_new ();

  switch (trans->getType ())
    {
      case transitionReplace:
        transition->type = POPPLER_PAGE_TRANSITION_REPLACE;
	break;
      case transitionSplit:
	transition->type = POPPLER_PAGE_TRANSITION_SPLIT;
	break;
      case transitionBlinds:
        transition->type = POPPLER_PAGE_TRANSITION_BLINDS;
	break;
      case transitionBox:
        transition->type = POPPLER_PAGE_TRANSITION_BOX;
	break;
      case transitionWipe:
        transition->type = POPPLER_PAGE_TRANSITION_WIPE;
	break;
      case transitionDissolve:
        transition->type = POPPLER_PAGE_TRANSITION_DISSOLVE;
	break;
      case transitionGlitter:
        transition->type = POPPLER_PAGE_TRANSITION_GLITTER;
	break;
      case transitionFly:
        transition->type = POPPLER_PAGE_TRANSITION_FLY;
	break;
      case transitionPush:
        transition->type = POPPLER_PAGE_TRANSITION_PUSH;
	break;
      case transitionCover:
        transition->type = POPPLER_PAGE_TRANSITION_COVER;
	break;
      case transitionUncover:
        transition->type = POPPLER_PAGE_TRANSITION_UNCOVER;
        break;
      case transitionFade:
        transition->type = POPPLER_PAGE_TRANSITION_FADE;
	break;
      default:
        g_assert_not_reached ();
    }

  transition->alignment = (trans->getAlignment() == transitionHorizontal) ?
	  POPPLER_PAGE_TRANSITION_HORIZONTAL :
	  POPPLER_PAGE_TRANSITION_VERTICAL;

  transition->direction = (trans->getDirection() == transitionInward) ?
	  POPPLER_PAGE_TRANSITION_INWARD :
	  POPPLER_PAGE_TRANSITION_OUTWARD;

  transition->duration = trans->getDuration();
  transition->angle = trans->getAngle();
  transition->scale = trans->getScale();
  transition->rectangular = trans->isRectangular();
  
  delete trans;
  
  return transition;
}

#if !defined (HAVE_CAIRO)
static TextOutputDev *
poppler_page_get_text_output_dev (PopplerPage *page)
{
  if (page->text_dev == NULL) {
    page->text_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);

    if (page->gfx)
      delete page->gfx;
    page->gfx = page->page->createGfx(page->text_dev,
				      72.0, 72.0, 0,
				      gFalse, /* useMediaBox */
				      gTrue, /* Crop */
				      -1, -1, -1, -1,
				      gFalse, /* printing */
				      page->document->doc->getCatalog (),
				      NULL, NULL, NULL, NULL);

    page->page->display(page->gfx);

    page->text_dev->endPage();
  }

  return page->text_dev;
}
#endif /* !defined (HAVE_CAIRO) */

#if defined (HAVE_CAIRO)

static TextPage *
poppler_page_get_text_page (PopplerPage *page)
{
  if (page->text == NULL) {
    cairo_t *cr;
    cairo_surface_t *surface;

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 1, 1);
    cr = cairo_create (surface);
    poppler_page_render (page, cr);
    cairo_destroy (cr);
    cairo_surface_destroy (surface);

  }

  return page->text;
}

#ifdef POPPLER_WITH_GDK
typedef struct {
  unsigned char *cairo_data;
  cairo_surface_t *surface;
  cairo_t *cairo;
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 int rotation,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  CairoOutputDev *output_dev;
  cairo_surface_t *surface;
  double width, height;
  int cairo_width, cairo_height, cairo_rowstride, rotate;
  unsigned char *cairo_data;

  rotate = rotation + page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    height = page->page->getCropWidth ();
    width = page->page->getCropHeight ();
  } else {
    width = page->page->getCropWidth ();
    height = page->page->getCropHeight ();
  }

  cairo_width = (int) ceil(width * scale);
  cairo_height = (int) ceil(height * scale);

  output_dev = page->document->output_dev;
  cairo_rowstride = cairo_width * 4;
  cairo_data = (guchar *) gmallocn (cairo_height, cairo_rowstride);
  if (transparent)
      memset (cairo_data, 0x00, cairo_height * cairo_rowstride);
  else
      memset (cairo_data, 0xff, cairo_height * cairo_rowstride);

  surface = cairo_image_surface_create_for_data(cairo_data,
						CAIRO_FORMAT_ARGB32,
	  					cairo_width, cairo_height, 
						cairo_rowstride);

  output_dev_data->cairo_data = cairo_data;
  output_dev_data->surface = surface;
  output_dev_data->cairo = cairo_create (surface);
  output_dev->setCairo (output_dev_data->cairo);
}

static void
copy_cairo_surface_to_pixbuf (cairo_surface_t *surface,
			      unsigned char   *data,
			      GdkPixbuf       *pixbuf)
{
  int cairo_width, cairo_height, cairo_rowstride;
  unsigned char *pixbuf_data, *dst, *cairo_data;
  int pixbuf_rowstride, pixbuf_n_channels;
  unsigned int *src;
  int x, y;

  cairo_width = cairo_image_surface_get_width (surface);
  cairo_height = cairo_image_surface_get_height (surface);
  cairo_rowstride = cairo_width * 4;
  cairo_data = data;

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (cairo_width > gdk_pixbuf_get_width (pixbuf))
    cairo_width = gdk_pixbuf_get_width (pixbuf);
  if (cairo_height > gdk_pixbuf_get_height (pixbuf))
    cairo_height = gdk_pixbuf_get_height (pixbuf);
  for (y = 0; y < cairo_height; y++)
    {
      src = (unsigned int *) (cairo_data + y * cairo_rowstride);
      dst = pixbuf_data + y * pixbuf_rowstride;
      for (x = 0; x < cairo_width; x++) 
	{
	  dst[0] = (*src >> 16) & 0xff;
	  dst[1] = (*src >> 8) & 0xff; 
	  dst[2] = (*src >> 0) & 0xff;
	  if (pixbuf_n_channels == 4)
	      dst[3] = (*src >> 24) & 0xff;
	  dst += pixbuf_n_channels;
	  src++;
	}
    }
}	

static void
poppler_page_copy_to_pixbuf (PopplerPage *page,
			     GdkPixbuf *pixbuf,
			     OutputDevData *output_dev_data)
{
  copy_cairo_surface_to_pixbuf (output_dev_data->surface,
				output_dev_data->cairo_data,
				pixbuf);
  
  page->document->output_dev->setCairo (NULL);
  cairo_surface_destroy (output_dev_data->surface);
  cairo_destroy (output_dev_data->cairo);
  gfree (output_dev_data->cairo_data);
}

static void
poppler_page_set_selection_alpha (PopplerPage           *page,
				  double                 scale,
				  GdkPixbuf             *pixbuf,
				  PopplerSelectionStyle  style,
				  PopplerRectangle      *selection)
{
  /* Cairo doesn't need this, since cairo generates an alpha channel. */ 
}
#endif /* POPPLER_WITH_GDK */

#elif defined (HAVE_SPLASH) && defined (POPPLER_WITH_GDK)
 
typedef struct {
} OutputDevData;

static void
poppler_page_prepare_output_dev (PopplerPage *page,
				 double scale,
				 int rotation,
				 gboolean transparent,
				 OutputDevData *output_dev_data)
{
  /* pft */
}

static void
poppler_page_copy_to_pixbuf(PopplerPage *page,
			    GdkPixbuf *pixbuf,
			    OutputDevData *data)
{
  SplashOutputDev *output_dev;
  SplashBitmap *bitmap;
  SplashColorPtr color_ptr;
  int splash_width, splash_height, splash_rowstride;
  int pixbuf_rowstride, pixbuf_n_channels;
  guchar *pixbuf_data, *dst;
  int x, y;

  output_dev = page->document->output_dev;

  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();

  splash_width = bitmap->getWidth ();
  splash_height = bitmap->getHeight ();
  splash_rowstride = bitmap->getRowSize ();

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (splash_width > gdk_pixbuf_get_width (pixbuf))
    splash_width = gdk_pixbuf_get_width (pixbuf);
  if (splash_height > gdk_pixbuf_get_height (pixbuf))
    splash_height = gdk_pixbuf_get_height (pixbuf);

  SplashColorPtr pixel = new Guchar[4];
  for (y = 0; y < splash_height; y++)
  {
    dst = pixbuf_data + y * pixbuf_rowstride;
    for (x = 0; x < splash_width; x++)
    {
      output_dev->getBitmap()->getPixel(x, y, pixel);
      dst[0] = pixel[0];
      dst[1] = pixel[1];
      dst[2] = pixel[2];
      if (pixbuf_n_channels == 4)
        dst[3] = 0xff;
      dst += pixbuf_n_channels;
    }
  }
  delete [] pixel;
}

static void
poppler_page_set_selection_alpha (PopplerPage           *page,
				  double                 scale,
				  GdkPixbuf             *pixbuf,
				  PopplerSelectionStyle  style,
				  PopplerRectangle      *selection)
{
  GList *region, *l;
  gint x, y, width, height;
  int pixbuf_rowstride, pixbuf_n_channels;
  guchar *pixbuf_data, *dst;

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  if (pixbuf_n_channels != 4)
    return;

  for (y = 0; y < height; y++) {
      dst = pixbuf_data + y * pixbuf_rowstride;
      for (x = 0; x < width; x++) {
	  dst[3] = 0x00;
	  dst += pixbuf_n_channels;
      }
  }

  region = poppler_page_get_selection_region (page, scale, style, selection);

  for (l = region; l; l = g_list_next (l)) {
    PopplerRectangle *rectangle = (PopplerRectangle *)l->data;
    GdkRectangle rect;
    
    rect.x = (gint)rectangle->x1;
    rect.y = (gint)rectangle->y1;
    rect.width  = (gint) (rectangle->x2 - rectangle->x1);
    rect.height = (gint) (rectangle->y2 - rectangle->y1);
    
    for (y = 0; y < rect.height; y++) {
      dst = pixbuf_data + (rect.y + y) * pixbuf_rowstride +
	rect.x * pixbuf_n_channels;
      for (x = 0; x < rect.width; x++) {
	  dst[3] = 0xff;
	  dst += pixbuf_n_channels;
      }
    }
  }

  poppler_page_selection_region_free (region);
}

#endif /* HAVE_SPLASH */

static GBool
poppler_print_annot_cb (Annot *annot, void *user_data)
{
  if (annot->getFlags () & Annot::flagPrint)
    return gTrue;
  return (annot->getType() == Annot::typeWidget);
}

#if defined (HAVE_CAIRO)

static void
_poppler_page_render (PopplerPage *page,
		      cairo_t *cairo,
		      GBool printing)
{
  CairoOutputDev *output_dev;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  output_dev = page->document->output_dev;
  output_dev->setCairo (cairo);
  output_dev->setPrinting (printing);

  if (!printing)
    output_dev->setTextPage (page->text);

  /* NOTE: instead of passing -1 we should/could use cairo_clip_extents()
   * to get a bounding box */
  cairo_save (cairo);
  page->page->displaySlice(output_dev,
			   72.0, 72.0, 0,
			   gFalse, /* useMediaBox */
			   gTrue, /* Crop */
			   -1, -1,
			   -1, -1,
			   printing,
			   page->document->doc->getCatalog (),
			   NULL, NULL,
			   printing ? poppler_print_annot_cb : NULL, NULL);
  cairo_restore (cairo);

  output_dev->setCairo (NULL);
  output_dev->setTextPage (NULL);
}

/**
 * poppler_page_render:
 * @page: the page to render from
 * @cairo: cairo context to render to
 *
 * Render the page to the given cairo context. This function
 * is for rendering a page that will be displayed. If you want
 * to render a page that will be printed use
 * poppler_page_render_for_printing() instead
 **/
void
poppler_page_render (PopplerPage *page,
		     cairo_t *cairo)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));

  if (!page->text)
    page->text = new TextPage(gFalse);

  _poppler_page_render (page, cairo, gFalse);
}

/**
 * poppler_page_render_for_printing:
 * @page: the page to render from
 * @cairo: cairo context to render to
 *
 * Render the page to the given cairo context for printing.
 **/
void
poppler_page_render_for_printing (PopplerPage *page,
				  cairo_t *cairo)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  
  _poppler_page_render (page, cairo, gTrue);	
}

static cairo_surface_t *
create_surface_from_thumbnail_data (guchar *data,
				    gint    width,
				    gint    height,
				    gint    rowstride)
{
  guchar *cairo_pixels;
  gint cairo_stride;
  cairo_surface_t *surface;
  int j;

  surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
  if (cairo_surface_status (surface))
    return NULL;

  cairo_pixels = cairo_image_surface_get_data (surface);
  cairo_stride = cairo_image_surface_get_stride (surface);

  for (j = height; j; j--) {
    guchar *p = data;
    guchar *q = cairo_pixels;
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

    data += rowstride;
    cairo_pixels += cairo_stride;
  }

  return surface;
}
				    

/**
 * poppler_page_get_thumbnail:
 * @page: the #PopperPage to get the thumbnail for
 * 
 * Get the embedded thumbnail for the specified page.  If the document
 * doesn't have an embedded thumbnail for the page, this function
 * returns %NULL.
 * 
 * Return value: the tumbnail as a cairo_surface_t or %NULL if the document
 * doesn't have a thumbnail for this page.
 **/
cairo_surface_t *
poppler_page_get_thumbnail (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;
  cairo_surface_t *surface;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);

  if (!page->page->loadThumb (&data, &width, &height, &rowstride))
    return NULL;

  surface = create_surface_from_thumbnail_data (data, width, height, rowstride);
  gfree (data);
  
  return surface;
}

/**
 * poppler_page_render_selection:
 * @page: the #PopplerPage for which to render selection
 * @cairo: cairo context to render to
 * @selection: start and end point of selection as a rectangle
 * @old_selection: previous selection
 * @style: a #PopplerSelectionStyle
 * @glyph_color: color to use for drawing glyphs
 * @background_color: color to use for the selection background
 *
 * Render the selection specified by @selection for @page to
 * the given cairo context.  The selection will be rendered, using
 * @glyph_color for the glyphs and @background_color for the selection
 * background.
 *
 * If non-NULL, @old_selection specifies the selection that is already
 * rendered to @cairo, in which case this function will (some day)
 * only render the changed part of the selection.
 **/
void
poppler_page_render_selection (PopplerPage           *page,
			       cairo_t               *cairo,
			       PopplerRectangle      *selection,
			       PopplerRectangle      *old_selection,
			       PopplerSelectionStyle  style, 
			       PopplerColor          *glyph_color,
			       PopplerColor          *background_color)
{
  CairoOutputDev *output_dev;
  TextPage *text;
  SelectionStyle selection_style = selectionStyleGlyph;
  PDFRectangle pdf_selection(selection->x1, selection->y1,
			     selection->x2, selection->y2);

  GfxColor gfx_background_color = {
      {
	  background_color->red,
	  background_color->green,
	  background_color->blue
      }
  };
  GfxColor gfx_glyph_color = {
      {
	  glyph_color->red,
	  glyph_color->green,
	  glyph_color->blue
      }
  };

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  output_dev = page->document->output_dev;
  output_dev->setCairo (cairo);

  text = poppler_page_get_text_page (page);
  text->drawSelection (output_dev, 1.0, 0,
		       &pdf_selection, selection_style,
		       &gfx_glyph_color, &gfx_background_color);

  output_dev->setCairo (NULL);
}

#endif /* HAVE_CAIRO */

#ifdef POPPLER_WITH_GDK
static void
_poppler_page_render_to_pixbuf (PopplerPage *page,
				int src_x, int src_y,
				int src_width, int src_height,
				double scale,
				int rotation,
				GBool printing,
				GdkPixbuf *pixbuf)
{
  OutputDevData data;
  
  poppler_page_prepare_output_dev (page, scale, rotation, FALSE, &data);

  page->page->displaySlice(page->document->output_dev,
			   72.0 * scale, 72.0 * scale,
			   rotation,
			   gFalse, /* useMediaBox */
			   gTrue, /* Crop */
			   src_x, src_y,
			   src_width, src_height,
			   printing,
			   page->document->doc->getCatalog (),
			   NULL, NULL,
			   printing ? poppler_print_annot_cb : NULL, NULL);
  
  poppler_page_copy_to_pixbuf (page, pixbuf, &data);
}

/**
 * poppler_page_render_to_pixbuf:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner  
 * @src_y: y coordinate of upper left corner  
 * @src_width: width of rectangle to render  
 * @src_height: height of rectangle to render
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render into
 *
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.
 * This function is for rendering a page that will be displayed.
 * If you want to render a page that will be printed use
 * poppler_page_render_to_pixbuf_for_printing() instead
 **/
void
poppler_page_render_to_pixbuf (PopplerPage *page,
			       int src_x, int src_y,
			       int src_width, int src_height,
			       double scale,
			       int rotation,
			       GdkPixbuf *pixbuf)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  _poppler_page_render_to_pixbuf (page, src_x, src_y,
				  src_width, src_height,
				  scale, rotation,
				  gFalse,
				  pixbuf);
}

/**
 * poppler_page_render_to_pixbuf_for_printing:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner  
 * @src_y: y coordinate of upper left corner  
 * @src_width: width of rectangle to render  
 * @src_height: height of rectangle to render
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render into
 *
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.
 * This function is for rendering a page that will be printed.
 **/
void
poppler_page_render_to_pixbuf_for_printing (PopplerPage *page,
					    int src_x, int src_y,
					    int src_width, int src_height,
					    double scale,
					    int rotation,
					    GdkPixbuf *pixbuf)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  _poppler_page_render_to_pixbuf (page, src_x, src_y,
				  src_width, src_height,
				  scale, rotation,
				  gTrue,
				  pixbuf);
}

/**
 * poppler_page_get_thumbnail_pixbuf:
 * @page: the #PopperPage to get the thumbnail for
 * 
 * Get the embedded thumbnail for the specified page.  If the document
 * doesn't have an embedded thumbnail for the page, this function
 * returns %NULL.
 * 
 * Return value: the tumbnail as a #GdkPixbuf or %NULL if the document
 * doesn't have a thumbnail for this page.
 **/
GdkPixbuf *
poppler_page_get_thumbnail_pixbuf (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);

  if (!page->page->loadThumb (&data, &width, &height, &rowstride))
    return NULL;

  return gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB,
				   FALSE, 8, width, height, rowstride,
				   (GdkPixbufDestroyNotify)gfree, NULL);
}

/**
 * poppler_page_render_selection_to_pixbuf:
 * @page: the #PopplerPage for which to render selection
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render to
 * @selection: start and end point of selection as a rectangle
 * @old_selection: previous selection
 * @style: a #PopplerSelectionStyle
 * @glyph_color: color to use for drawing glyphs
 * @background_color: color to use for the selection background
 * 
 * Render the selection specified by @selection for @page into
 * @pixbuf.  The selection will be rendered at @scale, using
 * @glyph_color for the glyphs and @background_color for the selection
 * background.
 *
 * If non-NULL, @old_selection specifies the selection that is already
 * rendered in @pixbuf, in which case this function will (some day)
 * only render the changed part of the selection.
 **/
void
poppler_page_render_selection_to_pixbuf (PopplerPage           *page,
                                         gdouble                scale,
                                         int                    rotation,
                                         GdkPixbuf             *pixbuf,
                                         PopplerRectangle      *selection,
                                         PopplerRectangle      *old_selection,
					 PopplerSelectionStyle  style,
                                         GdkColor              *glyph_color,
                                         GdkColor              *background_color)
{
  OutputDev *output_dev;
  OutputDevData data;
  SelectionStyle selection_style = selectionStyleGlyph;
  PDFRectangle pdf_selection(selection->x1, selection->y1,
			     selection->x2, selection->y2);

  GfxColor gfx_background_color = { 
      {
	  background_color->red,
	  background_color->green,
	  background_color->blue
      }
  };
  GfxColor gfx_glyph_color = {
      {
	  glyph_color->red,
	  glyph_color->green,
	  glyph_color->blue
      }
  };

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  output_dev = page->document->output_dev;

  poppler_page_prepare_output_dev (page, scale, rotation, TRUE, &data);

#if defined (HAVE_CAIRO)
  TextPage *text;

  text = poppler_page_get_text_page (page);
  text->drawSelection (output_dev, scale, rotation,
			   &pdf_selection, selection_style,
			   &gfx_glyph_color, &gfx_background_color);
#else
  TextOutputDev *text_dev;

  text_dev = poppler_page_get_text_output_dev (page);
  text_dev->drawSelection (output_dev, scale, rotation,
			   &pdf_selection, selection_style,
			   &gfx_glyph_color, &gfx_background_color);
  /* We'll need a function to destroy page->text_dev and page->gfx
   * when the application wants to get rid of them.
   *
   * Two improvements: 1) make GfxFont refcounted and let TextPage and
   * friends hold a reference to the GfxFonts they need so we can free
   * up Gfx early.  2) use a TextPage directly when rendering the page
   * so we don't have to use TextOutputDev and render a second
   * time. */
#endif
  
  poppler_page_copy_to_pixbuf (page, pixbuf, &data);

  poppler_page_set_selection_alpha (page, scale, pixbuf, style, selection);
}

#endif /* POPPLER_WITH_GDK */

/**
 * poppler_page_get_thumbnail_size:
 * @page: A #PopplerPage
 * @width: return location for width
 * @height: return location for height
 *
 * Returns %TRUE if @page has a thumbnail associated with it.  It also
 * fills in @width and @height with the width and height of the
 * thumbnail.  The values of width and height are not changed if no
 * appropriate thumbnail exists.
 *
 * Return value: %TRUE, if @page has a thumbnail associated with it.
 **/
gboolean
poppler_page_get_thumbnail_size (PopplerPage *page,
				 int         *width,
				 int         *height)
{
  Object thumb;
  Dict *dict;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (width != NULL, FALSE);
  g_return_val_if_fail (height != NULL, FALSE);

  page->page->getThumb (&thumb);
  if (!thumb.isStream ())
    {
      thumb.free ();
      return FALSE;
    }

  dict = thumb.streamGetDict();

  /* Theoretically, this could succeed and you would still fail when
   * loading the thumb */
  if (dict->lookupInt ("Width", "W", width)  &&
      dict->lookupInt ("Height", "H", height))
    retval = TRUE;

  thumb.free ();

  return retval;
}

/**
 * poppler_page_get_selection_region:
 * @page: a #PopplerPage
 * @scale: scale specified as pixels per point
 * @style: a #PopplerSelectionStyle
 * @selection: start and end point of selection as a rectangle
 * 
 * Returns a region containing the area that would be rendered by
 * poppler_page_render_selection() or 
 * poppler_page_render_selection_to_pixbuf() as a #GList of
 * #PopplerRectangle. The returned list must be freed with
 * poppler_page_selection_region_free().
 * 
 * Return value: a #GList of #PopplerRectangle
 **/
GList *
poppler_page_get_selection_region (PopplerPage           *page,
				   gdouble                scale,
				   PopplerSelectionStyle  style,
				   PopplerRectangle      *selection)
{
  PDFRectangle poppler_selection;
  SelectionStyle selection_style = selectionStyleGlyph;
  GooList *list;
  GList *region = NULL;
  int i;

  poppler_selection.x1 = selection->x1;
  poppler_selection.y1 = selection->y1;
  poppler_selection.x2 = selection->x2;
  poppler_selection.y2 = selection->y2;

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

#if defined (HAVE_CAIRO)
  TextPage *text;

  text = poppler_page_get_text_page (page);
  list = text->getSelectionRegion(&poppler_selection,
				  selection_style, scale);
#else
  TextOutputDev *text_dev;
  
  text_dev = poppler_page_get_text_output_dev (page);
  list = text_dev->getSelectionRegion(&poppler_selection,
				      selection_style, scale);
#endif
  
  for (i = 0; i < list->getLength(); i++) {
    PDFRectangle *selection_rect = (PDFRectangle *) list->get(i);
    PopplerRectangle *rect;

    rect = poppler_rectangle_new ();
    
    rect->x1 = selection_rect->x1;
    rect->y1 = selection_rect->y1;
    rect->x2 = selection_rect->x2;
    rect->y2 = selection_rect->y2;
    
    region = g_list_prepend (region, rect);
    
    delete selection_rect;
  }

  delete list;

  return g_list_reverse (region);
}

void
poppler_page_selection_region_free (GList *region)
{
  if (!region)
    return;

  g_list_foreach (region, (GFunc)poppler_rectangle_free, NULL);
  g_list_free (region);
}

/**
 * poppler_page_get_text:
 * @page: a #PopplerPage
 * @style: a #PopplerSelectionStyle
 * @selection: the #PopplerRectangle including the text
 * 
 * Retrieves the contents of the specified @selection as text.
 * 
 * Return value: a pointer to the contents of the @selection
 *               as a string
 **/
char *
poppler_page_get_text (PopplerPage          *page,
		       PopplerSelectionStyle style,
		       PopplerRectangle     *selection)
{
  GooString *sel_text;
  double height;
  char *result;
  SelectionStyle selection_style = selectionStyleGlyph;
  PDFRectangle pdf_selection;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (selection != NULL, NULL);

  poppler_page_get_size (page, NULL, &height);
  pdf_selection.x1 = selection->x1;
  pdf_selection.y1 = height - selection->y2;
  pdf_selection.x2 = selection->x2;
  pdf_selection.y2 = height - selection->y1;

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

#if defined (HAVE_CAIRO)
  TextPage *text;

  text = poppler_page_get_text_page (page);
  sel_text = text->getSelectionText (&pdf_selection, selection_style);
#else
  TextOutputDev *text_dev;

  text_dev = poppler_page_get_text_output_dev (page);
  sel_text = text_dev->getSelectionText (&pdf_selection, selection_style);
#endif
	  
  result = g_strdup (sel_text->getCString ());
  delete sel_text;

  return result;
}

/**
 * poppler_page_find_text:
 * @page: a #PopplerPage
 * @text: the text to search for (UTF-8 encoded)
 * 
 * A #GList of rectangles for each occurance of the text on the page.
 * The coordinates are in PDF points.
 * 
 * Return value: a #GList of PopplerRectangle, 
 **/
GList *
poppler_page_find_text (PopplerPage *page,
			const char  *text)
{
  PopplerRectangle *match;
  GList *matches;
  double xMin, yMin, xMax, yMax;
  gunichar *ucs4;
  glong ucs4_len;
  double height;
#if defined (HAVE_CAIRO)
  TextPage *text_dev;
#else
  TextOutputDev *text_dev;
#endif
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (text != NULL, FALSE);

#if defined (HAVE_CAIRO)
  text_dev = poppler_page_get_text_page (page);
#else
  text_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);
  page->page->display (text_dev, 72, 72, 0,
		       gFalse, gTrue, gFalse,
		       page->document->doc->getCatalog());
#endif

  ucs4 = g_utf8_to_ucs4_fast (text, -1, &ucs4_len);
  poppler_page_get_size (page, NULL, &height);
  
  matches = NULL;
  xMin = 0;
  yMin = 0;

  while (text_dev->findText (ucs4, ucs4_len,
			     gFalse, gTrue, // startAtTop, stopAtBottom
			     gTrue, gFalse, // startAtLast, stopAtLast
			     gFalse, gFalse, // caseSensitive, backwards
			     &xMin, &yMin, &xMax, &yMax))
    {
      match = g_new (PopplerRectangle, 1);
      match->x1 = xMin;
      match->y1 = height - yMax;
      match->x2 = xMax;
      match->y2 = height - yMin;
      matches = g_list_prepend (matches, match);
    }

#if !defined (HAVE_CAIRO)
  delete text_dev;
#endif
  
  g_free (ucs4);

  return g_list_reverse (matches);
}

#if defined (HAVE_CAIRO)

static CairoImageOutputDev *
poppler_page_get_image_output_dev (PopplerPage *page,
				   GBool (*imgDrawDeviceCbk)(int img_id, void *data),
				   void *imgDrawCbkData)
{
  CairoImageOutputDev *image_dev;
  Gfx *gfx;
  
  image_dev = new CairoImageOutputDev ();

  if (imgDrawDeviceCbk) {
    image_dev->setImageDrawDecideCbk (imgDrawDeviceCbk,
				      imgDrawCbkData);
  }

  gfx = page->page->createGfx(image_dev,
			      72.0, 72.0, 0,
			      gFalse, /* useMediaBox */
			      gTrue, /* Crop */
			      -1, -1, -1, -1,
			      gFalse, /* printing */
			      page->document->doc->getCatalog (),
			      NULL, NULL, NULL, NULL);
  page->page->display(gfx);
  delete gfx;

  return image_dev;
}

/**
 * poppler_page_get_image_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerImageMapping items that map from a
 * location on @page to an image of the page. This list must be freed
 * with poppler_page_free_image_mapping() when done.
 *
 * Return value: A #GList of #PopplerImageMapping
 **/
GList *
poppler_page_get_image_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  CairoImageOutputDev *out;
  gint i;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  out = poppler_page_get_image_output_dev (page, NULL, NULL);

  for (i = 0; i < out->getNumImages (); i++) {
    PopplerImageMapping *mapping;
    CairoImage *image;

    image = out->getImage (i);

    /* Create the mapping */
    mapping = g_new (PopplerImageMapping, 1);

    image->getRect (&(mapping->area.x1), &(mapping->area.y1),
		    &(mapping->area.x2), &(mapping->area.y2));
    mapping->image_id = i;
    
    mapping->area.x1 -= page->page->getCropBox()->x1;
    mapping->area.x2 -= page->page->getCropBox()->x1;
    mapping->area.y1 -= page->page->getCropBox()->y1;
    mapping->area.y2 -= page->page->getCropBox()->y1;

    map_list = g_list_prepend (map_list, mapping);
  }

  delete out;

  return map_list;	
}

static GBool
image_draw_decide_cb (int image_id, void *data)
{
  return (image_id == GPOINTER_TO_INT (data));
}

/**
 * poppler_page_get_image:
 * @page: A #PopplerPage
 * @image_id: The image identificator
 *
 * Returns a cairo surface for the image of the @page
 *
 * Return value: A cairo surface for the image
 **/
cairo_surface_t *
poppler_page_get_image (PopplerPage *page,
			gint         image_id)
{
  CairoImageOutputDev *out;
  cairo_surface_t *image;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  out = poppler_page_get_image_output_dev (page,
					   image_draw_decide_cb,
					   GINT_TO_POINTER (image_id));

  if (image_id >= out->getNumImages ()) {
    delete out;
    
    return NULL;
  }

  image = out->getImage (image_id)->getImage ();
  if (!image) {
    delete out;

    return NULL;
  }

  cairo_surface_reference (image);
  delete out;
  
  return image;
}

/**
 * poppler_page_free_image_mapping:
 * @list: A list of #PopplerImageMapping<!-- -->s
 *
 * Frees a list of #PopplerImageMapping<!-- -->s allocated by
 * poppler_page_get_image_mapping().
 **/
void
poppler_page_free_image_mapping (GList *list)
{
  if (list == NULL)
    return;

  g_list_foreach (list, (GFunc)g_free, NULL);
  g_list_free (list);
}

#else /* HAVE_CAIRO */

GList *
poppler_page_get_image_mapping (PopplerPage *page)
{
  return NULL;
}

void
poppler_page_free_image_mapping (GList *list)
{
}

#endif /* HAVE_CAIRO */

/**
 * poppler_page_render_to_ps:
 * @page: a #PopplerPage
 * @ps_file: the PopplerPSFile to render to
 * 
 * Render the page on a postscript file
 * 
 **/
void
poppler_page_render_to_ps (PopplerPage   *page,
			   PopplerPSFile *ps_file)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (ps_file != NULL);

  if (!ps_file->out)
    ps_file->out = new PSOutputDev (ps_file->filename,
                                    ps_file->document->doc->getXRef(),
                                    ps_file->document->doc->getCatalog(),
                                    NULL,
                                    ps_file->first_page, ps_file->last_page,
                                    psModePS, (int)ps_file->paper_width,
                                    (int)ps_file->paper_height, ps_file->duplex,
                                    0, 0, 0, 0, gFalse, gFalse);


  ps_file->document->doc->displayPage (ps_file->out, page->index + 1, 72.0, 72.0,
				       0, gFalse, gTrue, gFalse);
}

static void
poppler_page_get_property (GObject *object,
			   guint prop_id,
			   GValue *value,
			   GParamSpec *pspec)
{
  PopplerPage *page = POPPLER_PAGE (object);
  GooString label;

  switch (prop_id)
    {
    case PROP_LABEL:
      page->document->doc->getCatalog ()->indexToLabel (page->index, &label);
      g_value_take_string (value, _poppler_goo_string_to_utf8(&label));
      break;
    }
}

static void
poppler_page_class_init (PopplerPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  gobject_class->finalize = poppler_page_finalize;
  gobject_class->get_property = poppler_page_get_property;

  pspec = g_param_spec_string ("label",
			       "Page Label",
			       "The label of the page",
			       NULL,
			       G_PARAM_READABLE);
  g_object_class_install_property (G_OBJECT_CLASS (klass),
				   PROP_LABEL,
				   pspec);
}

static void
poppler_page_init (PopplerPage *page)
{
}

/**
 * poppler_page_get_link_mapping:
 * @page: A #PopplerPage
 * 
 * Returns a list of #PopplerLinkMapping items that map from a
 * location on @page to a #PopplerAction.  This list must be freed
 * with poppler_page_free_link_mapping() when done.
 * 
 * Return value: A #GList of #PopplerLinkMapping
 **/
GList *
poppler_page_get_link_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  gint i;
  Links *links;
  Object obj;
  double width, height;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);
  
  links = new Links (page->page->getAnnots (&obj),
		     page->document->doc->getCatalog ()->getBaseURI ());
  obj.free ();
  
  if (links == NULL)
    return NULL;
  
  poppler_page_get_size (page, &width, &height);
  
  for (i = 0; i < links->getNumLinks (); i++)
    {
      PopplerLinkMapping *mapping;
      PopplerRectangle rect;
      LinkAction *link_action;
      Link *link;
      
      link = links->getLink (i);
      link_action = link->getAction ();
      
      /* Create the mapping */
      mapping = g_new (PopplerLinkMapping, 1);
      mapping->action = _poppler_action_new (page->document, link_action, NULL);
      
      link->getRect (&rect.x1, &rect.y1, &rect.x2, &rect.y2);
      
      switch (page->page->getRotate ())
        {
        case 90:
	  mapping->area.x1 = rect.y1;
	  mapping->area.y1 = height - rect.x2;
	  mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
	  mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
	  
	  break;
	case 180:
	  mapping->area.x1 = width - rect.x2;
	  mapping->area.y1 = height - rect.y2;
	  mapping->area.x2 = mapping->area.x1 + (rect.x2 - rect.x1);
	  mapping->area.y2 = mapping->area.y1 + (rect.y2 - rect.y1);
	  
	  break;
	case 270:
	  mapping->area.x1 = width - rect.y2;
	  mapping->area.y1 = rect.x1;
	  mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
	  mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
	  
	  break;
	default:
	  mapping->area.x1 = rect.x1;
	  mapping->area.y1 = rect.y1;
	  mapping->area.x2 = rect.x2;
	  mapping->area.y2 = rect.y2;
	}
			
      mapping->area.x1 -= page->page->getCropBox()->x1;
      mapping->area.x2 -= page->page->getCropBox()->x1;
      mapping->area.y1 -= page->page->getCropBox()->y1;
      mapping->area.y2 -= page->page->getCropBox()->y1;
      
      map_list = g_list_prepend (map_list, mapping);
    }
  
  delete links;
  
  return map_list;
}

static void
poppler_mapping_free (PopplerLinkMapping *mapping)
{
  poppler_action_free (mapping->action);
  g_free (mapping);
}

/**
 * poppler_page_free_link_mapping:
 * @list: A list of #PopplerLinkMapping<!-- -->s
 * 
 * Frees a list of #PopplerLinkMapping<!-- -->s allocated by
 * poppler_page_get_link_mapping().  It also frees the #PopplerAction<!-- -->s
 * that each mapping contains, so if you want to keep them around, you need to
 * copy them with poppler_action_copy().
 **/
void
poppler_page_free_link_mapping (GList *list)
{
  if (list == NULL)
    return;

  g_list_foreach (list, (GFunc) (poppler_mapping_free), NULL);
  g_list_free (list);
}

/**
 * poppler_page_get_form_field_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerFormFieldMapping items that map from a
 * location on @page to a form field.  This list must be freed
 * with poppler_page_free_form_field_mapping() when done.
 *
 * Return value: A #GList of #PopplerFormFieldMapping
 **/
GList *
poppler_page_get_form_field_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  FormPageWidgets *forms;
  gint i;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  forms = page->page->getPageWidgets ();
  if (forms == NULL)
    return NULL;
  
  for (i = 0; i < forms->getNumWidgets (); i++) {
    PopplerFormFieldMapping *mapping;
    FormWidget *field;

    mapping = poppler_form_field_mapping_new ();
    
    field = forms->getWidget (i);

    mapping->field = _poppler_form_field_new (page->document, field);
    field->getRect (&(mapping->area.x1), &(mapping->area.y1),
		    &(mapping->area.x2), &(mapping->area.y2));

    mapping->area.x1 -= page->page->getCropBox()->x1;
    mapping->area.x2 -= page->page->getCropBox()->x1;
    mapping->area.y1 -= page->page->getCropBox()->y1;
    mapping->area.y2 -= page->page->getCropBox()->y1;
    
    map_list = g_list_prepend (map_list, mapping);
  }
  
  return map_list;
}

/**
 * poppler_page_free_form_field_mapping:
 * @list: A list of #PopplerFormFieldMapping<!-- -->s
 *
 * Frees a list of #PopplerFormFieldMapping<!-- -->s allocated by
 * poppler_page_get_form_field_mapping().
 **/
void
poppler_page_free_form_field_mapping (GList *list)
{
  if (list == NULL)
    return;

  g_list_foreach (list, (GFunc) poppler_form_field_mapping_free, NULL);
  g_list_free (list);
}

/**
 * poppler_page_get_annot_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerAnnotMapping items that map from a location on
 * @page to a #PopplerAnnot.  This list must be freed with
 * poppler_page_free_annot_mapping() when done.
 *
 * Return value: A #GList of #PopplerAnnotMapping
 **/
GList *
poppler_page_get_annot_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  double width, height;
  gint i;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  if (!page->annots)
    page->annots = page->page->getAnnots (page->document->doc->getCatalog ());
  
  if (!page->annots)
    return NULL;

  poppler_page_get_size (page, &width, &height);

  for (i = 0; i < page->annots->getNumAnnots (); i++) {
    PopplerAnnotMapping *mapping;
    PopplerRectangle rect;
    Annot *annot;
    PDFRectangle *annot_rect;
    gint rotation = 0;

    annot = page->annots->getAnnot (i);

    /* Create the mapping */
    mapping = poppler_annot_mapping_new ();

    switch (annot->getType ())
      {
      case Annot::typeText:
        mapping->annot = _poppler_annot_text_new (annot);
	break;
      case Annot::typeFreeText:
        mapping->annot = _poppler_annot_free_text_new (annot);
	break;
      default:
        mapping->annot = _poppler_annot_new (annot);
	break;
      }

    annot_rect = annot->getRect ();
    rect.x1 = annot_rect->x1;
    rect.y1 = annot_rect->y1;
    rect.x2 = annot_rect->x2;
    rect.y2 = annot_rect->y2;

    if (! (annot->getFlags () & Annot::flagNoRotate))
      rotation = page->page->getRotate ();

    switch (rotation)
      {
      case 90:
        mapping->area.x1 = rect.y1;
        mapping->area.y1 = height - rect.x2;
        mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
        mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
        break;
      case 180:
        mapping->area.x1 = width - rect.x2;
        mapping->area.y1 = height - rect.y2;
        mapping->area.x2 = mapping->area.x1 + (rect.x2 - rect.x1);
        mapping->area.y2 = mapping->area.y1 + (rect.y2 - rect.y1);
        break;
      case 270:
        mapping->area.x1 = width - rect.y2;
        mapping->area.y1 = rect.x1;
        mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
        mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
        break;
      default:
        mapping->area.x1 = rect.x1;
        mapping->area.y1 = rect.y1;
        mapping->area.x2 = rect.x2;
        mapping->area.y2 = rect.y2;
      }

    mapping->area.x1 -= page->page->getCropBox()->x1;
    mapping->area.x2 -= page->page->getCropBox()->x1;
    mapping->area.y1 -= page->page->getCropBox()->y1;
    mapping->area.y2 -= page->page->getCropBox()->y1;

    map_list = g_list_prepend (map_list, mapping);
  }

  return g_list_reverse (map_list);
}

/**
 * poppler_page_free_annot_mapping:
 * @list: A list of #PopplerAnnotMapping<!-- -->s
 *
 * Frees a list of #PopplerAnnotMapping<!-- -->s allocated by
 * poppler_page_get_annot_mapping().  It also frees the #PopplerAnnot<!-- -->s
 * that each mapping contains, so if you want to keep them around, you need to
 * copy them with poppler_annot_copy().
 **/
void
poppler_page_free_annot_mapping (GList *list)
{
  if (!list)
    return;

  g_list_foreach (list, (GFunc)poppler_annot_mapping_free, NULL);
  g_list_free (list);
}

/* PopplerRectangle type */

POPPLER_DEFINE_BOXED_TYPE (PopplerRectangle, poppler_rectangle,
			   poppler_rectangle_copy,
			   poppler_rectangle_free)

PopplerRectangle *
poppler_rectangle_new (void)
{
  return g_new0 (PopplerRectangle, 1);
}

PopplerRectangle *
poppler_rectangle_copy (PopplerRectangle *rectangle)
{
  PopplerRectangle *new_rectangle;

  g_return_val_if_fail (rectangle != NULL, NULL);
  
  new_rectangle = g_new (PopplerRectangle, 1);
  *new_rectangle = *rectangle;

  return new_rectangle;
}

void
poppler_rectangle_free (PopplerRectangle *rectangle)
{
  g_free (rectangle);
}

/* PopplerColor type */
POPPLER_DEFINE_BOXED_TYPE (PopplerColor, poppler_color, poppler_color_copy, poppler_color_free)

PopplerColor *
poppler_color_new (void)
{
  return (PopplerColor *) g_new0 (PopplerColor, 1);
}

PopplerColor *
poppler_color_copy (PopplerColor *color)
{
  PopplerColor *new_color;

  new_color = g_new (PopplerColor, 1);
  *new_color = *color;

  return new_color;
}

void
poppler_color_free (PopplerColor *color)
{
  g_free (color);
}

/* PopplerLinkMapping type */
POPPLER_DEFINE_BOXED_TYPE (PopplerLinkMapping, poppler_link_mapping,
			   poppler_link_mapping_copy,
			   poppler_link_mapping_free)

PopplerLinkMapping *
poppler_link_mapping_new (void)
{
  return (PopplerLinkMapping *) g_new0 (PopplerLinkMapping, 1);
}

PopplerLinkMapping *
poppler_link_mapping_copy (PopplerLinkMapping *mapping)
{
  PopplerLinkMapping *new_mapping;

  new_mapping = poppler_link_mapping_new ();
	
  *new_mapping = *mapping;
  if (new_mapping->action)
    new_mapping->action = poppler_action_copy (new_mapping->action);

  return new_mapping;
}

void
poppler_link_mapping_free (PopplerLinkMapping *mapping)
{
  if (mapping->action)
    poppler_action_free (mapping->action);

  g_free (mapping);
}

/* Poppler Image mapping type */
POPPLER_DEFINE_BOXED_TYPE (PopplerImageMapping, poppler_image_mapping,
			   poppler_image_mapping_copy,
			   poppler_image_mapping_free)

PopplerImageMapping *
poppler_image_mapping_new (void)
{
  return (PopplerImageMapping *) g_new0 (PopplerImageMapping, 1);
}

PopplerImageMapping *
poppler_image_mapping_copy (PopplerImageMapping *mapping)
{
  PopplerImageMapping *new_mapping;

  new_mapping = poppler_image_mapping_new ();

  *new_mapping = *mapping;

  return new_mapping;
}

void
poppler_image_mapping_free (PopplerImageMapping *mapping)
{
  g_free (mapping);
}

/* Page Transition */
POPPLER_DEFINE_BOXED_TYPE (PopplerPageTransition, poppler_page_transition,
			   poppler_page_transition_copy,
			   poppler_page_transition_free)

PopplerPageTransition *
poppler_page_transition_new (void)
{
  return (PopplerPageTransition *) g_new0 (PopplerPageTransition, 1);
}

PopplerPageTransition *
poppler_page_transition_copy (PopplerPageTransition *transition)
{
  PopplerPageTransition *new_transition;

  new_transition = poppler_page_transition_new ();
  *new_transition = *transition;
  
  return new_transition;
}

void
poppler_page_transition_free (PopplerPageTransition *transition)
{
  g_free (transition);
}

/* Form Field Mapping Type */
POPPLER_DEFINE_BOXED_TYPE (PopplerFormFieldMapping, poppler_form_field_mapping,
			   poppler_form_field_mapping_copy,
			   poppler_form_field_mapping_free)

PopplerFormFieldMapping *
poppler_form_field_mapping_new (void)
{
  return (PopplerFormFieldMapping *) g_new0 (PopplerFormFieldMapping, 1);
}

PopplerFormFieldMapping *
poppler_form_field_mapping_copy (PopplerFormFieldMapping *mapping)
{
  PopplerFormFieldMapping *new_mapping;
  
  new_mapping = poppler_form_field_mapping_new ();
  *new_mapping = *mapping;

  if (mapping->field)
	  new_mapping->field = (PopplerFormField *)g_object_ref (mapping->field);
	    
  return new_mapping;
}

void
poppler_form_field_mapping_free (PopplerFormFieldMapping *mapping)
{
  if (!mapping)
    return;

  if (mapping->field)
    g_object_unref (mapping->field);
  
  g_free (mapping);
}

/* PopplerAnnot Mapping Type */
POPPLER_DEFINE_BOXED_TYPE (PopplerAnnotMapping, poppler_annot_mapping,
			   poppler_annot_mapping_copy,
			   poppler_annot_mapping_free)

PopplerAnnotMapping *
poppler_annot_mapping_new (void)
{
  return (PopplerAnnotMapping *) g_new0 (PopplerAnnotMapping, 1);
}

PopplerAnnotMapping *
poppler_annot_mapping_copy (PopplerAnnotMapping *mapping)
{
  PopplerAnnotMapping *new_mapping;

  new_mapping = poppler_annot_mapping_new ();

  *new_mapping = *mapping;
  if (mapping->annot)
    new_mapping->annot = (PopplerAnnot *) g_object_ref (mapping->annot);

  return new_mapping;
}

void
poppler_annot_mapping_free (PopplerAnnotMapping *mapping)
{
  if (!mapping)
    return;

  if (mapping->annot)
    g_object_unref (mapping->annot);

  g_free (mapping);
}

void 
poppler_page_get_crop_box (PopplerPage *page, PopplerRectangle *rect)
{
  PDFRectangle* cropBox = page->page->getCropBox ();
  
  rect->x1 = cropBox->x1;
  rect->x2 = cropBox->x2;
  rect->y1 = cropBox->y1;
  rect->y2 = cropBox->y2;
}

