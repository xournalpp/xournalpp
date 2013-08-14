/* poppler-annot.cc: glib interface to poppler
 *
 * Copyright (C) 2007 Inigo Martinez <inigomartinez@gmail.com>
 * Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include "poppler.h"
#include "poppler-private.h"

typedef struct _PopplerAnnotClass         PopplerAnnotClass;
typedef struct _PopplerAnnotMarkupClass   PopplerAnnotMarkupClass;
typedef struct _PopplerAnnotFreeTextClass PopplerAnnotFreeTextClass;
typedef struct _PopplerAnnotTextClass     PopplerAnnotTextClass;

struct _PopplerAnnot
{
  GObject  parent_instance;
  Annot   *annot;
};

struct _PopplerAnnotClass
{
  GObjectClass parent_class;
};

struct _PopplerAnnotMarkup
{
  PopplerAnnot parent_instance;
};

struct _PopplerAnnotMarkupClass
{
  PopplerAnnotClass parent_class;
};

struct _PopplerAnnotText
{
  PopplerAnnotMarkup parent_instance;
};

struct _PopplerAnnotTextClass
{
  PopplerAnnotMarkupClass parent_class;
};

struct _PopplerAnnotFreeText
{
  PopplerAnnotMarkup parent_instance;
};

struct _PopplerAnnotFreeTextClass
{
  PopplerAnnotMarkupClass parent_class;
};

G_DEFINE_TYPE (PopplerAnnot, poppler_annot, G_TYPE_OBJECT)
G_DEFINE_TYPE (PopplerAnnotMarkup, poppler_annot_markup, POPPLER_TYPE_ANNOT)
G_DEFINE_TYPE (PopplerAnnotText, poppler_annot_text, POPPLER_TYPE_ANNOT_MARKUP)
G_DEFINE_TYPE (PopplerAnnotFreeText, poppler_annot_free_text, POPPLER_TYPE_ANNOT_MARKUP)

static void
poppler_annot_finalize (GObject *object)
{
  PopplerAnnot *poppler_annot = POPPLER_ANNOT (object);

  poppler_annot->annot = NULL;

  G_OBJECT_CLASS (poppler_annot_parent_class)->finalize (object);
}

static void
poppler_annot_init (PopplerAnnot *poppler_annot)
{
}

static void
poppler_annot_class_init (PopplerAnnotClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_annot_finalize;
}

PopplerAnnot *
_poppler_annot_new (Annot *annot)
{
  PopplerAnnot *poppler_annot;

  poppler_annot = POPPLER_ANNOT (g_object_new (POPPLER_TYPE_ANNOT, NULL));
  poppler_annot->annot = annot;

  return poppler_annot;
}

static void
poppler_annot_markup_init (PopplerAnnotMarkup *poppler_annot)
{
}

static void
poppler_annot_markup_class_init (PopplerAnnotMarkupClass *klass)
{
}

static void
poppler_annot_text_init (PopplerAnnotText *poppler_annot)
{
}

static void
poppler_annot_text_class_init (PopplerAnnotTextClass *klass)
{
}

PopplerAnnot *
_poppler_annot_text_new (Annot *annot)
{
  PopplerAnnot *poppler_annot;

  poppler_annot = POPPLER_ANNOT (g_object_new (POPPLER_TYPE_ANNOT_TEXT, NULL));
  poppler_annot->annot = annot;

  return poppler_annot;
}

static void
poppler_annot_free_text_init (PopplerAnnotFreeText *poppler_annot)
{
}

static void
poppler_annot_free_text_class_init (PopplerAnnotFreeTextClass *klass)
{
}

PopplerAnnot *
_poppler_annot_free_text_new (Annot *annot)
{
  PopplerAnnot *poppler_annot;

  poppler_annot = POPPLER_ANNOT (g_object_new (POPPLER_TYPE_ANNOT_FREE_TEXT, NULL));
  poppler_annot->annot = annot;

  return poppler_annot;
}

/* Public methods */
/**
 * poppler_annot_get_annot_type:
 * @poppler_annot: a #PopplerAnnot
 *
 * Gets the type of @poppler_annot
 *
 * Return value: #PopplerAnnotType of @poppler_annot.
 **/ 
PopplerAnnotType
poppler_annot_get_annot_type (PopplerAnnot *poppler_annot)
{
  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), POPPLER_ANNOT_UNKNOWN);

  switch (poppler_annot->annot->getType ())
    {
    case Annot::typeText:
      return POPPLER_ANNOT_TEXT;
    case Annot::typeLink:
      return POPPLER_ANNOT_LINK;
    case Annot::typeFreeText:
      return POPPLER_ANNOT_FREE_TEXT;
    case Annot::typeLine:
      return POPPLER_ANNOT_LINE;
    case Annot::typeSquare:
      return POPPLER_ANNOT_SQUARE;
    case Annot::typeCircle:
      return POPPLER_ANNOT_CIRCLE;
    case Annot::typePolygon:
      return POPPLER_ANNOT_POLYGON;
    case Annot::typePolyLine:
      return POPPLER_ANNOT_POLY_LINE;
    case Annot::typeHighlight:
      return POPPLER_ANNOT_HIGHLIGHT;
    case Annot::typeUnderline:
      return POPPLER_ANNOT_UNDERLINE;
    case Annot::typeSquiggly:
      return POPPLER_ANNOT_SQUIGGLY;
    case Annot::typeStrikeOut:
      return POPPLER_ANNOT_STRIKE_OUT;
    case Annot::typeStamp:
      return POPPLER_ANNOT_STAMP;
    case Annot::typeCaret:
      return POPPLER_ANNOT_CARET;
    case Annot::typeInk:
      return POPPLER_ANNOT_INK;
    case Annot::typePopup:
      return POPPLER_ANNOT_POPUP;
    case Annot::typeFileAttachment:
      return POPPLER_ANNOT_FILE_ATTACHMENT;
    case Annot::typeSound:
      return POPPLER_ANNOT_SOUND;
    case Annot::typeMovie:
      return POPPLER_ANNOT_MOVIE;
    case Annot::typeWidget:
      return POPPLER_ANNOT_WIDGET;
    case Annot::typeScreen:
      return POPPLER_ANNOT_SCREEN;
    case Annot::typePrinterMark:
      return POPPLER_ANNOT_PRINTER_MARK;
    case Annot::typeTrapNet:
      return POPPLER_ANNOT_TRAP_NET;
    case Annot::typeWatermark:
      return POPPLER_ANNOT_WATERMARK;
    case Annot::type3D:
      return POPPLER_ANNOT_3D;
    default:
      g_warning ("Unsupported Annot Type");
    }

  return POPPLER_ANNOT_UNKNOWN;
}

/**
 * poppler_annot_get_contents:
 * @poppler_annot: a #PopplerAnnot
 *
 * Retrieves the contents of @poppler_annot.
 *
 * Return value: a new allocated string with the contents of @poppler_annot. It
 *               must be freed with g_free() when done.
 **/
gchar *
poppler_annot_get_contents (PopplerAnnot *poppler_annot)
{
  GooString *contents;

  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), NULL);

  contents = poppler_annot->annot->getContents ();

  return contents ? _poppler_goo_string_to_utf8 (contents) : NULL;
}

/**
 * poppler_annot_set_contents:
 * @poppler_annot: a #PopplerAnnot
 * @contents: a text string containing the new contents
 *
 * Sets the contents of @poppler_annot to the given value,
 * replacing the current contents.
 **/
void
poppler_annot_set_contents (PopplerAnnot *poppler_annot,
			    const gchar  *contents)
{
  GooString *goo_tmp;
  gchar *tmp;
  gsize length = 0;
  
  g_return_if_fail (POPPLER_IS_ANNOT (poppler_annot));

  tmp = contents ? g_convert (contents, -1, "UTF-16BE", "UTF-8", NULL, &length, NULL) : NULL;
  goo_tmp = new GooString (tmp, length);
  g_free (tmp);
  poppler_annot->annot->setContents (goo_tmp);
  delete (goo_tmp);
}

/**
 * poppler_annot_get_name:
 * @poppler_annot: a #PopplerAnnot
 *
 * Retrieves the name of @poppler_annot.
 *
 * Return value: a new allocated string with the name of @poppler_annot. It must
 *               be freed with g_free() when done.
 **/
gchar *
poppler_annot_get_name (PopplerAnnot *poppler_annot)
{
  GooString *name;

  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), NULL);

  name = poppler_annot->annot->getName ();

  return name ? _poppler_goo_string_to_utf8 (name) : NULL;
}

/**
 * poppler_annot_get_modified:
 * @poppler_annot: a #PopplerAnnot
 *
 * Retrieves the last modification data of @poppler_annot. The returned
 * string will be either a PDF format date or a text string.
 * See also #poppler_date_parse()
 *
 * Return value: a new allocated string with the last modification data of
 *               @poppler_annot. It must be freed with g_free() when done.
 **/
gchar *
poppler_annot_get_modified (PopplerAnnot *poppler_annot)
{
  GooString *text;

  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), NULL);

  text = poppler_annot->annot->getModified ();

  return text ? _poppler_goo_string_to_utf8 (text) : NULL;
}

/**
 * poppler_annot_get_flags
 * @poppler_annot: a #PopplerAnnot
 *
 * Retrieves the flag field specifying various characteristics of the
 * @poppler_annot.
 *
 * Return value: the flag field of @poppler_annot.
 **/
PopplerAnnotFlag
poppler_annot_get_flags (PopplerAnnot *poppler_annot)
{
  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), (PopplerAnnotFlag) 0);

  return (PopplerAnnotFlag) poppler_annot->annot->getFlags ();
}

/**
 * poppler_annot_get_color:
 * @poppler_annot: a #PopplerAnnot
 *
 * Retrieves the color of @poppler_annot.
 *
 * Return value: a new allocated #PopplerColor with the color values of
 *               @poppler_annot, or %NULL. It must be freed with g_free() when done.
 **/
PopplerColor *
poppler_annot_get_color (PopplerAnnot *poppler_annot)
{
  AnnotColor *color;
  PopplerColor *poppler_color = NULL;

  g_return_val_if_fail (POPPLER_IS_ANNOT (poppler_annot), NULL);

  color = poppler_annot->annot->getColor ();

  if (color) {
    double *values = color->getValues ();

    switch (color->getSpace ())
      {
      case AnnotColor::colorGray:
        poppler_color = g_new (PopplerColor, 1);
	
        poppler_color->red = (guint16) (values[0] * 65535);
        poppler_color->green = poppler_color->red;
        poppler_color->blue = poppler_color->red;

	break;
      case AnnotColor::colorRGB:
        poppler_color = g_new (PopplerColor, 1);
	
        poppler_color->red = (guint16) (values[0] * 65535);
        poppler_color->green = (guint16) (values[1] * 65535);
        poppler_color->blue = (guint16) (values[2] * 65535);

	break;
      case AnnotColor::colorCMYK:
        g_warning ("Unsupported Annot Color: colorCMYK");
      case AnnotColor::colorTransparent:
        break;
      }
  }

  return poppler_color;
}

/* PopplerAnnotMarkup */
/**
* poppler_annot_markup_get_label:
* @poppler_annot: a #PopplerAnnotMarkup
*
* Retrieves the label text of @poppler_annot.
*
* Return value: the label text of @poppler_annot.
*/
gchar *
poppler_annot_markup_get_label (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;
  GooString *text;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), NULL);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);

  text = annot->getLabel ();

  return text ? _poppler_goo_string_to_utf8 (text) : NULL;
}

/**
 * poppler_annot_markup_has_popup:
 * @poppler_annot: a #PopplerAnnotMarkup
 *
 * Return %TRUE if the markup annotation has a popup window associated
 *
 * Return value: %TRUE, if @poppler_annot has popup, %FALSE otherwise
 **/
gboolean
poppler_annot_markup_has_popup (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), FALSE);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);

  return annot->getPopup () != NULL;
}

/**
 * poppler_annot_markup_get_popup_is_open:
 * @poppler_annot: a #PopplerAnnotMarkup
 *
 * Retrieves the state of the popup annot related to @poppler_annot.
 *
 * Return value: the state of @poppler_annot. %TRUE if it's open, %FALSE in
 *               other case.
 **/
gboolean
poppler_annot_markup_get_popup_is_open (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;
  AnnotPopup *annot_popup;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), FALSE);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);

  if ((annot_popup = annot->getPopup ()))
    return annot_popup->getOpen ();

  return FALSE;
}

/**
 * poppler_annot_markup_get_popup_rectangle:
 * @poppler_annot: a #PopplerAnnotMarkup
 * @poppler_rect: a #PopplerRectangle to store the popup rectangle
 *
 * Retrieves the rectangle of the popup annot related to @poppler_annot.
 *
 * Return value: %TRUE if #PopplerRectangle was correctly filled,
 *               %FALSE otherwise
 **/
gboolean
poppler_annot_markup_get_popup_rectangle (PopplerAnnotMarkup *poppler_annot,
					  PopplerRectangle   *poppler_rect)
{
  AnnotMarkup *annot;
  Annot *annot_popup;
  PDFRectangle *annot_rect;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), FALSE);
  g_return_val_if_fail (poppler_rect != NULL, FALSE);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);
  annot_popup = annot->getPopup ();
  if (!annot_popup)
    return FALSE;

  annot_rect = annot_popup->getRect ();
  poppler_rect->x1 = annot_rect->x1;
  poppler_rect->x2 = annot_rect->x2;
  poppler_rect->y1 = annot_rect->y1;
  poppler_rect->y2 = annot_rect->y2;

  return TRUE;
}

/**
* poppler_annot_markup_get_opacity:
* @poppler_annot: a #PopplerAnnotMarkup
*
* Retrieves the opacity value of @poppler_annot.
*
* Return value: the opacity value of @poppler_annot.
*/
gdouble
poppler_annot_markup_get_opacity (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), 0);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);
  
  return annot->getOpacity ();
}

GDate *
poppler_annot_markup_get_date (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;
  GooString *annot_date;
  time_t timet;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), NULL);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);
  annot_date = annot->getDate ();
  if (!annot_date)
    return NULL;

  if (_poppler_convert_pdf_date_to_gtime (annot_date, &timet)) {
    GDate *date;

    date = g_date_new ();
    g_date_set_time_t (date, timet);

    return date;
  }

  return NULL;
}

/**
* poppler_annot_markup_get_subject:
* @poppler_annot: a #PopplerAnnotMarkup
*
* Retrives the subject text of @poppler_annot.
*
* Return value: the subject text of @poppler_annot.
*/
gchar *
poppler_annot_markup_get_subject (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;
  GooString *text;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), NULL);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);

  text = annot->getSubject ();

  return text ? _poppler_goo_string_to_utf8 (text) : NULL;
}

/**
* poppler_annot_markup_get_reply_to:
* @poppler_annot: a #PopplerAnnotMarkup
*
* Gets the reply type of @poppler_annot.
*
* Return value: #PopplerAnnotMarkupReplyType of @poppler_annot.
*/
PopplerAnnotMarkupReplyType
poppler_annot_markup_get_reply_to (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), POPPLER_ANNOT_MARKUP_REPLY_TYPE_R);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);
  
  switch (annot->getReplyTo ())
  {
    case AnnotMarkup::replyTypeR:
      return POPPLER_ANNOT_MARKUP_REPLY_TYPE_R;
    case AnnotMarkup::replyTypeGroup:
      return POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP;
    default:
      g_warning ("Unsupported Annot Markup Reply To Type");
  }

  return POPPLER_ANNOT_MARKUP_REPLY_TYPE_R;
}

/**
* poppler_annot_markup_get_external_data:
* @poppler_annot: a #PopplerAnnotMarkup
*
* Gets the external data type of @poppler_annot.
*
* Return value: #PopplerAnnotExternalDataType of @poppler_annot.
*/
PopplerAnnotExternalDataType
poppler_annot_markup_get_external_data (PopplerAnnotMarkup *poppler_annot)
{
  AnnotMarkup *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_MARKUP (poppler_annot), POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN);

  annot = static_cast<AnnotMarkup *>(POPPLER_ANNOT (poppler_annot)->annot);

  switch (annot->getExData ())
    {
    case annotExternalDataMarkup3D:
      return POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D;
    case annotExternalDataMarkupUnknown:
      return POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN;
    default:
      g_warning ("Unsupported Annot Markup External Data");
    }

  return POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN;
}

/* PopplerAnnotText */
/**
 * poppler_annot_text_get_is_open:
 * @poppler_annot: a #PopplerAnnotText
 *
 * Retrieves the state of @poppler_annot.
 *
 * Return value: the state of @poppler_annot. %TRUE if it's open, %FALSE in
 *               other case.
 **/
gboolean
poppler_annot_text_get_is_open (PopplerAnnotText *poppler_annot)
{
  AnnotText *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_TEXT (poppler_annot), FALSE);

  annot = static_cast<AnnotText *>(POPPLER_ANNOT (poppler_annot)->annot);

  return annot->getOpen ();
}

/**
 * poppler_annot_text_get_icon:
 * @poppler_annot: a #PopplerAnnotText
 *
 * Gets the icon type of @poppler_annot.
 *
 * Return value: #PopplerAnnotTextIcon of @poppler_annot.
 **/ 
gchar *
poppler_annot_text_get_icon (PopplerAnnotText *poppler_annot)
{
  AnnotText *annot;
  GooString *text;

  g_return_val_if_fail (POPPLER_IS_ANNOT_TEXT (poppler_annot), NULL);

  annot = static_cast<AnnotText *>(POPPLER_ANNOT (poppler_annot)->annot);

  text = annot->getIcon ();

  return text ? _poppler_goo_string_to_utf8 (text) : NULL;
}

/**
 * poppler_annot_text_get_state:
 * @poppler_annot: a #PopplerAnnotText
 *
 * Retrieves the state of @poppler_annot.
 *
 * Return value: #PopplerAnnotTextState of @poppler_annot.
 **/ 
PopplerAnnotTextState
poppler_annot_text_get_state (PopplerAnnotText *poppler_annot)
{
  AnnotText *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_TEXT (poppler_annot), POPPLER_ANNOT_TEXT_STATE_UNKNOWN);

  annot = static_cast<AnnotText *>(POPPLER_ANNOT (poppler_annot)->annot);

  switch (annot->getState ())
    {
    case AnnotText::stateUnknown:
      return POPPLER_ANNOT_TEXT_STATE_UNKNOWN;
    case AnnotText::stateMarked:
      return POPPLER_ANNOT_TEXT_STATE_MARKED;
    case AnnotText::stateUnmarked:
      return POPPLER_ANNOT_TEXT_STATE_UNMARKED;
    case AnnotText::stateAccepted:
      return POPPLER_ANNOT_TEXT_STATE_ACCEPTED;
    case AnnotText::stateRejected:
      return POPPLER_ANNOT_TEXT_STATE_REJECTED;
    case AnnotText::stateCancelled:
      return POPPLER_ANNOT_TEXT_STATE_CANCELLED;
    case AnnotText::stateCompleted:
      return POPPLER_ANNOT_TEXT_STATE_COMPLETED;
    case AnnotText::stateNone:
      return POPPLER_ANNOT_TEXT_STATE_NONE;
    default:
      g_warning ("Unsupported Annot Text State");
    }

  return POPPLER_ANNOT_TEXT_STATE_UNKNOWN;
}

/* PopplerAnnotFreeText */
/**
 * poppler_annot_free_text_get_quadding:
 * @poppler_annot: a #PopplerAnnotFreeText
 *
 * Retrieves the justification of the text of @poppler_annot.
 *
 * Return value: #PopplerAnnotFreeTextQuadding of @poppler_annot.
 **/ 
PopplerAnnotFreeTextQuadding
poppler_annot_free_text_get_quadding (PopplerAnnotFreeText *poppler_annot)
{
  AnnotFreeText *annot;

  g_return_val_if_fail (POPPLER_IS_ANNOT_FREE_TEXT (poppler_annot), POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED);

  annot = static_cast<AnnotFreeText *>(POPPLER_ANNOT (poppler_annot)->annot);

  switch (annot->getQuadding ())
  {
    case AnnotFreeText::quaddingLeftJustified:
      return POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED;
    case AnnotFreeText::quaddingCentered:
      return POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED;
    case AnnotFreeText::quaddingRightJustified:
      return POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED;
    default:
      g_warning ("Unsupported Annot Free Text Quadding");
  }

  return POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED;
}

/**
 * poppler_annot_free_text_get_callout_line:
 * @poppler_annot: a #PopplerAnnotFreeText
 *
 * Retrieves a #PopplerCalloutLine of four or six numbers specifying a callout
 * line attached to the @poppler_annot.
 *
 * Return value: a new allocated #PopplerCalloutLine if the annot has a callout
 *               line, NULL in other case. It must be freed with g_free() when
 *               done.
 **/
PopplerAnnotCalloutLine *
poppler_annot_free_text_get_callout_line (PopplerAnnotFreeText *poppler_annot)
{
  AnnotFreeText *annot;
  AnnotCalloutLine *line;

  g_return_val_if_fail (POPPLER_IS_ANNOT_FREE_TEXT (poppler_annot), NULL);

  annot = static_cast<AnnotFreeText *>(POPPLER_ANNOT (poppler_annot)->annot);

  if ((line = annot->getCalloutLine ())) {
    AnnotCalloutMultiLine *multiline;
    PopplerAnnotCalloutLine *callout = g_new0 (PopplerAnnotCalloutLine, 1);

    callout->x1 = line->getX1();
    callout->y1 = line->getY1();
    callout->x2 = line->getX2();
    callout->y2 = line->getY2();

    if ((multiline = static_cast<AnnotCalloutMultiLine *>(line))) {
      callout->multiline = TRUE;
      callout->x3 = multiline->getX3();
      callout->y3 = multiline->getY3();
      return callout;
    }

    callout->multiline = FALSE;
    return callout;
  }

  return NULL;
}

/* PopplerAnnotCalloutLine */
POPPLER_DEFINE_BOXED_TYPE (PopplerAnnotCalloutLine, poppler_annot_callout_line,
			   poppler_annot_callout_line_copy,
			   poppler_annot_callout_line_free)

/**
 * poppler_annot_callout_line_new:
 *
 * Creates a new empty #PopplerAnnotCalloutLine.
 *
 * Return value: a new allocated #PopplerAnnotCalloutLine, NULL in other case.
 *               It must be freed when done.
 **/
PopplerAnnotCalloutLine *
poppler_annot_callout_line_new (void)
{
  return g_new0 (PopplerAnnotCalloutLine, 1);
}

/**
 * poppler_annot_callout_line_copy:
 * @callout: the #PopplerAnnotCalloutline to be copied.
 *
 * It does copy @callout to a new #PopplerAnnotCalloutLine.
 *
 * Return value: a new allocated #PopplerAnnotCalloutLine as exact copy of
 *               @callout, NULL in other case. It must be freed when done.
 **/
PopplerAnnotCalloutLine *
poppler_annot_callout_line_copy (PopplerAnnotCalloutLine *callout)
{
  PopplerAnnotCalloutLine *new_callout;

  g_return_val_if_fail (callout != NULL, NULL);
  
  new_callout = g_new0 (PopplerAnnotCalloutLine, 1);
  *new_callout = *callout;

  return new_callout;
}

/**
 * poppler_annot_callout_line_free:
 * @callout: a #PopplerAnnotCalloutLine
 *
 * Frees the memory used by #PopplerAnnotCalloutLine.
 **/
void
poppler_annot_callout_line_free (PopplerAnnotCalloutLine *callout)
{
  g_free (callout);
}
