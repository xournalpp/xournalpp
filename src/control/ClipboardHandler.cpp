#include "ClipboardHandler.h"
#include "Control.h"
#include "../util/pixbuf-utils.h"
#include <cairo-svg.h>

ClipboardHandler::ClipboardHandler(ClipboardListener * listener, GtkWidget * widget) {
	this->listener = listener;
	this->clipboard = gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);
	this->containsText = false;
	this->selection = NULL;

	this->hanlderId = g_signal_connect(this->clipboard, "owner-change", G_CALLBACK(&ownerChangedCallback), this);

	this->listener->clipboardCutCopyEnabled(false);

	GdkDisplay * display = gtk_clipboard_get_display(clipboard);

	if (gdk_display_supports_selection_notification(display)) {
		gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
				(GtkClipboardReceivedFunc) receivedClipboardContents, this);
	} else {
		// XFIXES extension not available, make Paste always sensitive
		this->listener->clipboardPasteEnabled(true);
	}
}

ClipboardHandler::~ClipboardHandler() {
	g_signal_handler_disconnect(this->clipboard, this->hanlderId);
}

void ClipboardHandler::paste() {
	gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("UTF8_STRING"),
			(GtkClipboardReceivedFunc) pasteClipboardContents, this);
}

void ClipboardHandler::cut() {
	this->copy();
}

gint ElementCompareFunc(Element * a, Element * b) {
	if (a->getY() == b->getY()) {
		return a->getX() - b->getX();
	}
	return a->getY() - b->getY();
}

static GdkAtom atomSvg1 = gdk_atom_intern_static_string("image/svg");
static GdkAtom atomSvg2 = gdk_atom_intern_static_string("image/svg+xml");

// The contents of the clipboard
class ClipboardContents {
public:
	ClipboardContents(String text, GdkPixbuf * image, String svg) {
		this->text = text;
		this->image = image;
		this->svg = svg;
	}

	~ClipboardContents() {
		gdk_pixbuf_unref(this->image);
	}

public:
	static void getFunction(GtkClipboard *clipboard, GtkSelectionData *selection, guint info,
			ClipboardContents * contents) {

		if (selection->target == gdk_atom_intern_static_string("UTF8_STRING")) {
			gtk_selection_data_set_text(selection, contents->text.c_str(), -1);
			// TODO: debug output
			printf("get Text\n");
		} else if (selection->target == gdk_atom_intern_static_string("image/png") || selection->target
				== gdk_atom_intern_static_string("image/jpeg") || selection->target == gdk_atom_intern_static_string(
				"image/gif")) {
			gtk_selection_data_set_pixbuf(selection, contents->image);
			// TODO: debug output
			printf("get image\n");
		} else if (atomSvg1 == selection->target || atomSvg2 == selection->target) {
			// TODO: debug output
			printf("get SVG\n");
			gtk_selection_data_set(selection, selection->target, 8, (guchar *)contents->svg.c_str(), contents->svg.length());
		}

		char * target = gdk_atom_name(selection->target);
		printf("target: %s\n", target);
		g_free(target);

	}

	static void clearFunction(GtkClipboard *clipboard, ClipboardContents * contents) {
		delete contents;
	}

private:
	String text;
	GdkPixbuf * image;
	String svg;
};

static cairo_status_t svgWriteFunction(GString * string, const unsigned char *data, unsigned int length) {
	g_string_append_len(string, (const gchar *) data, length);
	return CAIRO_STATUS_SUCCESS;
}

void ClipboardHandler::copy() {
	if (!this->selection) {
		return;
	}

	/////////////////////////////////////////////////////////////////
	// prepare text contents
	/////////////////////////////////////////////////////////////////

	GList * textElements = NULL;

	for (GList * l = this->selection->getElements(); l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (e->getType() == ELEMENT_TEXT) {
			textElements = g_list_insert_sorted(textElements, e, (GCompareFunc) ElementCompareFunc);
		}
	}

	String text = "";
	for (GList * l = textElements; l != NULL; l = l->next) {
		Text * e = (Text *) l->data;
		if (text != "") {
			text += "\n";
		}
		text += e->getText();
	}
	g_list_free(textElements);

	/////////////////////////////////////////////////////////////////
	// prepare image contents: PNG
	/////////////////////////////////////////////////////////////////

	DocumentView view;

	double dpiFactor = 1.0 / 72.0 * 300.0;

	cairo_surface_t * surfacePng = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, selection->getWidth() * dpiFactor,
			selection->getHeight() * dpiFactor);
	cairo_t * crPng = cairo_create(surfacePng);
	cairo_scale(crPng, dpiFactor, dpiFactor);

	cairo_translate(crPng, -selection->getX(), -selection->getY());
	view.drawSelection(crPng, this->selection);

	cairo_destroy(crPng);

	GdkPixbuf * image = f_pixbuf_from_cairo_surface(surfacePng);

	cairo_surface_destroy(surfacePng);

	/////////////////////////////////////////////////////////////////
	// prepare image contents: SVG
	/////////////////////////////////////////////////////////////////

	GString * svgString = g_string_sized_new(1048576); // 1MB

	cairo_surface_t * surfaceSVG = cairo_svg_surface_create_for_stream((cairo_write_func_t) svgWriteFunction,
			svgString, selection->getWidth(), selection->getHeight());
	cairo_t * crSVG = cairo_create(surfaceSVG);

	view.drawSelection(crSVG, this->selection);

	cairo_surface_destroy(surfaceSVG);
	cairo_destroy(crSVG);

	/////////////////////////////////////////////////////////////////
	// copy to clipboard
	/////////////////////////////////////////////////////////////////

	GtkTargetList * list = gtk_target_list_new(NULL, 0);
	GtkTargetEntry *targets;
	int n_targets;
	int len = text.length();

	// if we have text elements...
	if (!text.isEmpty()) {
		gtk_target_list_add_text_targets(list, 0);
	}
	// we always copy an image to clipboard
	gtk_target_list_add_image_targets(list, 0, TRUE);
	gtk_target_list_add(list, atomSvg1, 0, 0);
	gtk_target_list_add(list, atomSvg2, 0, 0);

	targets = gtk_target_table_new_from_list(list, &n_targets);

	ClipboardContents * contents = new ClipboardContents(text, image, svgString->str);

	gtk_clipboard_set_with_data(clipboard, targets, n_targets, (GtkClipboardGetFunc) ClipboardContents::getFunction,
			(GtkClipboardClearFunc) ClipboardContents::clearFunction, contents);
	gtk_clipboard_set_can_store(clipboard, NULL, 0);

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

	g_string_free(svgString, true);
}

void ClipboardHandler::setSelection(EditSelection * selection) {
	this->selection = selection;

	this->listener->clipboardCutCopyEnabled(selection != NULL);
}

void ClipboardHandler::setCopyPasteEnabled(bool enabled) {
	if (enabled) {
		listener->clipboardCutCopyEnabled(true);
	} else if (!selection) {
		listener->clipboardCutCopyEnabled(false);
	}
}

void ClipboardHandler::ownerChangedCallback(GtkClipboard *clip, GdkEvent *event, ClipboardHandler * handler) {
	if (event->type == GDK_OWNER_CHANGE) {
		handler->clipboardUpdated(event->owner_change.selection);
	}
}

void ClipboardHandler::clipboardUpdated(GdkAtom atom) {
	if (this->containsText) {
		gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
				(GtkClipboardReceivedFunc) receivedClipboardContents, this);

	}
}

void ClipboardHandler::pasteClipboardContents(GtkClipboard *clipboard, GtkSelectionData *selectionData,
		ClipboardHandler * handler) {

	guchar * text = gtk_selection_data_get_text(selectionData);
	if (text != NULL) {
		handler->listener->clipboardPasteText((const char *) text);
		g_free(text);
	}
}

void ClipboardHandler::receivedClipboardContents(GtkClipboard *clipboard, GtkSelectionData * selectionData,
		ClipboardHandler * handler) {

	handler->containsText = gtk_selection_data_targets_include_text(selectionData);
	handler->listener->clipboardPasteEnabled(handler->containsText);
}

#ifdef NOT_DEF

/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/gimp.h"
#include "core/gimpbuffer.h"

#include "gimpclipboard.h"
#include "gimppixbuf.h"
#include "gimpselectiondata.h"

#include "gimp-intl.h"

#define GIMP_CLIPBOARD_KEY "gimp-clipboard"

typedef struct _GimpClipboard GimpClipboard;

struct _GimpClipboard {
	GSList *pixbuf_formats;

	GtkTargetEntry *target_entries;
	gint n_target_entries;

	GtkTargetEntry *svg_target_entries;
	gint n_svg_target_entries;

	GimpBuffer *buffer;
	gchar *svg;
};

static GimpClipboard * gimp_clipboard_get(Gimp *gimp);
static void gimp_clipboard_clear(GimpClipboard *gimp_clip);
static void gimp_clipboard_free(GimpClipboard *gimp_clip);

static GdkAtom * gimp_clipboard_wait_for_targets(Gimp *gimp, gint *n_targets);
static GdkAtom gimp_clipboard_wait_for_buffer(Gimp *gimp);
static GdkAtom gimp_clipboard_wait_for_svg(Gimp *gimp);

static void gimp_clipboard_send_buffer(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info,
		Gimp *gimp);
static void gimp_clipboard_send_svg(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, Gimp *gimp);

/*  public functions  */

void gimp_clipboard_init(Gimp *gimp) {
	GimpClipboard *gimp_clip;
	GSList *list;

	g_return_if_fail (GIMP_IS_GIMP (gimp));

	gimp_clip = gimp_clipboard_get(gimp);

	g_return_if_fail (gimp_clip == NULL);

	gimp_clip = g_slice_new0 (GimpClipboard);

	g_object_set_data_full(G_OBJECT (gimp), GIMP_CLIPBOARD_KEY, gimp_clip, (GDestroyNotify) gimp_clipboard_free);

	gimp_clip->pixbuf_formats = gimp_pixbuf_get_formats();

	for (list = gimp_clip->pixbuf_formats; list; list = g_slist_next (list)) {
		GdkPixbufFormat *format = list->data;

		if (gdk_pixbuf_format_is_writable(format)) {
			gchar **mime_types;
			gchar **type;

			mime_types = gdk_pixbuf_format_get_mime_types(format);

			for (type = mime_types; *type; type++)
			gimp_clip->n_target_entries++;

			g_strfreev(mime_types);
		}
	}

	if (gimp_clip->n_target_entries > 0) {
		gint i = 0;

		gimp_clip->target_entries = g_new0 (GtkTargetEntry,
				gimp_clip->n_target_entries);

		for (list = gimp_clip->pixbuf_formats; list; list = g_slist_next (list)) {
			GdkPixbufFormat *format = list->data;

			if (gdk_pixbuf_format_is_writable(format)) {
				gchar *format_name;
				gchar **mime_types;
				gchar **type;

				format_name = gdk_pixbuf_format_get_name(format);
				mime_types = gdk_pixbuf_format_get_mime_types(format);

				for (type = mime_types; *type; type++) {
					const gchar *mime_type = *type;

					if (gimp->be_verbose)
					g_printerr("clipboard: writable pixbuf format: %s\n", mime_type);

					gimp_clip->target_entries[i].target = g_strdup(mime_type);
					gimp_clip->target_entries[i].flags = 0;
					gimp_clip->target_entries[i].info = i;

					i++;
				}

				g_strfreev(mime_types);
				g_free(format_name);
			}
		}
	}

	gimp_clip->n_svg_target_entries = 2;
	gimp_clip->svg_target_entries = g_new0 (GtkTargetEntry, 2);

	gimp_clip->svg_target_entries[0].target = g_strdup("image/svg");
	gimp_clip->svg_target_entries[0].flags = 0;
	gimp_clip->svg_target_entries[0].info = 0;

	gimp_clip->svg_target_entries[1].target = g_strdup("image/svg+xml");
	gimp_clip->svg_target_entries[1].flags = 0;
	gimp_clip->svg_target_entries[1].info = 1;
}

void gimp_clipboard_exit(Gimp *gimp) {
	GtkClipboard *clipboard;

	g_return_if_fail (GIMP_IS_GIMP (gimp));

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard && gtk_clipboard_get_owner(clipboard) == G_OBJECT (gimp))
	gtk_clipboard_store(clipboard);

	g_object_set_data(G_OBJECT (gimp), GIMP_CLIPBOARD_KEY, NULL);
}

/**
 * gimp_clipboard_has_buffer:
 * @gimp: pointer to #Gimp
 *
 * Tests if there's image data in the clipboard. If the global cut
 * buffer of @gimp is empty, this function checks if there's image
 * data in %GDK_SELECTION_CLIPBOARD. This is done in a main-loop
 * similar to gtk_clipboard_wait_is_text_available(). The same caveats
 * apply here.
 *
 * Return value: %TRUE if there's image data in the clipboard, %FALSE otherwise
 **/
gboolean gimp_clipboard_has_buffer(Gimp *gimp) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;

	g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard && gtk_clipboard_get_owner(clipboard) != G_OBJECT (gimp) && gimp_clipboard_wait_for_buffer(gimp)
			!= GDK_NONE) {
		return TRUE;
	}

	gimp_clip = gimp_clipboard_get(gimp);

	return (gimp_clip->buffer != NULL);
}

/**
 * gimp_clipboard_has_svg:
 * @gimp: pointer to #Gimp
 *
 * Tests if there's SVG data in %GDK_SELECTION_CLIPBOARD.
 * This is done in a main-loop similar to
 * gtk_clipboard_wait_is_text_available(). The same caveats apply here.
 *
 * Return value: %TRUE if there's SVG data in the clipboard, %FALSE otherwise
 **/
gboolean gimp_clipboard_has_svg(Gimp *gimp) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;

	g_return_val_if_fail (GIMP_IS_GIMP (gimp), FALSE);

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard && gtk_clipboard_get_owner(clipboard) != G_OBJECT (gimp) && gimp_clipboard_wait_for_svg(gimp)
			!= GDK_NONE) {
		return TRUE;
	}

	gimp_clip = gimp_clipboard_get(gimp);

	return (gimp_clip->svg != NULL);
}

/**
 * gimp_clipboard_get_buffer:
 * @gimp: pointer to #Gimp
 *
 * Retrieves either image data from %GDK_SELECTION_CLIPBOARD or from
 * the global cut buffer of @gimp.
 *
 * The returned #GimpBuffer needs to be unref'ed when it's no longer
 * needed.
 *
 * Return value: a reference to a #GimpBuffer or %NULL if there's no
 *               image data
 **/
GimpBuffer *
gimp_clipboard_get_buffer(Gimp *gimp) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;
	GdkAtom atom;
	GimpBuffer *buffer = NULL;

	g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard && gtk_clipboard_get_owner(clipboard) != G_OBJECT (gimp) && (atom = gimp_clipboard_wait_for_buffer(
							gimp)) != GDK_NONE) {
		GtkSelectionData *data;

		gimp_set_busy(gimp);

		data = gtk_clipboard_wait_for_contents(clipboard, atom);

		if (data) {
			GdkPixbuf *pixbuf = gtk_selection_data_get_pixbuf(data);

			gtk_selection_data_free(data);

			if (pixbuf) {
				buffer = gimp_buffer_new_from_pixbuf(pixbuf, _("Clipboard"));
				g_object_unref(pixbuf);
			}
		}

		gimp_unset_busy(gimp);
	}

	gimp_clip = gimp_clipboard_get(gimp);

	if (!buffer && gimp_clip->buffer)
	buffer = g_object_ref(gimp_clip->buffer);

	return buffer;
}

/**
 * gimp_clipboard_get_svg:
 * @gimp: pointer to #Gimp
 * @svg_length: returns the size of the SVG stream in bytes
 *
 * Retrieves SVG data from %GDK_SELECTION_CLIPBOARD or from the global
 * SVG buffer of @gimp.
 *
 * The returned data needs to be freed when it's no longer needed.
 *
 * Return value: a reference to a #GimpBuffer or %NULL if there's no
 *               image data
 **/
gchar *
gimp_clipboard_get_svg(Gimp *gimp, gsize *svg_length) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;
	GdkAtom atom;
	gchar *svg = NULL;

	g_return_val_if_fail (GIMP_IS_GIMP (gimp), NULL);
	g_return_val_if_fail (svg_length != NULL, NULL);

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard && gtk_clipboard_get_owner(clipboard) != G_OBJECT (gimp)
			&& (atom = gimp_clipboard_wait_for_svg(gimp)) != GDK_NONE) {
		GtkSelectionData *data;

		gimp_set_busy(gimp);

		data = gtk_clipboard_wait_for_contents(clipboard, atom);

		if (data) {
			const guchar *stream;

			stream = gimp_selection_data_get_stream(data, svg_length);

			if (stream)
			svg = g_memdup(stream, *svg_length);

			gtk_selection_data_free(data);
		}

		gimp_unset_busy(gimp);
	}

	gimp_clip = gimp_clipboard_get(gimp);

	if (!svg && gimp_clip->svg) {
		svg = g_strdup(gimp_clip->svg);
		*svg_length = strlen(svg);
	}

	return svg;
}

/**
 * gimp_clipboard_set_buffer:
 * @gimp:   pointer to #Gimp
 * @buffer: a #GimpBuffer, or %NULL.
 *
 * Offers the buffer in %GDK_SELECTION_CLIPBOARD.
 **/
void gimp_clipboard_set_buffer(Gimp *gimp, GimpBuffer *buffer) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;

	g_return_if_fail (GIMP_IS_GIMP (gimp));
	g_return_if_fail (buffer == NULL || GIMP_IS_BUFFER (buffer));

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	if (!clipboard)
	return;

	gimp_clip = gimp_clipboard_get(gimp);

	gimp_clipboard_clear(gimp_clip);

	if (buffer) {
		gimp_clip->buffer = g_object_ref(buffer);

		gtk_clipboard_set_with_owner(clipboard, gimp_clip->target_entries, gimp_clip->n_target_entries,
				(GtkClipboardGetFunc) gimp_clipboard_send_buffer, (GtkClipboardClearFunc) NULL, G_OBJECT (gimp));

		/*  mark the first entry (image/png) as suitable for storing  */
		gtk_clipboard_set_can_store(clipboard, gimp_clip->target_entries, 1);
	} else if (gtk_clipboard_get_owner(clipboard) == G_OBJECT (gimp)) {
		gtk_clipboard_clear(clipboard);
	}
}

/**
 * gimp_clipboard_set_svg:
 * @gimp: pointer to #Gimp
 * @svg: a string containing the SVG data, or %NULL
 *
 * Offers SVG data in %GDK_SELECTION_CLIPBOARD.
 **/
void gimp_clipboard_set_svg(Gimp *gimp, const gchar *svg) {
	GimpClipboard *gimp_clip;
	GtkClipboard *clipboard;

	g_return_if_fail (GIMP_IS_GIMP (gimp));

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	if (!clipboard)
	return;

	gimp_clip = gimp_clipboard_get(gimp);

	gimp_clipboard_clear(gimp_clip);

	if (svg) {
		gimp_clip->svg = g_strdup(svg);

		gtk_clipboard_set_with_owner(clipboard, gimp_clip->svg_target_entries, gimp_clip->n_svg_target_entries,
				(GtkClipboardGetFunc) gimp_clipboard_send_svg, (GtkClipboardClearFunc) NULL, G_OBJECT (gimp));

		/*  mark the first entry (image/svg) as suitable for storing  */
		gtk_clipboard_set_can_store(clipboard, gimp_clip->svg_target_entries, 1);
	} else if (gtk_clipboard_get_owner(clipboard) == G_OBJECT (gimp)) {
		gtk_clipboard_clear(clipboard);
	}
}

/**
 * gimp_clipboard_set_text:
 * @gimp: pointer to #Gimp
 * @text: a %NULL-terminated string in UTF-8 encoding
 *
 * Offers @text in %GDK_SELECTION_CLIPBOARD and %GDK_SELECTION_PRIMARY.
 **/
void gimp_clipboard_set_text(Gimp *gimp, const gchar *text) {
	GtkClipboard *clipboard;

	g_return_if_fail (GIMP_IS_GIMP (gimp));
	g_return_if_fail (text != NULL);

	gimp_clipboard_clear(gimp_clipboard_get(gimp));

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);
	if (clipboard)
	gtk_clipboard_set_text(clipboard, text, -1);

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
	if (clipboard)
	gtk_clipboard_set_text(clipboard, text, -1);
}

/*  private functions  */

static GimpClipboard *
gimp_clipboard_get(Gimp *gimp) {
	return g_object_get_data(G_OBJECT (gimp), GIMP_CLIPBOARD_KEY);
}

static void gimp_clipboard_clear(GimpClipboard *gimp_clip) {
	if (gimp_clip->buffer) {
		g_object_unref(gimp_clip->buffer);
		gimp_clip->buffer = NULL;
	}

	if (gimp_clip->svg) {
		g_free(gimp_clip->svg);
		gimp_clip->svg = NULL;
	}
}

static void gimp_clipboard_free(GimpClipboard *gimp_clip) {
	gint i;

	gimp_clipboard_clear(gimp_clip);

	g_slist_free(gimp_clip->pixbuf_formats);

	for (i = 0; i < gimp_clip->n_target_entries; i++)
	g_free(gimp_clip->target_entries[i].target);

	g_free(gimp_clip->target_entries);

	for (i = 0; i < gimp_clip->n_svg_target_entries; i++)
	g_free(gimp_clip->svg_target_entries[i].target);

	g_free(gimp_clip->svg_target_entries);

	g_slice_free (GimpClipboard, gimp_clip);
}

static GdkAtom *
gimp_clipboard_wait_for_targets(Gimp *gimp, gint *n_targets) {
	GtkClipboard *clipboard;

	clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD);

	if (clipboard) {
		GtkSelectionData *data;
		GdkAtom atom = gdk_atom_intern_static_string("TARGETS");

		data = gtk_clipboard_wait_for_contents(clipboard, atom);

		if (data) {
			GdkAtom *targets;
			gboolean success;

			success = gtk_selection_data_get_targets(data, &targets, n_targets);

			gtk_selection_data_free(data);

			if (success) {
				if (gimp->be_verbose) {
					gint i;

					for (i = 0; i < *n_targets; i++)
					g_printerr("clipboard: offered type: %s\n", gdk_atom_name(targets[i]));

					g_printerr("\n");
				}

				return targets;
			}
		}
	}

	return NULL;
}

static GdkAtom gimp_clipboard_wait_for_buffer(Gimp *gimp) {
	GimpClipboard *gimp_clip = gimp_clipboard_get(gimp);
	GdkAtom *targets;
	gint n_targets;
	GdkAtom result = GDK_NONE;

	targets = gimp_clipboard_wait_for_targets(gimp, &n_targets);

	if (targets) {
		GSList *list;

		for (list = gimp_clip->pixbuf_formats; list; list = g_slist_next (list)) {
			GdkPixbufFormat *format = list->data;
			gchar **mime_types;
			gchar **type;

			if (gimp->be_verbose)
			g_printerr("clipboard: checking pixbuf format '%s'\n", gdk_pixbuf_format_get_name(format));

			mime_types = gdk_pixbuf_format_get_mime_types(format);

			for (type = mime_types; *type; type++) {
				gchar *mime_type = *type;
				GdkAtom atom = gdk_atom_intern(mime_type, FALSE);
				gint i;

				if (gimp->be_verbose)
				g_printerr("  - checking mime type '%s'\n", mime_type);

				for (i = 0; i < n_targets; i++) {
					if (targets[i] == atom) {
						result = atom;
						break;
					}
				}

				if (result != GDK_NONE)
				break;
			}

			g_strfreev(mime_types);

			if (result != GDK_NONE)
			break;
		}

		g_free(targets);
	}

	return result;
}

static GdkAtom gimp_clipboard_wait_for_svg(Gimp *gimp) {
	GdkAtom *targets;
	gint n_targets;
	GdkAtom result = GDK_NONE;

	targets = gimp_clipboard_wait_for_targets(gimp, &n_targets);

	if (targets) {
		GdkAtom svg_atom = gdk_atom_intern_static_string("image/svg");
		GdkAtom svg_xml_atom = gdk_atom_intern_static_string("image/svg+xml");
		gint i;

		for (i = 0; i < n_targets; i++) {
			if (targets[i] == svg_atom) {
				result = svg_atom;
				break;
			} else if (targets[i] == svg_xml_atom) {
				result = svg_xml_atom;
				break;
			}
		}

		g_free(targets);
	}

	return result;
}

static void gimp_clipboard_send_buffer(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info,
		Gimp *gimp) {
	GimpClipboard *gimp_clip = gimp_clipboard_get(gimp);
	GdkPixbuf *pixbuf;

	gimp_set_busy(gimp);

	pixbuf = gimp_viewable_get_pixbuf(GIMP_VIEWABLE(gimp_clip->buffer), gimp_get_user_context(gimp),
			gimp_buffer_get_width(gimp_clip->buffer), gimp_buffer_get_height(gimp_clip->buffer));

	if (pixbuf) {
		if (gimp->be_verbose)
		g_printerr("clipboard: sending pixbuf data as '%s'\n", gimp_clip->target_entries[info].target);

		gtk_selection_data_set_pixbuf(selection_data, pixbuf);
	} else {
		g_warning ("%s: gimp_viewable_get_pixbuf() failed", G_STRFUNC);
	}

	gimp_unset_busy(gimp);
}

void
gimp_selection_data_set_stream (GtkSelectionData *selection,
		const guchar *stream,
		gsize stream_length)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (stream != NULL);
	g_return_if_fail (stream_length > 0);

	gtk_selection_data_set (selection, selection->target,
			8, (guchar *) stream, stream_length);
}

static void gimp_clipboard_send_svg(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, Gimp *gimp) {
	GimpClipboard *gimp_clip = gimp_clipboard_get(gimp);

	gimp_set_busy(gimp);

	if (gimp_clip->svg) {
		if (gimp->be_verbose)
		g_printerr("clipboard: sending SVG data as '%s'\n", gimp_clip->svg_target_entries[info].target);

		gimp_selection_data_set_stream(selection_data, (const guchar *) gimp_clip->svg, strlen(gimp_clip->svg));
	}

	gimp_unset_busy(gimp);
}

#endif
