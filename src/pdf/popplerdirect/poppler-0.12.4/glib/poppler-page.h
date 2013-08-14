/* poppler-page.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
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

#ifndef __POPPLER_PAGE_H__
#define __POPPLER_PAGE_H__

#include <glib-object.h>

#include "poppler.h"

#ifdef POPPLER_WITH_GDK
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#ifdef POPPLER_HAS_CAIRO
#include <cairo.h>
#endif

G_BEGIN_DECLS

#define POPPLER_TYPE_PAGE             (poppler_page_get_type ())
#define POPPLER_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_PAGE, PopplerPage))
#define POPPLER_IS_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_PAGE))


GType      	       poppler_page_get_type             (void) G_GNUC_CONST;

#ifdef POPPLER_WITH_GDK
void                   poppler_page_render_to_pixbuf     (PopplerPage        *page,
							  int                 src_x,
							  int                 src_y,
							  int                 src_width,
							  int                 src_height,
							  double              scale,
							  int                 rotation,
							  GdkPixbuf          *pixbuf);
void          poppler_page_render_to_pixbuf_for_printing (PopplerPage        *page,
							  int                 src_x,
							  int                 src_y,
							  int                 src_width,
							  int                 src_height,
							  double              scale,
							  int                 rotation,
							  GdkPixbuf          *pixbuf);
GdkPixbuf             *poppler_page_get_thumbnail_pixbuf (PopplerPage        *page);
void                poppler_page_render_selection_to_pixbuf (
							  PopplerPage        *page,
							  gdouble             scale,
							  int		      rotation,
							  GdkPixbuf          *pixbuf,
							  PopplerRectangle   *selection,
							  PopplerRectangle   *old_selection,
							  PopplerSelectionStyle style,
							  GdkColor           *glyph_color,
							  GdkColor           *background_color);
#endif /* POPPLER_WITH_GDK */

#ifdef POPPLER_HAS_CAIRO
void                   poppler_page_render               (PopplerPage        *page,
							  cairo_t            *cairo);
void                   poppler_page_render_for_printing  (PopplerPage        *page,
							  cairo_t            *cairo);
cairo_surface_t       *poppler_page_get_thumbnail        (PopplerPage        *page);
void                   poppler_page_render_selection     (PopplerPage        *page,
							  cairo_t            *cairo,
							  PopplerRectangle   *selection,
							  PopplerRectangle   *old_selection,
							  PopplerSelectionStyle style,
							  PopplerColor       *glyph_color,
							  PopplerColor       *background_color);
#endif /* POPPLER_HAS_CAIRO */

void                   poppler_page_get_size             (PopplerPage        *page,
							  double             *width,
							  double             *height);
int                    poppler_page_get_index            (PopplerPage        *page);
double                 poppler_page_get_duration         (PopplerPage        *page);
PopplerPageTransition *poppler_page_get_transition       (PopplerPage        *page);
gboolean               poppler_page_get_thumbnail_size   (PopplerPage        *page,
							  int                *width,
							  int                *height);
GList     	      *poppler_page_find_text            (PopplerPage        *page,
							  const  char        *text);
void                   poppler_page_render_to_ps         (PopplerPage        *page,
							  PopplerPSFile      *ps_file);
char                  *poppler_page_get_text             (PopplerPage        *page,
							  PopplerSelectionStyle style,
							  PopplerRectangle   *rect);
GList                 *poppler_page_get_selection_region (PopplerPage        *page,
							  gdouble             scale,
							  PopplerSelectionStyle style,
							  PopplerRectangle   *selection);
void                   poppler_page_selection_region_free(GList              *region);
GList                 *poppler_page_get_link_mapping     (PopplerPage        *page);
void                   poppler_page_free_link_mapping    (GList              *list);
GList                 *poppler_page_get_image_mapping    (PopplerPage        *page);
void                   poppler_page_free_image_mapping   (GList              *list);
#ifdef POPPLER_HAS_CAIRO
cairo_surface_t       *poppler_page_get_image            (PopplerPage        *page,
							  gint                image_id);
#endif /* POPPLER_HAS_CAIRO */
GList              *poppler_page_get_form_field_mapping  (PopplerPage        *page);
void                poppler_page_free_form_field_mapping (GList              *list);
GList                 *poppler_page_get_annot_mapping    (PopplerPage        *page);
void                   poppler_page_free_annot_mapping   (GList              *list);
void 		      poppler_page_get_crop_box 	 (PopplerPage        *page,
							  PopplerRectangle   *rect);


/* A rectangle on a page, with coordinates in PDF points. */
#define POPPLER_TYPE_RECTANGLE             (poppler_rectangle_get_type ())
struct _PopplerRectangle
{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
};

GType             poppler_rectangle_get_type (void) G_GNUC_CONST;
PopplerRectangle *poppler_rectangle_new      (void);
PopplerRectangle *poppler_rectangle_copy     (PopplerRectangle *rectangle);
void              poppler_rectangle_free     (PopplerRectangle *rectangle);

/* A color in RGB */
#define POPPLER_TYPE_COLOR                 (poppler_color_get_type ())
struct _PopplerColor
{
  guint16 red;
  guint16 green;
  guint16 blue;
};

GType             poppler_color_get_type      (void) G_GNUC_CONST;
PopplerColor     *poppler_color_new           (void);
PopplerColor     *poppler_color_copy          (PopplerColor *color);
void              poppler_color_free          (PopplerColor *color);

/* Mapping between areas on the current page and PopplerActions */
#define POPPLER_TYPE_LINK_MAPPING             (poppler_link_mapping_get_type ())
struct  _PopplerLinkMapping
{
  PopplerRectangle area;
  PopplerAction *action;
};

GType               poppler_link_mapping_get_type (void) G_GNUC_CONST;
PopplerLinkMapping *poppler_link_mapping_new      (void);
PopplerLinkMapping *poppler_link_mapping_copy     (PopplerLinkMapping *mapping);
void                poppler_link_mapping_free     (PopplerLinkMapping *mapping);

/* Page Transition */
#define POPPLER_TYPE_PAGE_TRANSITION                (poppler_page_transition_get_type ())
struct _PopplerPageTransition
{
  PopplerPageTransitionType type;
  PopplerPageTransitionAlignment alignment;
  PopplerPageTransitionDirection direction;
  gint duration;
  gint angle;
  gdouble scale;
  gboolean rectangular;
};

GType                  poppler_page_transition_get_type (void) G_GNUC_CONST;
PopplerPageTransition *poppler_page_transition_new      (void);
PopplerPageTransition *poppler_page_transition_copy     (PopplerPageTransition *transition);
void                   poppler_page_transition_free     (PopplerPageTransition *transition);

/* Mapping between areas on the current page and images */
#define POPPLER_TYPE_IMAGE_MAPPING             (poppler_image_mapping_get_type ())
struct  _PopplerImageMapping
{
  PopplerRectangle area;
  gint image_id;	
};

GType                  poppler_image_mapping_get_type (void) G_GNUC_CONST;
PopplerImageMapping   *poppler_image_mapping_new      (void);
PopplerImageMapping   *poppler_image_mapping_copy     (PopplerImageMapping *mapping);
void                   poppler_image_mapping_free     (PopplerImageMapping *mapping);

/* Mapping between areas on the current page and form fields */
#define POPPLER_TYPE_FORM_FIELD_MAPPING               (poppler_form_field_mapping_get_type ())
struct  _PopplerFormFieldMapping
{
  PopplerRectangle area;
  PopplerFormField *field;
};

GType                    poppler_form_field_mapping_get_type (void) G_GNUC_CONST;
PopplerFormFieldMapping *poppler_form_field_mapping_new      (void);
PopplerFormFieldMapping *poppler_form_field_mapping_copy     (PopplerFormFieldMapping *mapping);
void                     poppler_form_field_mapping_free     (PopplerFormFieldMapping *mapping);

/* Mapping between areas on the current page and annots */
#define POPPLER_TYPE_ANNOT_MAPPING                  (poppler_annot_mapping_get_type ())
struct _PopplerAnnotMapping
{
  PopplerRectangle area;
  PopplerAnnot *annot;
};

GType                poppler_annot_mapping_get_type (void) G_GNUC_CONST;
PopplerAnnotMapping *poppler_annot_mapping_new      (void);
PopplerAnnotMapping *poppler_annot_mapping_copy     (PopplerAnnotMapping *mapping);
void                 poppler_annot_mapping_free     (PopplerAnnotMapping *mapping);

G_END_DECLS

#endif /* __POPPLER_PAGE_H__ */
