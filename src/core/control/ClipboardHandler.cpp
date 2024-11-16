#include "ClipboardHandler.h"

#include <set>      // for multiset, operator!=
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo-svg.h>      // for cairo_svg_surface_c...
#include <cairo.h>          // for cairo_create, cairo...
#include <glib-object.h>    // for g_object_unref, g_s...

#include "control/tools/EditSelection.h"          // for EditSelection
#include "model/Element.h"                        // for Element, ELEMENT_TEXT
#include "model/Text.h"                           // for Text
#include "util/Util.h"                            // for DPI_NORMALIZATION_F...
#include "util/gtk4_helper.h"                     // for gtk_widget_get_clipboard
#include "util/safe_casts.h"                      // for as_unsigned
#include "util/serializing/BinObjectEncoding.h"   // for BinObjectEncoding
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "view/ElementContainerView.h"            // for ElementContainerView
#include "view/View.h"                            // for Context

#include "config.h"  // for PROJECT_STRING

using std::string;

ClipboardListener::~ClipboardListener() = default;

ClipboardHandler::ClipboardHandler(ClipboardListener* listener, GtkWidget* widget) {
    this->listener = listener;
    this->clipboard = gtk_widget_get_clipboard(widget);

    this->handlerId = g_signal_connect(this->clipboard, "changed", G_CALLBACK(+[](GdkClipboard* clip, gpointer d) {
                                           static_cast<ClipboardHandler*>(d)->clipboardUpdated();
                                       }),
                                       this);

    this->listener->clipboardCutCopyEnabled(false);
    g_warning("Implement ClipboardHandler");

    // gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
    // reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
}

ClipboardHandler::~ClipboardHandler() { g_signal_handler_disconnect(this->clipboard, this->handlerId); }

// static GdkAtom atomXournal = gdk_atom_intern_static_string("application/xournal");

auto ClipboardHandler::paste() -> bool {
    /* Request targets again, since the owner-change signal is not emitted on MacOS and under X11 with no XFIXES
     * extension. See https://docs.gtk.org/gdk3/struct.EventOwnerChange.html and
     * https://gitlab.gnome.org/GNOME/gtk/-/issues/1757 */
    // gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
    //                                reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
    //
    // if (this->containsXournal) {
    //     gtk_clipboard_request_contents(this->clipboard, atomXournal,
    //                                    reinterpret_cast<GtkClipboardReceivedFunc>(pasteClipboardContents), this);
    //     return true;
    // }
    // if (this->containsText) {
    //     gtk_clipboard_request_text(this->clipboard,
    //     reinterpret_cast<GtkClipboardTextReceivedFunc>(pasteClipboardText),
    //                                this);
    //     return true;
    // }
    // if (this->containsImage) {
    //     gtk_clipboard_request_image(this->clipboard,
    //                                 reinterpret_cast<GtkClipboardImageReceivedFunc>(pasteClipboardImage), this);
    //     return true;
    // }

    return false;
}

auto ClipboardHandler::cut() -> bool {
    bool result = this->copy();
    this->listener->deleteSelection();

    return result;
}

auto ElementCompareFunc(Element* a, Element* b) -> bool {
    if (a->getY() == b->getY()) {
        return (a->getX() - b->getX()) < 0;
    }
    return (a->getY() - b->getY()) < 0;
}

// static GdkAtom atomSvg1 = gdk_atom_intern_static_string("image/svg");
// static GdkAtom atomSvg2 = gdk_atom_intern_static_string("image/svg+xml");

// The contents of the clipboard
class ClipboardContents {
public:
    ClipboardContents(string text, GdkPixbuf* image, string svg, GString* str) {
        this->text = std::move(text);
        this->image = image;
        this->svg = std::move(svg);
        this->str = str;
    }

    ~ClipboardContents() {
        g_object_unref(this->image);
        g_string_free(this->str, true);
    }


    // static void getFunction(GtkClipboard* clipboard, GtkSelectionData* selection, guint info,
    //                         ClipboardContents* contents) {
    //     GdkAtom target = gtk_selection_data_get_target(selection);
    //
    //     if (target == gdk_atom_intern_static_string("UTF8_STRING")) {
    //         gtk_selection_data_set_text(selection, contents->text.c_str(), -1);
    //     } else if (target == gdk_atom_intern_static_string("image/png") ||
    //                target == gdk_atom_intern_static_string("image/jpeg") ||
    //                target == gdk_atom_intern_static_string("image/gif")) {
    //         gtk_selection_data_set_pixbuf(selection, contents->image);
    //     } else if (atomSvg1 == target || atomSvg2 == target) {
    //         gtk_selection_data_set(selection, target, 8, reinterpret_cast<guchar const*>(contents->svg.c_str()),
    //                                static_cast<gint>(contents->svg.length()));
    //     } else if (atomXournal == target) {
    //         gtk_selection_data_set(selection, target, 8, reinterpret_cast<guchar*>(contents->str->str),
    //                                static_cast<gint>(contents->str->len));
    //     }
    // }
    //
    // static void clearFunction(GtkClipboard* clipboard, ClipboardContents* contents) { delete contents; }

private:
    string text;
    GdkPixbuf* image;
    string svg;
    GString* str;
};

static auto svgWriteFunction(GString* string, const unsigned char* data, unsigned int length) -> cairo_status_t {
    g_string_append_len(string, reinterpret_cast<const gchar*>(data), length);
    return CAIRO_STATUS_SUCCESS;
}

auto ClipboardHandler::copy() -> bool {
    if (!this->selection) {
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

    std::multiset<Text*, decltype(&ElementCompareFunc)> textElements(ElementCompareFunc);

    for (Element* e: this->selection->getElements()) {
        if (e->getType() == ELEMENT_TEXT) {
            textElements.insert(dynamic_cast<Text*>(e));
        }
    }

    string text{};
    for (Text* t: textElements) {
        if (!text.empty()) {
            text += "\n";
        }
        text += t->getText();
    }

    /////////////////////////////////////////////////////////////////
    // prepare image contents: PNG
    /////////////////////////////////////////////////////////////////

    double dpiFactor = 1.0 / Util::DPI_NORMALIZATION_FACTOR * 300.0;

    int width = static_cast<int>(selection->getWidth() * dpiFactor);
    int height = static_cast<int>(selection->getHeight() * dpiFactor);
    cairo_surface_t* surfacePng = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* crPng = cairo_create(surfacePng);
    cairo_scale(crPng, dpiFactor, dpiFactor);

    cairo_translate(crPng, -selection->getOriginalXOnView(), -selection->getOriginalYOnView());

    xoj::view::ElementContainerView view(this->selection);
    view.draw(xoj::view::Context::createDefault(crPng));

    cairo_destroy(crPng);

    GdkPixbuf* image = gdk_pixbuf_get_from_surface(surfacePng, 0, 0, width, height);

    cairo_surface_destroy(surfacePng);

    /////////////////////////////////////////////////////////////////
    // prepare image contents: SVG
    /////////////////////////////////////////////////////////////////

    GString* svgString = g_string_sized_new(1048576);  // 1MB

    cairo_surface_t* surfaceSVG =
            cairo_svg_surface_create_for_stream(reinterpret_cast<cairo_write_func_t>(svgWriteFunction), svgString,
                                                selection->getWidth(), selection->getHeight());
    cairo_t* crSVG = cairo_create(surfaceSVG);

    cairo_translate(crSVG, -selection->getOriginalXOnView(), -selection->getOriginalYOnView());
    view.draw(xoj::view::Context::createDefault(crSVG));

    cairo_surface_destroy(surfaceSVG);
    cairo_destroy(crSVG);

    /////////////////////////////////////////////////////////////////
    // copy to clipboard
    /////////////////////////////////////////////////////////////////

    g_warning("Implement ClipboardHandler");
    // GtkTargetList* list = gtk_target_list_new(nullptr, 0);
    // GtkTargetEntry* targets = nullptr;
    // int n_targets = 0;
    //
    // // if we have text elements...
    // if (!text.empty()) {
    //     gtk_target_list_add_text_targets(list, 0);
    // }
    // // we always copy an image to clipboard
    // gtk_target_list_add_image_targets(list, 0, true);
    // gtk_target_list_add(list, atomSvg1, 0, 0);
    // gtk_target_list_add(list, atomSvg2, 0, 0);
    // gtk_target_list_add(list, atomXournal, 0, 0);
    //
    // targets = gtk_target_table_new_from_list(list, &n_targets);
    //
    // auto* contents = new ClipboardContents(text, image, svgString->str, out.getStr());
    //
    // gtk_clipboard_set_with_data(this->clipboard, targets, static_cast<guint>(n_targets),
    //                             reinterpret_cast<GtkClipboardGetFunc>(ClipboardContents::getFunction),
    //                             reinterpret_cast<GtkClipboardClearFunc>(ClipboardContents::clearFunction), contents);
    // gtk_clipboard_set_can_store(this->clipboard, nullptr, 0);
    //
    // gtk_target_table_free(targets, n_targets);
    // gtk_target_list_unref(list);
    //
    // g_string_free(svgString, true);

    return true;
}

void ClipboardHandler::setSelection(EditSelection* selection) {
    this->selection = selection;

    this->listener->clipboardCutCopyEnabled(selection != nullptr);
}

void ClipboardHandler::setCopyCutEnabled(bool enabled) {
    if (enabled) {
        listener->clipboardCutCopyEnabled(true);
    } else if (!selection) {
        listener->clipboardCutCopyEnabled(false);
    }
}

void ClipboardHandler::clipboardUpdated() {
    gdk_clipboard_get_formats(clipboard);
    // gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
    // reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
}
/*
void ClipboardHandler::pasteClipboardImage(GtkClipboard* clipboard, GdkPixbuf* pixbuf, ClipboardHandler* handler) {
    if (pixbuf) {
        handler->listener->clipboardPasteImage(pixbuf);
    } else {
        g_warning("Trying to paste image, but pixbuf is null");
    }
}

void ClipboardHandler::pasteClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                              ClipboardHandler* handler) {
    ObjectInputStream in;

    if (in.read(reinterpret_cast<const char*>(gtk_selection_data_get_data(selectionData)),
                as_unsigned(gtk_selection_data_get_length(selectionData)))) {
        handler->listener->clipboardPasteXournal(in);
    }
}

void ClipboardHandler::pasteClipboardText(GtkClipboard* clipboard, const gchar* text, ClipboardHandler* handler) {
    if (text) {
        handler->listener->clipboardPasteText(text);
    }
}

auto gtk_selection_data_targets_include_xournal(GtkSelectionData* selection_data) -> gboolean {
    GdkAtom* targets = nullptr;
    gint n_targets = 0;
    gboolean result = false;

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

void ClipboardHandler::receivedClipboardContents(GtkClipboard* clipboard, GtkSelectionData* selectionData,
                                                 ClipboardHandler* handler) {
    handler->containsText = gtk_selection_data_targets_include_text(selectionData);
    handler->containsXournal = gtk_selection_data_targets_include_xournal(selectionData);
    handler->containsImage = gtk_selection_data_targets_include_image(selectionData, false);

    handler->listener->clipboardPasteEnabled(handler->containsText || handler->containsXournal ||
                                             handler->containsImage);
}*/
