/* poppler-document.cc: glib wrapper for poppler
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

#include <string.h>

#include <goo/GooList.h>
#include <splash/SplashBitmap.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>
#include <SplashOutputDev.h>
#include <Stream.h>
#include <FontInfo.h>
#include <PDFDocEncoding.h>
#include <OptionalContent.h>

#include "poppler.h"
#include "poppler-private.h"
#include "poppler-enums.h"

enum {
	PROP_0,
	PROP_TITLE,
	PROP_FORMAT,
	PROP_FORMAT_MAJOR,
	PROP_FORMAT_MINOR,
	PROP_AUTHOR,
	PROP_SUBJECT,
	PROP_KEYWORDS,
	PROP_CREATOR,
	PROP_PRODUCER,
	PROP_CREATION_DATE,
	PROP_MOD_DATE,
	PROP_LINEARIZED,
	PROP_PAGE_LAYOUT,
	PROP_PAGE_MODE,
	PROP_VIEWER_PREFERENCES,
	PROP_PERMISSIONS,
	PROP_METADATA
};

static void poppler_document_layers_free (PopplerDocument *document);

typedef struct _PopplerDocumentClass PopplerDocumentClass;
struct _PopplerDocumentClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerDocument, poppler_document, G_TYPE_OBJECT)

static PopplerDocument *
_poppler_document_new_from_pdfdoc (PDFDoc  *newDoc,
                                   GError **error)
{
  PopplerDocument *document;

  document = (PopplerDocument *) g_object_new (POPPLER_TYPE_DOCUMENT, NULL, NULL);

  if (!newDoc->isOk()) {
    int fopen_errno;
    switch (newDoc->getErrorCode())
      {
      case errOpenFile:
        // If there was an error opening the file, count it as a G_FILE_ERROR 
        // and set the GError parameters accordingly. (this assumes that the 
        // only way to get an errOpenFile error is if newDoc was created using 
        // a filename and thus fopen was called, which right now is true.
        fopen_errno = newDoc->getFopenErrno();
        g_set_error (error, G_FILE_ERROR,
		     g_file_error_from_errno (fopen_errno),
		     "%s", g_strerror (fopen_errno));
	break;
      case errBadCatalog:
        g_set_error (error, POPPLER_ERROR,
		     POPPLER_ERROR_BAD_CATALOG,
		     "Failed to read the document catalog");
	break;
      case errDamaged:
        g_set_error (error, POPPLER_ERROR,
		     POPPLER_ERROR_DAMAGED,
		     "PDF document is damaged");
	break;
      case errEncrypted:
        g_set_error (error, POPPLER_ERROR,
		     POPPLER_ERROR_ENCRYPTED,
		     "Document is encrypted");
	break;
      default:
        g_set_error (error, POPPLER_ERROR,
		     POPPLER_ERROR_INVALID,
		     "Failed to load document");
    }
    
    delete newDoc;
    return NULL;
  }

  document->doc = newDoc;

#if defined (HAVE_CAIRO)
  document->output_dev = new CairoOutputDev ();
  document->output_dev->startDoc(document->doc->getXRef (), document->doc->getCatalog ());
#elif defined (HAVE_SPLASH)
  SplashColor white;
  white[0] = 255;
  white[1] = 255;
  white[2] = 255;
  document->output_dev = new SplashOutputDev(splashModeRGB8, 4, gFalse, white);
  document->output_dev->startDoc(document->doc->getXRef ());
#endif

  return document;
}

/**
 * poppler_document_new_from_file:
 * @uri: uri of the file to load
 * @password: password to unlock the file with, or %NULL
 * @error: Return location for an error, or %NULL
 * 
 * Creates a new #PopplerDocument.  If %NULL is returned, then @error will be
 * set. Possible errors include those in the #POPPLER_ERROR and #G_FILE_ERROR
 * domains.
 * 
 * Return value: A newly created #PopplerDocument, or %NULL
 **/
PopplerDocument *
poppler_document_new_from_file (const char  *uri,
				const char  *password,
				GError     **error)
{
  PDFDoc *newDoc;
  GooString *filename_g;
  GooString *password_g;
  char *filename;

  if (!globalParams) {
    globalParams = new GlobalParams();
  }

  filename = g_filename_from_uri (uri, NULL, error);
  if (!filename)
    return NULL;

  password_g = NULL;
  if (password != NULL) {
    if (g_utf8_validate (password, -1, NULL)) {
      gchar *password_latin;
      
      password_latin = g_convert (password, -1,
				  "ISO-8859-1",
				  "UTF-8",
				  NULL, NULL, NULL);
      password_g = new GooString (password_latin);
      g_free (password_latin);
    } else {
      password_g = new GooString (password);
    }
  }

#ifdef G_OS_WIN32
  wchar_t *filenameW;
  int length;

  length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);

  filenameW = new WCHAR[length];
  if (!filenameW)
      return NULL;

  length = MultiByteToWideChar(CP_UTF8, 0, filename, -1, filenameW, length);

  newDoc = new PDFDoc(filenameW, length, password_g, password_g);
  delete filenameW;
#else
  filename_g = new GooString (filename);
  newDoc = new PDFDoc(filename_g, password_g, password_g);
#endif
  g_free (filename);

  delete password_g;

  return _poppler_document_new_from_pdfdoc (newDoc, error);
}

/**
 * poppler_document_new_from_data:
 * @data: the pdf data contained in a char array
 * @length: the length of #data
 * @password: password to unlock the file with, or %NULL
 * @error: Return location for an error, or %NULL
 * 
 * Creates a new #PopplerDocument.  If %NULL is returned, then @error will be
 * set. Possible errors include those in the #POPPLER_ERROR and #G_FILE_ERROR
 * domains.
 * 
 * Return value: A newly created #PopplerDocument, or %NULL
 **/
PopplerDocument *
poppler_document_new_from_data (char        *data,
                                int          length,
                                const char  *password,
                                GError     **error)
{
  Object obj;
  PDFDoc *newDoc;
  MemStream *str;
  GooString *password_g;

  if (!globalParams) {
    globalParams = new GlobalParams();
  }
  
  // create stream
  obj.initNull();
  str = new MemStream(data, 0, length, &obj);

  password_g = NULL;
  if (password != NULL)
    password_g = new GooString (password);

  newDoc = new PDFDoc(str, password_g, password_g);
  delete password_g;

  return _poppler_document_new_from_pdfdoc (newDoc, error);
}

static gboolean
handle_save_error (int      err_code,
		   GError **error)
{
  switch (err_code)
    {
    case errNone:
      break;
    case errOpenFile:
      g_set_error (error, POPPLER_ERROR,
		   POPPLER_ERROR_OPEN_FILE,
		   "Failed to open file for writing");
      break;
    case errEncrypted:
      g_set_error (error, POPPLER_ERROR,
		   POPPLER_ERROR_ENCRYPTED,
		   "Document is encrypted");
      break;
    default:
      g_set_error (error, POPPLER_ERROR,
		   POPPLER_ERROR_INVALID,
		   "Failed to save document");
    }

  return err_code == errNone;
}

/**
 * poppler_document_save:
 * @document: a #PopplerDocument
 * @uri: uri of file to save
 * @error: return location for an error, or %NULL
 * 
 * Saves @document. Any change made in the document such as 
 * form fields filled by the user will be saved. 
 * If @error is set, %FALSE will be returned. Possible errors
 * include those in the #G_FILE_ERROR domain.
 * 
 * Return value: %TRUE, if the document was successfully saved
 **/
gboolean
poppler_document_save (PopplerDocument  *document,
		       const char       *uri,
		       GError          **error)
{
  char *filename;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), FALSE);

  filename = g_filename_from_uri (uri, NULL, error);
  if (filename != NULL) {
    GooString *fname = new GooString (filename);
    int err_code;
    g_free (filename);

    err_code = document->doc->saveAs (fname);
    retval = handle_save_error (err_code, error);
    delete fname;
  }

  return retval;
}

/**
 * poppler_document_save_a_copy:
 * @document: a #PopplerDocument
 * @uri: uri of file to save
 * @error: return location for an error, or %NULL
 * 
 * Saves a copy of the original @document.
 * Any change made in the document such as 
 * form fields filled by the user will not be saved. 
 * If @error is set, %FALSE will be returned. Possible errors
 * include those in the #G_FILE_ERROR domain.
 * 
 * Return value: %TRUE, if the document was successfully saved
 **/
gboolean
poppler_document_save_a_copy (PopplerDocument  *document,
			      const char       *uri,
			      GError          **error)
{
  char *filename;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), FALSE);

  filename = g_filename_from_uri (uri, NULL, error);
  if (filename != NULL) {
    GooString *fname = new GooString (filename);
    int err_code;
    g_free (filename);

    err_code = document->doc->saveWithoutChangesAs (fname);
    retval = handle_save_error (err_code, error);
    delete fname;
  }

  return retval;
}

static void
poppler_document_finalize (GObject *object)
{
  PopplerDocument *document = POPPLER_DOCUMENT (object);

  poppler_document_layers_free (document);
  delete document->output_dev;
  delete document->doc;
}

/**
 * poppler_document_get_n_pages:
 * @document: A #PopplerDocument
 * 
 * Returns the number of pages in a loaded document.
 * 
 * Return value: Number of pages
 **/
int
poppler_document_get_n_pages (PopplerDocument *document)
{
  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), 0);

  return document->doc->getNumPages();
}

/**
 * poppler_document_get_page:
 * @document: A #PopplerDocument
 * @index: a page index 
 * 
 * Returns the #PopplerPage indexed at @index.  This object is owned by the
 * caller.
 *
 * #PopplerPage<!-- -->s are indexed starting at 0.
 * 
 * Return value: The #PopplerPage at @index
 **/
PopplerPage *
poppler_document_get_page (PopplerDocument  *document,
			   int               index)
{
  Catalog *catalog;
  Page *page;

  g_return_val_if_fail (0 <= index &&
			index < poppler_document_get_n_pages (document),
			NULL);

  catalog = document->doc->getCatalog();
  page = catalog->getPage (index + 1);

  return _poppler_page_new (document, page, index);
}

/**
 * poppler_document_get_page_by_label:
 * @document: A #PopplerDocument
 * @label: a page label
 * 
 * Returns the #PopplerPage reference by @label.  This object is owned by the
 * caller.  @label is a human-readable string representation of the page number,
 * and can be document specific.  Typically, it is a value such as "iii" or "3".
 *
 * By default, "1" refers to the first page.
 * 
 * Return value: The #PopplerPage referenced by @label
 **/
PopplerPage *
poppler_document_get_page_by_label (PopplerDocument  *document,
				    const char       *label)
{
  Catalog *catalog;
  GooString label_g(label);
  int index;

  catalog = document->doc->getCatalog();
  if (!catalog->labelToIndex (&label_g, &index))
    return NULL;

  return poppler_document_get_page (document, index);
}

/**
 * poppler_document_has_attachments:
 * @document: A #PopplerDocument
 * 
 * Returns %TRUE of @document has any attachments.
 * 
 * Return value: %TRUE, if @document has attachments.
 **/
gboolean
poppler_document_has_attachments (PopplerDocument *document)
{
  Catalog *catalog;
  int n_files = 0;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), FALSE);

  catalog = document->doc->getCatalog ();
  if (catalog && catalog->isOk ())
    {
      n_files = catalog->numEmbeddedFiles ();
    }

  return (n_files != 0);
}

/**
 * poppler_document_get_attachments:
 * @document: A #PopplerDocument
 * 
 * Returns a #GList containing #PopplerAttachment<!-- -->s.  These attachments
 * are unowned, and must be unreffed, and the list must be freed with
 * g_list_free().
 * 
 * Return value: a list of available attachments.
 **/
GList *
poppler_document_get_attachments (PopplerDocument *document)
{
  Catalog *catalog;
  int n_files, i;
  GList *retval = NULL;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

  catalog = document->doc->getCatalog ();
  if (catalog == NULL || ! catalog->isOk ())
    return NULL;

  n_files = catalog->numEmbeddedFiles ();
  for (i = 0; i < n_files; i++)
    {
      PopplerAttachment *attachment;
      EmbFile *emb_file;

      emb_file = catalog->embeddedFile (i);
      if (!emb_file->isOk ()) {
        delete emb_file;
	continue;
      }
      attachment = _poppler_attachment_new (document, emb_file);
      delete emb_file;

      retval = g_list_prepend (retval, attachment);
    }
  return g_list_reverse (retval);
}

/**
 * poppler_document_find_dest:
 * @document: A #PopplerDocument
 * @link_name: a named destination
 *
 * Finds named destination @link_name in @document
 *
 * Return value: The #PopplerDest destination or %NULL if
 * @link_name is not a destination. Returned value must
 * be freed with #poppler_dest_free
 **/
PopplerDest *
poppler_document_find_dest (PopplerDocument *document,
			    const gchar     *link_name)
{
	PopplerDest *dest = NULL;
	LinkDest *link_dest = NULL;
	GooString *g_link_name;

	g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (link_name != NULL, NULL);

	g_link_name = new GooString (link_name);

	if (g_link_name) {
		link_dest = document->doc->findDest (g_link_name);
		delete g_link_name;
	}

	if (link_dest) {
		dest = _poppler_dest_new_goto (document, link_dest);
		delete link_dest;
	}

	return dest;
}

char *_poppler_goo_string_to_utf8(GooString *s)
{
  char *result;

  if (s->hasUnicodeMarker()) {
    result = g_convert (s->getCString () + 2,
			s->getLength () - 2,
			"UTF-8", "UTF-16BE", NULL, NULL, NULL);
  } else {
    int len;
    gunichar *ucs4_temp;
    int i;
    
    len = s->getLength ();
    ucs4_temp = g_new (gunichar, len + 1);
    for (i = 0; i < len; ++i) {
      ucs4_temp[i] = pdfDocEncoding[(unsigned char)s->getChar(i)];
    }
    ucs4_temp[i] = 0;

    result = g_ucs4_to_utf8 (ucs4_temp, -1, NULL, NULL, NULL);

    g_free (ucs4_temp);
  }

  return result;
}

static void
info_dict_get_string (Dict *info_dict, const gchar *key, GValue *value)
{
  Object obj;
  GooString *goo_value;
  gchar *result;

  if (!info_dict->lookup ((gchar *)key, &obj)->isString ()) {
    obj.free ();
    return;
  }

  goo_value = obj.getString ();

  result = _poppler_goo_string_to_utf8(goo_value);

  obj.free ();

  g_value_set_string (value, result);

  g_free (result);
}

static void
info_dict_get_date (Dict *info_dict, const gchar *key, GValue *value) 
{
  Object obj;
  time_t result;

  if (!info_dict->lookup ((gchar *)key, &obj)->isString ()) {
    obj.free ();
    return;
  }

  if (_poppler_convert_pdf_date_to_gtime (obj.getString (), &result))
    g_value_set_int (value, result);

  obj.free ();
}

static PopplerPageLayout
convert_page_layout (Catalog::PageLayout pageLayout)
{
  switch (pageLayout)
    {
    case Catalog::pageLayoutSinglePage:
      return POPPLER_PAGE_LAYOUT_SINGLE_PAGE;
    case Catalog::pageLayoutOneColumn:
      return POPPLER_PAGE_LAYOUT_ONE_COLUMN;
    case Catalog::pageLayoutTwoColumnLeft:
      return POPPLER_PAGE_LAYOUT_TWO_COLUMN_LEFT;
    case Catalog::pageLayoutTwoColumnRight:
      return POPPLER_PAGE_LAYOUT_TWO_COLUMN_RIGHT;
    case Catalog::pageLayoutTwoPageLeft:
      return POPPLER_PAGE_LAYOUT_TWO_PAGE_LEFT;
    case Catalog::pageLayoutTwoPageRight:
      return POPPLER_PAGE_LAYOUT_TWO_PAGE_RIGHT;
    case Catalog::pageLayoutNone:
    default:
      return POPPLER_PAGE_LAYOUT_UNSET;
    }
}

static PopplerPageMode
convert_page_mode (Catalog::PageMode pageMode)
{
  switch (pageMode)
    {
    case Catalog::pageModeOutlines:
      return POPPLER_PAGE_MODE_USE_OUTLINES;
    case Catalog::pageModeThumbs:
      return POPPLER_PAGE_MODE_USE_THUMBS;
    case Catalog::pageModeFullScreen:
      return POPPLER_PAGE_MODE_FULL_SCREEN;
    case Catalog::pageModeOC:
      return POPPLER_PAGE_MODE_USE_OC;
    case Catalog::pageModeAttach:
      return POPPLER_PAGE_MODE_USE_ATTACHMENTS;
    case Catalog::pageModeNone:
    default:
      return POPPLER_PAGE_MODE_UNSET;
    }
}

static void
poppler_document_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
  PopplerDocument *document = POPPLER_DOCUMENT (object);
  Object obj;
  Catalog *catalog;
  gchar *str;
  guint flag;

  switch (prop_id)
    {
    case PROP_TITLE:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Title", value);
      obj.free ();
      break;
    case PROP_FORMAT:
      str = g_strndup("PDF-", 15); /* allocates 16 chars, pads with \0s */
      g_ascii_formatd (str + 4, 15 + 1 - 4,
		       "%.2g", document->doc->getPDFMajorVersion () + document->doc->getPDFMinorVersion() / 10.0);
      g_value_take_string (value, str);
      break;
    case PROP_FORMAT_MAJOR:
      g_value_set_uint (value, document->doc->getPDFMajorVersion ());
      break;
    case PROP_FORMAT_MINOR:
      g_value_set_uint (value, document->doc->getPDFMinorVersion());
      break;
    case PROP_AUTHOR:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Author", value);
      obj.free ();
      break;
    case PROP_SUBJECT:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Subject", value);
      obj.free ();
      break;
    case PROP_KEYWORDS:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Keywords", value);
      obj.free ();
      break;
    case PROP_CREATOR:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Creator", value);
      obj.free ();
      break;
    case PROP_PRODUCER:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_string (obj.getDict(), "Producer", value);
      obj.free ();
      break;
    case PROP_CREATION_DATE:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_date (obj.getDict(), "CreationDate", value);
      obj.free ();
      break;
    case PROP_MOD_DATE:
      document->doc->getDocInfo (&obj);
      if (obj.isDict ())
	info_dict_get_date (obj.getDict(), "ModDate", value);
      obj.free ();
	break;
    case PROP_LINEARIZED:
      if (document->doc->isLinearized ()) {	
	  g_value_set_string (value, "Yes");
      }	else {
	  g_value_set_string (value, "No");
      }
      break;
    case PROP_PAGE_LAYOUT:
      catalog = document->doc->getCatalog ();
      if (catalog && catalog->isOk ())
	{
	  PopplerPageLayout page_layout = convert_page_layout (catalog->getPageLayout ());
	  g_value_set_enum (value, page_layout);
	}
      break;
    case PROP_PAGE_MODE:
      catalog = document->doc->getCatalog ();
      if (catalog && catalog->isOk ())
	{
	  PopplerPageMode page_mode = convert_page_mode (catalog->getPageMode ());
	  g_value_set_enum (value, page_mode);
	}
      break;
    case PROP_VIEWER_PREFERENCES:
      /* FIXME: write... */
      g_value_set_flags (value, POPPLER_VIEWER_PREFERENCES_UNSET);
      break;
    case PROP_PERMISSIONS:
      flag = 0;
      if (document->doc->okToPrint ())
	flag |= POPPLER_PERMISSIONS_OK_TO_PRINT;
      if (document->doc->okToChange ())
	flag |= POPPLER_PERMISSIONS_OK_TO_MODIFY;
      if (document->doc->okToCopy ())
	flag |= POPPLER_PERMISSIONS_OK_TO_COPY;
      if (document->doc->okToAddNotes ())
	flag |= POPPLER_PERMISSIONS_OK_TO_ADD_NOTES;
      if (document->doc->okToFillForm ())
        flag |= POPPLER_PERMISSIONS_OK_TO_FILL_FORM;
      g_value_set_flags (value, flag);
      break;
    case PROP_METADATA:
      catalog = document->doc->getCatalog ();
      if (catalog && catalog->isOk ())
	{
	  GooString *s = catalog->readMetadata ();
	  if ( s != NULL ) {
	  	g_value_set_string (value, s->getCString());
	  	delete s;
	  }
	}
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
poppler_document_class_init (PopplerDocumentClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_document_finalize;
  gobject_class->get_property = poppler_document_get_property;

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_TITLE,
	   g_param_spec_string ("title",
				"Document Title",
				"The title of the document",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_FORMAT,
	   g_param_spec_string ("format",
				"PDF Format",
				"The PDF version of the document",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_FORMAT_MAJOR,
	   g_param_spec_uint ("format-major",
			      "PDF Format Major",
			      "The PDF major version number of the document",
			      0, G_MAXUINT, 1,
			      G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_FORMAT_MINOR,
	   g_param_spec_uint ("format-minor",
			      "PDF Format Minor",
			      "The PDF minor version number of the document",
			      0, G_MAXUINT, 0,
			      G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_AUTHOR,
	   g_param_spec_string ("author",
				"Author",
				"The author of the document",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_SUBJECT,
	   g_param_spec_string ("subject",
				"Subject",
				"Subjects the document touches",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_KEYWORDS,
	   g_param_spec_string ("keywords",
				"Keywords",
				"Keywords",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_CREATOR,
	   g_param_spec_string ("creator",
				"Creator",
				"The software that created the document",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	  PROP_PRODUCER,
	   g_param_spec_string ("producer",
				"Producer",
				"The software that converted the document",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_CREATION_DATE,
	   g_param_spec_int ("creation-date",
				"Creation Date",
				"The date and time the document was created",
				0, G_MAXINT, 0,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_MOD_DATE,
	   g_param_spec_int ("mod-date",
				"Modification Date",
				"The date and time the document was modified",
				0, G_MAXINT, 0,
				G_PARAM_READABLE));
				
  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_LINEARIZED,
	   g_param_spec_string ("linearized",
				"Fast Web View Enabled",
				"Is the document optimized for web viewing?",
				NULL,
				G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_PAGE_LAYOUT,
	   g_param_spec_enum ("page-layout",
			      "Page Layout",
			      "Initial Page Layout",
			      POPPLER_TYPE_PAGE_LAYOUT,
			      POPPLER_PAGE_LAYOUT_UNSET,
			      G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_PAGE_MODE,
	   g_param_spec_enum ("page-mode",
			      "Page Mode",
			      "Page Mode",
			      POPPLER_TYPE_PAGE_MODE,
			      POPPLER_PAGE_MODE_UNSET,
			      G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_VIEWER_PREFERENCES,
	   g_param_spec_flags ("viewer-preferences",
			       "Viewer Preferences",
			       "Viewer Preferences",
			       POPPLER_TYPE_VIEWER_PREFERENCES,
			       POPPLER_VIEWER_PREFERENCES_UNSET,
			       G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_PERMISSIONS,
	   g_param_spec_flags ("permissions",
			       "Permissions",
			       "Permissions",
			       POPPLER_TYPE_PERMISSIONS,
			       POPPLER_PERMISSIONS_FULL,
			       G_PARAM_READABLE));

  g_object_class_install_property
	  (G_OBJECT_CLASS (klass),
	   PROP_METADATA,
	   g_param_spec_string ("metadata",
				"XML Metadata",
				"Embedded XML metadata",
				NULL,
				G_PARAM_READABLE));
}

static void
poppler_document_init (PopplerDocument *document)
{
}

/* PopplerIndexIter: For determining the index of a tree */
struct _PopplerIndexIter
{
	PopplerDocument *document;
	GooList *items;
	int index;
};


POPPLER_DEFINE_BOXED_TYPE (PopplerIndexIter, poppler_index_iter,
			   poppler_index_iter_copy,
			   poppler_index_iter_free)

/**
 * poppler_index_iter_copy:
 * @iter: a #PopplerIndexIter
 * 
 * Creates a new #PopplerIndexIter as a copy of @iter.  This must be freed with
 * poppler_index_iter_free().
 * 
 * Return value: a new #PopplerIndexIter
 **/
PopplerIndexIter *
poppler_index_iter_copy (PopplerIndexIter *iter)
{
	PopplerIndexIter *new_iter;

	g_return_val_if_fail (iter != NULL, NULL);

	new_iter = g_new0 (PopplerIndexIter, 1);
	*new_iter = *iter;
	new_iter->document = (PopplerDocument *) g_object_ref (new_iter->document);

	return new_iter;
}

/**
 * poppler_index_iter_new:
 * @document: a #PopplerDocument
 * 
 * Returns the root #PopplerIndexIter for @document, or %NULL.  This must be
 * freed with poppler_index_iter_free().
 *
 * Certain documents have an index associated with them.  This index can be used
 * to help the user navigate the document, and is similar to a table of
 * contents.  Each node in the index will contain a #PopplerAction that can be
 * displayed to the user &mdash; typically a #POPPLER_ACTION_GOTO_DEST or a
 * #POPPLER_ACTION_URI<!-- -->.
 *
 * Here is a simple example of some code that walks the full index:
 *
 * <informalexample><programlisting>
 * static void
 * walk_index (PopplerIndexIter *iter)
 * {
 *   do
 *     {
 *       /<!-- -->* Get the the action and do something with it *<!-- -->/
 *       PopplerIndexIter *child = poppler_index_iter_get_child (iter);
 *       if (child)
 *         walk_index (child);
 *       poppler_index_iter_free (child);
 *     }
 *   while (poppler_index_iter_next (iter));
 * }
 * ...
 * {
 *   iter = poppler_index_iter_new (document);
 *   walk_index (iter);
 *   poppler_index_iter_free (iter);
 * }
 *</programlisting></informalexample>
 *
 * Return value: a new #PopplerIndexIter
 **/
PopplerIndexIter *
poppler_index_iter_new (PopplerDocument *document)
{
	PopplerIndexIter *iter;
	Outline *outline;
	GooList *items;

	outline = document->doc->getOutline();
	if (outline == NULL)
		return NULL;

	items = outline->getItems();
	if (items == NULL)
		return NULL;

	iter = g_new0 (PopplerIndexIter, 1);
	iter->document = (PopplerDocument *) g_object_ref (document);
	iter->items = items;
	iter->index = 0;

	return iter;
}

/**
 * poppler_index_iter_get_child:
 * @parent: a #PopplerIndexIter
 * 
 * Returns a newly created child of @parent, or %NULL if the iter has no child.
 * See poppler_index_iter_new() for more information on this function.
 * 
 * Return value: a new #PopplerIndexIter
 **/
PopplerIndexIter *
poppler_index_iter_get_child (PopplerIndexIter *parent)
{
	PopplerIndexIter *child;
	OutlineItem *item;

	g_return_val_if_fail (parent != NULL, NULL);
	
	item = (OutlineItem *)parent->items->get (parent->index);
	item->open ();
	if (! (item->hasKids() && item->getKids()) )
		return NULL;

	child = g_new0 (PopplerIndexIter, 1);
	child->document = (PopplerDocument *)g_object_ref (parent->document);
	child->items = item->getKids ();

	g_assert (child->items);

	return child;
}

static gchar *
unicode_to_char (Unicode *unicode,
		 int      len)
{
	static UnicodeMap *uMap = NULL;
	if (uMap == NULL) {
		GooString *enc = new GooString("UTF-8");
		uMap = globalParams->getUnicodeMap(enc);
		uMap->incRefCnt ();
		delete enc;
	}
		
	GooString gstr;
	gchar buf[8]; /* 8 is enough for mapping an unicode char to a string */
	int i, n;

	for (i = 0; i < len; ++i) {
		n = uMap->mapUnicode(unicode[i], buf, sizeof(buf));
		gstr.append(buf, n);
	}

	return g_strdup (gstr.getCString ());
}

/**
 * poppler_index_iter_is_open:
 * @iter: a #PopplerIndexIter
 * 
 * Returns whether this node should be expanded by default to the user.  The
 * document can provide a hint as to how the document's index should be expanded
 * initially.
 * 
 * Return value: %TRUE, if the document wants @iter to be expanded
 **/
gboolean
poppler_index_iter_is_open (PopplerIndexIter *iter)
{
	OutlineItem *item;

	item = (OutlineItem *)iter->items->get (iter->index);

	return item->isOpen();
}

/**
 * poppler_index_iter_get_action:
 * @iter: a #PopplerIndexIter
 * 
 * Returns the #PopplerAction associated with @iter.  It must be freed with
 * poppler_action_free().
 * 
 * Return value: a new #PopplerAction
 **/
PopplerAction *
poppler_index_iter_get_action (PopplerIndexIter  *iter)
{
	OutlineItem *item;
	LinkAction *link_action;
	PopplerAction *action;
	gchar *title;

	g_return_val_if_fail (iter != NULL, NULL);

	item = (OutlineItem *)iter->items->get (iter->index);
	link_action = item->getAction ();

	title = unicode_to_char (item->getTitle(),
				 item->getTitleLength ());

	action = _poppler_action_new (iter->document, link_action, title);
	g_free (title);

	return action;
}

/**
 * poppler_index_iter_next:
 * @iter: a #PopplerIndexIter
 * 
 * Sets @iter to point to the next action at the current level, if valid.  See
 * poppler_index_iter_new() for more information.
 * 
 * Return value: %TRUE, if @iter was set to the next action
 **/
gboolean
poppler_index_iter_next (PopplerIndexIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	iter->index++;
	if (iter->index >= iter->items->getLength())
		return FALSE;

	return TRUE;
}

/**
 * poppler_index_iter_free:
 * @iter: a #PopplerIndexIter
 * 
 * Frees @iter.
 **/
void
poppler_index_iter_free (PopplerIndexIter *iter)
{
	if (iter == NULL)
		return;

	g_object_unref (iter->document);
	g_free (iter);
	
}

struct _PopplerFontsIter
{
	GooList *items;
	int index;
};

POPPLER_DEFINE_BOXED_TYPE (PopplerFontsIter, poppler_fonts_iter,
			   poppler_fonts_iter_copy,
			   poppler_fonts_iter_free)

const char *
poppler_fonts_iter_get_full_name (PopplerFontsIter *iter)
{
	GooString *name;
	FontInfo *info;

	info = (FontInfo *)iter->items->get (iter->index);

	name = info->getName();
	if (name != NULL) {
		return info->getName()->getCString();
	} else {
		return NULL;
	}
}

const char *
poppler_fonts_iter_get_name (PopplerFontsIter *iter)
{
	FontInfo *info;
	const char *name;

	name = poppler_fonts_iter_get_full_name (iter);
	info = (FontInfo *)iter->items->get (iter->index);

	if (info->getSubset() && name) {
		while (*name && *name != '+')
			name++;

		if (*name)
			name++;
	}

	return name;
}

const char *
poppler_fonts_iter_get_file_name (PopplerFontsIter *iter)
{
	GooString *file;
	FontInfo *info;

	info = (FontInfo *)iter->items->get (iter->index);

	file = info->getFile();
	if (file != NULL) {
		return file->getCString();
	} else {
		return NULL;
	}
}

PopplerFontType
poppler_fonts_iter_get_font_type (PopplerFontsIter *iter)
{
	FontInfo *info;

	g_return_val_if_fail (iter != NULL, POPPLER_FONT_TYPE_UNKNOWN);

	info = (FontInfo *)iter->items->get (iter->index);

	return (PopplerFontType)info->getType ();
}

gboolean
poppler_fonts_iter_is_embedded (PopplerFontsIter *iter)
{
	FontInfo *info;

	info = (FontInfo *)iter->items->get (iter->index);

	return info->getEmbedded();
}

gboolean
poppler_fonts_iter_is_subset (PopplerFontsIter *iter)
{
	FontInfo *info;

	info = (FontInfo *)iter->items->get (iter->index);

	return info->getSubset();
}

gboolean
poppler_fonts_iter_next (PopplerFontsIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	iter->index++;
	if (iter->index >= iter->items->getLength())
		return FALSE;

	return TRUE;
}

PopplerFontsIter *
poppler_fonts_iter_copy (PopplerFontsIter *iter)
{
	PopplerFontsIter *new_iter;

	g_return_val_if_fail (iter != NULL, NULL);

	new_iter = g_new0 (PopplerFontsIter, 1);
	*new_iter = *iter;

	new_iter->items = new GooList ();
	for (int i = 0; i < iter->items->getLength(); i++) {
		FontInfo *info = (FontInfo *)iter->items->get(i);
		new_iter->items->append (new FontInfo (*info));
	}

	return new_iter;
}

void
poppler_fonts_iter_free (PopplerFontsIter *iter)
{
	if (iter == NULL)
		return;

	deleteGooList (iter->items, FontInfo);

	g_free (iter);
}

static PopplerFontsIter *
poppler_fonts_iter_new (GooList *items)
{
	PopplerFontsIter *iter;

	iter = g_new0 (PopplerFontsIter, 1);
	iter->items = items;
	iter->index = 0;

	return iter;
}


typedef struct _PopplerFontInfoClass PopplerFontInfoClass;
struct _PopplerFontInfoClass
{
        GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerFontInfo, poppler_font_info, G_TYPE_OBJECT)

static void poppler_font_info_finalize (GObject *object);


static void
poppler_font_info_class_init (PopplerFontInfoClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        gobject_class->finalize = poppler_font_info_finalize;
}

static void
poppler_font_info_init (PopplerFontInfo *font_info)
{
        font_info->document = NULL;
        font_info->scanner = NULL;
}

static void
poppler_font_info_finalize (GObject *object)
{
        PopplerFontInfo *font_info = POPPLER_FONT_INFO (object);

        delete font_info->scanner;
        g_object_unref (font_info->document);
}

PopplerFontInfo *
poppler_font_info_new (PopplerDocument *document)
{
	PopplerFontInfo *font_info;

	g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

	font_info = (PopplerFontInfo *) g_object_new (POPPLER_TYPE_FONT_INFO,
						      NULL);
	font_info->document = (PopplerDocument *) g_object_ref (document);
	font_info->scanner = new FontInfoScanner(document->doc);

	return font_info;
}

gboolean
poppler_font_info_scan (PopplerFontInfo   *font_info,
			int                n_pages,
			PopplerFontsIter **iter)
{
	GooList *items;

	g_return_val_if_fail (iter != NULL, FALSE);

	items = font_info->scanner->scan(n_pages);

	if (items == NULL) {
		*iter = NULL;
	} else if (items->getLength() == 0) {
		*iter = NULL;
		delete items;
	} else {
		*iter = poppler_fonts_iter_new(items);
	}
	
	return (items != NULL);
}

/* For backward compatibility */
void
poppler_font_info_free (PopplerFontInfo *font_info)
{
	g_return_if_fail (font_info != NULL);

	g_object_unref (font_info);
}


/* Optional content (layers) */
static Layer *
layer_new (OptionalContentGroup *oc)
{
  Layer *layer;

  layer = g_new0 (Layer, 1);
  layer->oc = oc;

  return layer;
}

static void
layer_free (Layer *layer)
{
  if (!layer)
    return;

  if (layer->kids) {
	  g_list_foreach (layer->kids, (GFunc)layer_free, NULL);
	  g_list_free (layer->kids);
  }

  if (layer->label) {
	  g_free (layer->label);
  }

  g_free (layer);
}

static GList *
get_optional_content_rbgroups (OCGs *ocg)
{
  Array *rb;
  GList *groups = NULL;
  
  rb = ocg->getRBGroupsArray ();
  
  if (rb) {
    int i, j;

    for (i = 0; i < rb->getLength (); ++i) {
      Object obj;
      Array *rb_array;
      GList *group = NULL;

      rb->get (i, &obj);
      if (!obj.isArray ()) {
        obj.free ();
	continue;
      }

      rb_array = obj.getArray ();
      for (j = 0; j < rb_array->getLength (); ++j) {
        Object ref;
	OptionalContentGroup *oc;

	rb_array->getNF (j, &ref);
	if (!ref.isRef ()) {
	  ref.free ();
	  continue;
	}

	oc = ocg->findOcgByRef (ref.getRef ());
	group = g_list_prepend (group, oc);
	ref.free ();
      }
      obj.free ();

      groups = g_list_prepend (groups, group);
    }
  }

  return groups;
}

static GList *
poppler_document_get_layer_rbgroup (PopplerDocument *document,
				    Layer           *layer)
{
  GList *l;

  for (l = document->layers_rbgroups; l && l->data; l = g_list_next (l)) {
    GList *group = (GList *)l->data;

    if (g_list_find (group, layer->oc))
      return group;
  }

  return NULL;
}

static GList *
get_optional_content_items_sorted (OCGs *ocg, Layer *parent, Array *order)
{
  GList *items = NULL;
  Layer *last_item = parent;
  int i;

  for (i = 0; i < order->getLength (); ++i) {
    Object orderItem;
      
    order->get (i, &orderItem);

    if (orderItem.isDict ()) {
      Object ref;
      
      order->getNF (i, &ref);
      if (ref.isRef ()) {
        OptionalContentGroup *oc = ocg->findOcgByRef (ref.getRef ());
	Layer *layer = layer_new (oc);

	items = g_list_prepend (items, layer);
	last_item = layer;
      }
      ref.free ();
    } else if (orderItem.isArray () && orderItem.arrayGetLength () > 0) {
      if (!last_item) {
        last_item = layer_new (NULL);
	items = g_list_prepend (items, last_item);
      }
      last_item->kids = get_optional_content_items_sorted (ocg, last_item, orderItem.getArray ());
    } else if (orderItem.isString ()) {
      last_item->label = _poppler_goo_string_to_utf8 (orderItem.getString ());
    }
    orderItem.free ();
  }
  
  return g_list_reverse (items);
}
		
static GList *
get_optional_content_items (OCGs *ocg)
{
  Array *order;
  GList *items = NULL;
  
  order = ocg->getOrderArray ();

  if (order) {
    items = get_optional_content_items_sorted (ocg, NULL, order);
  } else {
    GooList *ocgs;
    int i;

    ocgs = ocg->getOCGs ();
    
    for (i = 0; i < ocgs->getLength (); ++i) {
      OptionalContentGroup *oc = (OptionalContentGroup *) ocgs->get (i);
      Layer *layer = layer_new (oc);
      
      items = g_list_prepend (items, layer);
    }
    
    items = g_list_reverse (items);
  }

  return items;
}

static GList *
poppler_document_get_layers (PopplerDocument *document)
{
  if (!document->layers) {
    Catalog *catalog = document->doc->getCatalog ();
    OCGs *ocg = catalog->getOptContentConfig ();

    if (!ocg)
      return NULL;
    
    document->layers = get_optional_content_items (ocg);
    document->layers_rbgroups = get_optional_content_rbgroups (ocg);
  }

  return document->layers;
}

static void
poppler_document_layers_free (PopplerDocument *document)
{
  if (!document->layers)
	  return;

  g_list_foreach (document->layers, (GFunc)layer_free, NULL);
  g_list_free (document->layers);

  g_list_foreach (document->layers_rbgroups, (GFunc)g_list_free, NULL);
  g_list_free (document->layers_rbgroups);

  document->layers = NULL;
  document->layers_rbgroups = NULL;
}

/* PopplerLayersIter */
struct _PopplerLayersIter {
  PopplerDocument *document;
  GList *items;
  int index;
};

POPPLER_DEFINE_BOXED_TYPE (PopplerLayersIter, poppler_layers_iter,
			   poppler_layers_iter_copy,
			   poppler_layers_iter_free)

/**
 * poppler_layers_iter_copy:
 * @iter: a #PopplerLayersIter
 * 
 * Creates a new #PopplerLayersIter as a copy of @iter.  This must be freed with
 * poppler_layers_iter_free().
 * 
 * Return value: a new #PopplerLayersIter
 **/
PopplerLayersIter *
poppler_layers_iter_copy (PopplerLayersIter *iter)
{
  PopplerLayersIter *new_iter;

  g_return_val_if_fail (iter != NULL, NULL);
  
  new_iter = g_new0 (PopplerLayersIter, 1);
  *new_iter = *iter;
  new_iter->document = (PopplerDocument *) g_object_ref (new_iter->document);
  
  return new_iter;
}

/**
 * poppler_layers_iter_free:
 * @iter: a #PopplerLayersIter
 * 
 * Frees @iter.
 **/
void
poppler_layers_iter_free (PopplerLayersIter *iter)
{
  if (iter == NULL)
    return;

  g_object_unref (iter->document);
  g_free (iter);
}

/**
 * poppler_layers_iter_new:
 **/
PopplerLayersIter *
poppler_layers_iter_new (PopplerDocument *document)
{
  PopplerLayersIter *iter;
  GList *items;

  items = poppler_document_get_layers (document);

  if (!items)
    return NULL;

  iter = g_new0 (PopplerLayersIter, 1);
  iter->document = (PopplerDocument *)g_object_ref (document);
  iter->items = items;

  return iter;
}

/**
 * poppler_layers_iter_get_child:
 * @parent: a #PopplerLayersIter
 * 
 * Returns a newly created child of @parent, or %NULL if the iter has no child.
 * See poppler_layers_iter_new() for more information on this function.
 * 
 * Return value: a new #PopplerLayersIter, or %NULL
 **/
PopplerLayersIter *
poppler_layers_iter_get_child (PopplerLayersIter *parent)
{
  PopplerLayersIter *child;
  Layer *layer;

  g_return_val_if_fail (parent != NULL, NULL);
	
  layer = (Layer *) g_list_nth_data (parent->items, parent->index);
  if (!layer || !layer->kids)
    return NULL;

  child = g_new0 (PopplerLayersIter, 1);
  child->document = (PopplerDocument *)g_object_ref (parent->document);
  child->items = layer->kids;

  g_assert (child->items);

  return child;
}

/**
 * poppler_layers_iter_get_title:
 * @iter: a #PopplerLayersIter
 * 
 * Returns the title associated with @iter.  It must be freed with
 * g_free().
 * 
 * Return value: a new string containing the @iter's title or %NULL if @iter doesn't have a title.
 * The returned string should be freed with g_free() when no longer needed. 
 **/
gchar *
poppler_layers_iter_get_title (PopplerLayersIter *iter)
{
  Layer *layer;
  
  g_return_val_if_fail (iter != NULL, NULL);
  
  layer = (Layer *)g_list_nth_data (iter->items, iter->index);

  return layer->label ? g_strdup (layer->label) : NULL;
}

/**
 * poppler_layers_iter_get_layer:
 * @iter: a #PopplerLayersIter
 * 
 * Returns the #PopplerLayer associated with @iter.  It must be freed with
 * poppler_layer_free().
 * 
 * Return value: a new #PopplerLayer, or %NULL if there isn't any layer associated with @iter
 **/
PopplerLayer *
poppler_layers_iter_get_layer (PopplerLayersIter *iter)
{
  Layer *layer;
  PopplerLayer *poppler_layer = NULL;
  
  g_return_val_if_fail (iter != NULL, NULL);
  
  layer = (Layer *)g_list_nth_data (iter->items, iter->index);
  if (layer->oc) {
    GList *rb_group = NULL;

    rb_group = poppler_document_get_layer_rbgroup (iter->document, layer);
    poppler_layer = _poppler_layer_new (iter->document, layer, rb_group);
  }
  
  return poppler_layer;
}

/**
 * poppler_layers_iter_next:
 * @iter: a #PopplerLayersIter
 * 
 * Sets @iter to point to the next action at the current level, if valid.  See
 * poppler_layers_iter_new() for more information.
 * 
 * Return value: %TRUE, if @iter was set to the next action
 **/
gboolean
poppler_layers_iter_next (PopplerLayersIter *iter)
{
  g_return_val_if_fail (iter != NULL, FALSE);

  iter->index++;
  if (iter->index >= (gint)g_list_length (iter->items))
    return FALSE;
  
  return TRUE;
}

typedef struct _PopplerPSFileClass PopplerPSFileClass;
struct _PopplerPSFileClass
{
        GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerPSFile, poppler_ps_file, G_TYPE_OBJECT)

static void poppler_ps_file_finalize (GObject *object);


static void
poppler_ps_file_class_init (PopplerPSFileClass *klass)
{
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

        gobject_class->finalize = poppler_ps_file_finalize;
}

static void
poppler_ps_file_init (PopplerPSFile *ps_file)
{
        ps_file->out = NULL;
        ps_file->paper_width = -1;
        ps_file->paper_height = -1;
        ps_file->duplex = FALSE;
}

static void
poppler_ps_file_finalize (GObject *object)
{
        PopplerPSFile *ps_file = POPPLER_PS_FILE (object);

        delete ps_file->out;
        g_object_unref (ps_file->document);
        g_free (ps_file->filename);
}

/**
 * poppler_ps_file_new:
 * @document: a #PopplerDocument
 * @filename: the path of the output filename
 * @first_page: the first page to print
 * @n_pages: the number of pages to print
 * 
 * Create a new postscript file to render to
 * 
 * Return value: a PopplerPSFile 
 **/
PopplerPSFile *
poppler_ps_file_new (PopplerDocument *document, const char *filename,
		     int first_page, int n_pages)
{
	PopplerPSFile *ps_file;

	g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);
	g_return_val_if_fail (filename != NULL, NULL);
	g_return_val_if_fail (n_pages > 0, NULL);

        ps_file = (PopplerPSFile *) g_object_new (POPPLER_TYPE_PS_FILE, NULL);
	ps_file->document = (PopplerDocument *) g_object_ref (document);
        ps_file->filename = g_strdup (filename);
        ps_file->first_page = first_page + 1;
        ps_file->last_page = first_page + 1 + n_pages - 1;

	return ps_file;
}

/**
 * poppler_ps_file_set_paper_size:
 * @ps_file: a PopplerPSFile which was not yet printed to.
 * @width: the paper width in 1/72 inch
 * @height: the paper height in 1/72 inch
 *
 * Set the output paper size. These values will end up in the
 * DocumentMedia, the BoundingBox DSC comments and other places in the
 * generated PostScript.
 *
 **/
void
poppler_ps_file_set_paper_size (PopplerPSFile *ps_file,
                                double width, double height)
{
        g_return_if_fail (ps_file->out == NULL);
        
        ps_file->paper_width = width;
        ps_file->paper_height = height;
}

/**
 * poppler_ps_file_set_duplex:
 * @ps_file: a PopplerPSFile which was not yet printed to
 * @duplex: whether to force duplex printing (on printers which support this)
 *
 * Enable or disable Duplex printing. 
 *
 **/
void
poppler_ps_file_set_duplex (PopplerPSFile *ps_file, gboolean duplex)
{
        g_return_if_fail (ps_file->out == NULL);

        ps_file->duplex = duplex;
}

/**
 * poppler_ps_file_free:
 * @ps_file: a PopplerPSFile
 * 
 * Frees @ps_file
 * 
 **/
void
poppler_ps_file_free (PopplerPSFile *ps_file)
{
	g_return_if_fail (ps_file != NULL);
        g_object_unref (ps_file);
}

/**
 * poppler_document_get_form_field:
 * @document: a #PopplerDocument
 * @id: an id of a #PopplerFormField
 *
 * Returns the #PopplerFormField for the given @id. It must be freed with
 * g_object_unref()
 *
 * Return value: a new #PopplerFormField or NULL if not found
 **/
PopplerFormField *
poppler_document_get_form_field (PopplerDocument *document,
				 gint             id)
{
  Catalog *catalog = document->doc->getCatalog();
  unsigned pageNum;
  unsigned fieldNum;
  FormPageWidgets *widgets;
  FormWidget *field;

  FormWidget::decodeID (id, &pageNum, &fieldNum);
  
  widgets = catalog->getPage (pageNum)->getPageWidgets ();
  if (!widgets)
    return NULL;
  
  field = widgets->getWidget (fieldNum);
  if (field)
    return _poppler_form_field_new (document, field);

  return NULL;
}

gboolean
_poppler_convert_pdf_date_to_gtime (GooString *date,
				    time_t    *gdate) 
{
  gchar *date_string;
  gboolean retval;

  if (date->hasUnicodeMarker()) {
    date_string = g_convert (date->getCString () + 2,
			     date->getLength () - 2,
			     "UTF-8", "UTF-16BE", NULL, NULL, NULL);		
  } else {
    date_string = g_strndup (date->getCString (), date->getLength ());
  }

  retval = poppler_date_parse (date_string, gdate);
  g_free (date_string);

  return retval;
}
