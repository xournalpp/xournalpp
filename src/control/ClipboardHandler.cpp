#include "ClipboardHandler.h"
#include "Control.h"
#include <pixbuf-utils.h>
#include <cairo-svg.h>
#include <config.h>
#include "../view/DocumentView.h"
#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>
#include <serializing/BinObjectEncoding.h>

ClipboardHandler::ClipboardHandler(ClipboardListener * listener, GtkWidget * widget) {
	XOJ_INIT_TYPE(ClipboardHandler);

	this->listener = listener;
	this->clipboard = gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);
	this->containsText = false;
	this->containsImage = false;
	this->containsXournal = false;
	this->selection = NULL;

	this->hanlderId = g_signal_connect(this->clipboard, "owner-change", G_CALLBACK(&ownerChangedCallback), this);

	this->listener->clipboardCutCopyEnabled(false);

	GdkDisplay * display = gtk_clipboard_get_display(clipboard);

	if (gdk_display_supports_selection_notification(display)) {
		gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"), (GtkClipboardReceivedFunc) receivedClipboardContents, this);
	} else {
		// XFIXES extension not available, make Paste always sensitive
		this->listener->clipboardPasteEnabled(true);
	}
}

ClipboardHandler::~ClipboardHandler() {
	XOJ_CHECK_TYPE(ClipboardHandler);

	g_signal_handler_disconnect(this->clipboard, this->hanlderId);

	XOJ_RELEASE_TYPE(ClipboardHandler);
}

static GdkAtom atomXournal = gdk_atom_intern_static_string("application/xournal");
static GdkAtom atomUtf8 = gdk_atom_intern_static_string("UTF8_STRING");

bool ClipboardHandler::paste() {
	XOJ_CHECK_TYPE(ClipboardHandler);

	if (this->containsXournal) {
		gtk_clipboard_request_contents(this->clipboard, atomXournal, (GtkClipboardReceivedFunc) pasteClipboardContents, this);
		return true;
	} else if (this->containsText) {
		gtk_clipboard_request_contents(this->clipboard, atomUtf8, (GtkClipboardReceivedFunc) pasteClipboardContents, this);
		return true;
	} else if (this->containsImage) {
		gtk_clipboard_request_image(this->clipboard, (GtkClipboardImageReceivedFunc) pasteClipboardImage, this);
		return true;
	}

	return false;
}

bool ClipboardHandler::cut() {
	XOJ_CHECK_TYPE(ClipboardHandler);

	bool result = this->copy();
	this->listener->deleteSelection();

	return result;
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
	ClipboardContents(String text, GdkPixbuf * image, String svg, GString * str) {
		this->text = text;
		this->image = image;
		this->svg = svg;
		this->str = str;
	}

	~ClipboardContents() {
		gdk_pixbuf_unref(this->image);
		g_string_free(this->str, true);
	}

public:
	static void getFunction(GtkClipboard * clipboard, GtkSelectionData * selection, guint info, ClipboardContents * contents) {

		if (selection->target == gdk_atom_intern_static_string("UTF8_STRING")) {
			gtk_selection_data_set_text(selection, contents->text.c_str(), -1);
		} else if (selection->target == gdk_atom_intern_static_string("image/png") || selection->target == gdk_atom_intern_static_string("image/jpeg")
				|| selection->target == gdk_atom_intern_static_string("image/gif")) {
			gtk_selection_data_set_pixbuf(selection, contents->image);
		} else if (atomSvg1 == selection->target || atomSvg2 == selection->target) {
			gtk_selection_data_set(selection, selection->target, 8, (guchar *) contents->svg.c_str(), contents->svg.size());
		} else if (atomXournal == selection->target) {
			gtk_selection_data_set(selection, selection->target, 8, (guchar *) contents->str->str, contents->str->len);
		}
	}

	static void clearFunction(GtkClipboard * clipboard, ClipboardContents * contents) {
		delete contents;
	}

private:
	String text;
	GdkPixbuf * image;
	String svg;
	GString * str;
};

static cairo_status_t svgWriteFunction(GString * string, const unsigned char *data, unsigned int length) {
	g_string_append_len(string, (const gchar *) data, length);
	return CAIRO_STATUS_SUCCESS;
}

bool ClipboardHandler::copy() {
	XOJ_CHECK_TYPE(ClipboardHandler);

	if (!this->selection) {
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// prepare xournal contents
	/////////////////////////////////////////////////////////////////

	ObjectOutputStream out(new BinObjectEncoding());

	out.writeString(PACKAGE_STRING);

	out << this->selection;

	/////////////////////////////////////////////////////////////////
	// prepare text contents
	/////////////////////////////////////////////////////////////////

	GList * textElements = NULL;

	ListIterator<Element *> it = this->selection->getElements();

	while (it.hasNext()) {
		Element * e = it.next();
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

	int width = selection->getWidth() * dpiFactor;
	int heigth = selection->getHeight() * dpiFactor;
	cairo_surface_t * surfacePng = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, heigth);
	cairo_t * crPng = cairo_create(surfacePng);
	cairo_scale(crPng, dpiFactor, dpiFactor);

	cairo_translate(crPng, -selection->getXOnView(), -selection->getYOnView());
	view.drawSelection(crPng, this->selection);

	cairo_destroy(crPng);

	GdkPixbuf * image = xoj_pixbuf_get_from_surface(surfacePng, 0, 0, width, heigth);

	cairo_surface_destroy(surfacePng);

	/////////////////////////////////////////////////////////////////
	// prepare image contents: SVG
	/////////////////////////////////////////////////////////////////

	GString * svgString = g_string_sized_new(1048576); // 1MB

	cairo_surface_t * surfaceSVG = cairo_svg_surface_create_for_stream((cairo_write_func_t) svgWriteFunction, svgString, selection->getWidth(),
			selection->getHeight());
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

	// if we have text elements...
	if (!text.isEmpty()) {
		gtk_target_list_add_text_targets(list, 0);
	}
	// we always copy an image to clipboard
	gtk_target_list_add_image_targets(list, 0, TRUE);
	gtk_target_list_add(list, atomSvg1, 0, 0);
	gtk_target_list_add(list, atomSvg2, 0, 0);
	gtk_target_list_add(list, atomXournal, 0, 0);

	targets = gtk_target_table_new_from_list(list, &n_targets);

	ClipboardContents * contents = new ClipboardContents(text, image, svgString->str, out.getStr());

	gtk_clipboard_set_with_data(this->clipboard, targets, n_targets, (GtkClipboardGetFunc) ClipboardContents::getFunction,
			(GtkClipboardClearFunc) ClipboardContents::clearFunction, contents);
	gtk_clipboard_set_can_store(this->clipboard, NULL, 0);

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

	g_string_free(svgString, true);

	return true;
}

void ClipboardHandler::setSelection(EditSelection * selection) {
	XOJ_CHECK_TYPE(ClipboardHandler);

	this->selection = selection;

	this->listener->clipboardCutCopyEnabled(selection != NULL);
}

void ClipboardHandler::setCopyPasteEnabled(bool enabled) {
	XOJ_CHECK_TYPE(ClipboardHandler);

	if (enabled) {
		listener->clipboardCutCopyEnabled(true);
	} else if (!selection) {
		listener->clipboardCutCopyEnabled(false);
	}
}

void ClipboardHandler::ownerChangedCallback(GtkClipboard * clip, GdkEvent * event, ClipboardHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, ClipboardHandler);

	if (event->type == GDK_OWNER_CHANGE) {
		handler->clipboardUpdated(event->owner_change.selection);
	}
}

void ClipboardHandler::clipboardUpdated(GdkAtom atom) {
	XOJ_CHECK_TYPE(ClipboardHandler);

	gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"), (GtkClipboardReceivedFunc) receivedClipboardContents, this);
}

void ClipboardHandler::pasteClipboardImage(GtkClipboard * clipboard, GdkPixbuf * pixbuf, ClipboardHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, ClipboardHandler);

	handler->listener->clipboardPasteImage(pixbuf);
}

void ClipboardHandler::pasteClipboardContents(GtkClipboard * clipboard, GtkSelectionData * selectionData, ClipboardHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, ClipboardHandler);

	if (atomXournal == selectionData->target) {
		ObjectInputStream in;

		if (in.read((const char *) selectionData->data, selectionData->length)) {
			handler->listener->clipboardPasteXournal(in);
		}
	} else {
		guchar * text = gtk_selection_data_get_text(selectionData);
		if (text != NULL) {
			handler->listener->clipboardPasteText((const char *) text);
			g_free(text);
		}
	}
}

gboolean gtk_selection_data_targets_include_xournal(GtkSelectionData * selection_data) {
	GdkAtom * targets;
	gint n_targets;
	gboolean result = FALSE;

	if (gtk_selection_data_get_targets(selection_data, &targets, &n_targets)) {
		for (int i = 0; i < n_targets; i++) {
			if (targets[i] == atomXournal) {
				result = true;
				break;
			}
		}
		g_free(targets);
	}

	return result;
}

void ClipboardHandler::receivedClipboardContents(GtkClipboard * clipboard, GtkSelectionData * selectionData, ClipboardHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, ClipboardHandler);

	handler->containsText = gtk_selection_data_targets_include_text(selectionData);
	handler->containsXournal = gtk_selection_data_targets_include_xournal(selectionData);
	handler->containsImage = gtk_selection_data_targets_include_image(selectionData, false);

	handler->listener->clipboardPasteEnabled(handler->containsText || handler->containsXournal || handler->containsImage);
}
