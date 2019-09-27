#include "ClipboardHandler.h"

#include "Control.h"
#include "Util.h"
#include "view/DocumentView.h"

#include <config.h>
#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>
#include <serializing/BinObjectEncoding.h>

#include <cairo-svg.h>
#include <pixbuf-utils.h>

ClipboardListener::~ClipboardListener() {
}

ClipboardHandler::ClipboardHandler(ClipboardListener* listener, GtkWidget* widget)
{
	this->listener = listener;
	this->clipboard = gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);

	this->hanlderId = g_signal_connect(this->clipboard, "owner-change", G_CALLBACK(&ownerChangedCallback), this);

	this->listener->clipboardCutCopyEnabled(false);

	gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
								   (GtkClipboardReceivedFunc) receivedClipboardContents, this);
}

ClipboardHandler::~ClipboardHandler()
{
	g_signal_handler_disconnect(this->clipboard, this->hanlderId);
}

static GdkAtom atomXournal = gdk_atom_intern_static_string("application/xournal");

bool ClipboardHandler::paste()
{
	if (this->containsXournal)
	{
		gtk_clipboard_request_contents(this->clipboard, atomXournal,
									   (GtkClipboardReceivedFunc) pasteClipboardContents, this);
		return true;
	}
	else if (this->containsText)
	{
		gtk_clipboard_request_text(this->clipboard,
		                           (GtkClipboardTextReceivedFunc) pasteClipboardText, this);
		return true;
	}
	else if (this->containsImage)
	{
		gtk_clipboard_request_image(this->clipboard,
									(GtkClipboardImageReceivedFunc) pasteClipboardImage, this);
		return true;
	}

	return false;
}

bool ClipboardHandler::cut()
{
	bool result = this->copy();
	this->listener->deleteSelection();

	return result;
}

gint ElementCompareFunc(Element* a, Element* b)
{
	if (a->getY() == b->getY())
	{
		return a->getX() - b->getX();
	}
	return a->getY() - b->getY();
}

static GdkAtom atomSvg1 = gdk_atom_intern_static_string("image/svg");
static GdkAtom atomSvg2 = gdk_atom_intern_static_string("image/svg+xml");

// The contents of the clipboard
class ClipboardContents
{
public:
	ClipboardContents(string text, GdkPixbuf* image, string svg, GString* str)
	{
		this->text = text;
		this->image = image;
		this->svg = svg;
		this->str = str;
	}

	~ClipboardContents()
	{
		g_object_unref(this->image);
		g_string_free(this->str, true);
	}

public:

	static void getFunction(GtkClipboard* clipboard, GtkSelectionData* selection,
							guint info, ClipboardContents* contents)
	{
		GdkAtom target = gtk_selection_data_get_target(selection);

		if (target == gdk_atom_intern_static_string("UTF8_STRING"))
		{
			gtk_selection_data_set_text(selection, contents->text.c_str(), -1);
		}
		else if (target == gdk_atom_intern_static_string("image/png") ||
				 target == gdk_atom_intern_static_string("image/jpeg") ||
				 target == gdk_atom_intern_static_string("image/gif"))
		{
			gtk_selection_data_set_pixbuf(selection, contents->image);
		}
		else if (atomSvg1 == target || atomSvg2 == target)
		{
			gtk_selection_data_set(selection, target, 8, (guchar*) contents->svg.c_str(),
								   contents->svg.length());
		}
		else if (atomXournal == target)
		{
			gtk_selection_data_set(selection, target, 8, (guchar*) contents->str->str, contents->str->len);
		}
	}

	static void clearFunction(GtkClipboard* clipboard, ClipboardContents* contents)
	{
		delete contents;
	}

private:
	string text;
	GdkPixbuf* image;
	string svg;
	GString* str;
};

static cairo_status_t svgWriteFunction(GString* string, const unsigned char* data, unsigned int length)
{
	g_string_append_len(string, (const gchar*) data, length);
	return CAIRO_STATUS_SUCCESS;
}

bool ClipboardHandler::copy()
{
	if (!this->selection)
	{
		return false;
	}

	/////////////////////////////////////////////////////////////////
	// prepare xournal contents
	/////////////////////////////////////////////////////////////////

	ObjectOutputStream out(new BinObjectEncoding());

	out.writeString(PROJECT_STRING);

	this->selection->serialize(out);

	/////////////////////////////////////////////////////////////////
	// prepare text contents
	/////////////////////////////////////////////////////////////////

	GList* textElements = nullptr;

	for (Element* e : *this->selection->getElements())
	{
		if (e->getType() == ELEMENT_TEXT)
		{
			textElements = g_list_insert_sorted(textElements, e, (GCompareFunc) ElementCompareFunc);
		}
	}

	string text = "";
	for (GList* l = textElements; l != nullptr; l = l->next)
	{
		Text* e = (Text*) l->data;
		if (text != "")
		{
			text += "\n";
		}
		text += e->getText();
	}
	g_list_free(textElements);

	/////////////////////////////////////////////////////////////////
	// prepare image contents: PNG
	/////////////////////////////////////////////////////////////////

	DocumentView view;

	double dpiFactor = 1.0 / Util::DPI_NORMALIZATION_FACTOR * 300.0;

	int width = selection->getWidth() * dpiFactor;
	int height = selection->getHeight() * dpiFactor;
	cairo_surface_t* surfacePng = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	cairo_t* crPng = cairo_create(surfacePng);
	cairo_scale(crPng, dpiFactor, dpiFactor);

	cairo_translate(crPng, -selection->getXOnView(), -selection->getYOnView());
	view.drawSelection(crPng, this->selection);

	cairo_destroy(crPng);

	GdkPixbuf* image = xoj_pixbuf_get_from_surface(surfacePng, 0, 0, width, height);

	cairo_surface_destroy(surfacePng);

	/////////////////////////////////////////////////////////////////
	// prepare image contents: SVG
	/////////////////////////////////////////////////////////////////

	GString* svgString = g_string_sized_new(1048576); // 1MB

	cairo_surface_t* surfaceSVG = cairo_svg_surface_create_for_stream(
									(cairo_write_func_t) svgWriteFunction, svgString,
									selection->getWidth(), selection->getHeight()
									);
	cairo_t* crSVG = cairo_create(surfaceSVG);

	view.drawSelection(crSVG, this->selection);

	cairo_surface_destroy(surfaceSVG);
	cairo_destroy(crSVG);

	/////////////////////////////////////////////////////////////////
	// copy to clipboard
	/////////////////////////////////////////////////////////////////

	GtkTargetList* list = gtk_target_list_new(nullptr, 0);
	GtkTargetEntry* targets;
	int n_targets;

	// if we have text elements...
	if (!text.empty())
	{
		gtk_target_list_add_text_targets(list, 0);
	}
	// we always copy an image to clipboard
	gtk_target_list_add_image_targets(list, 0, true);
	gtk_target_list_add(list, atomSvg1, 0, 0);
	gtk_target_list_add(list, atomSvg2, 0, 0);
	gtk_target_list_add(list, atomXournal, 0, 0);

	targets = gtk_target_table_new_from_list(list, &n_targets);

	ClipboardContents* contents = new ClipboardContents(text, image, svgString->str, out.getStr());

	gtk_clipboard_set_with_data(this->clipboard, targets, n_targets,
								(GtkClipboardGetFunc) ClipboardContents::getFunction,
								(GtkClipboardClearFunc) ClipboardContents::clearFunction, contents);
	gtk_clipboard_set_can_store(this->clipboard, nullptr, 0);

	gtk_target_table_free(targets, n_targets);
	gtk_target_list_unref(list);

	g_string_free(svgString, true);

	return true;
}

void ClipboardHandler::setSelection(EditSelection* selection)
{
	this->selection = selection;

	this->listener->clipboardCutCopyEnabled(selection != nullptr);
}

void ClipboardHandler::setCopyPasteEnabled(bool enabled)
{
	if (enabled)
	{
		listener->clipboardCutCopyEnabled(true);
	}
	else if (!selection)
	{
		listener->clipboardCutCopyEnabled(false);
	}
}

void ClipboardHandler::ownerChangedCallback(GtkClipboard* clip, GdkEvent* event, ClipboardHandler* handler)
{
	if (event->type == GDK_OWNER_CHANGE)
	{
		handler->clipboardUpdated(event->owner_change.selection);
	}
}

void ClipboardHandler::clipboardUpdated(GdkAtom atom)
{
	gtk_clipboard_request_contents(clipboard,
								   gdk_atom_intern_static_string("TARGETS"),
								   (GtkClipboardReceivedFunc) receivedClipboardContents, this);
}

void ClipboardHandler::pasteClipboardImage(GtkClipboard* clipboard, GdkPixbuf* pixbuf, ClipboardHandler* handler)
{
	handler->listener->clipboardPasteImage(pixbuf);
}

void ClipboardHandler::pasteClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
											  ClipboardHandler* handler)
{
	ObjectInputStream in;

	if (in.read((const char*) gtk_selection_data_get_data(selectionData), gtk_selection_data_get_length(selectionData)))
	{
		handler->listener->clipboardPasteXournal(in);
	}
}

void ClipboardHandler::pasteClipboardText(GtkClipboard* clipboard, const gchar* text,
                                          ClipboardHandler* handler)
{
	if (text)
	{
		handler->listener->clipboardPasteText(text);
	}
}

gboolean gtk_selection_data_targets_include_xournal(GtkSelectionData* selection_data)
{
	GdkAtom* targets;
	gint n_targets;
	gboolean result = false;

	if (gtk_selection_data_get_targets(selection_data, &targets, &n_targets))
	{
		for (int i = 0; i < n_targets; i++)
		{
			if (targets[i] == atomXournal)
			{
				result = true;
				break;
			}
		}
		g_free(targets);
	}

	return result;
}

void ClipboardHandler::receivedClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
												 ClipboardHandler* handler)
{
	handler->containsText = gtk_selection_data_targets_include_text(selectionData);
	handler->containsXournal = gtk_selection_data_targets_include_xournal(selectionData);
	handler->containsImage = gtk_selection_data_targets_include_image(selectionData, false);

	handler->listener->clipboardPasteEnabled(handler->containsText || handler->containsXournal || handler->containsImage);
}
