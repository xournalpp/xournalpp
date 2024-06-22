#include "ClipboardHandler.h"

#include <set>      // for multiset, operator!=
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo-svg.h>    // for cairo_svg_surface_c...
#include <cairo.h>        // for cairo_create, cairo...
#include <glib-object.h>  // for g_object_unref, g_s...

#include "control/tools/EditSelection.h"  // for EditSelection
#include "model/Element.h"                // for Element, ELEMENT_TEXT
#include "model/Text.h"                   // for Text
#include "util/Util.h"                    // for DPI_NORMALIZATION_F...
#include "util/glib_casts.h"
#include "util/raii/CStringWrapper.h"
#include "util/raii/GBytesSPtr.h"
#include "util/raii/GLibGuards.h"
#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"                      // for as_unsigned
#include "util/serializing/BinObjectEncoding.h"   // for BinObjectEncoding
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream
#include "view/ElementContainerView.h"            // for ElementContainerView
#include "view/View.h"                            // for Context

#include "config.h"  // for PROJECT_STRING

constexpr auto CLIPBOARD_MIMETYPE_XOPP = "application/xournal";
constexpr auto CLIPBOARD_MIMETYPE_SVG = "image/svg";
constexpr auto CLIPBOARD_MIMETYPE_SVG_XML = "image/svg+xml";

static void pasteClipboardImage(GObject* source, GAsyncResult* res, ClipboardListener* listener) {
    const GValue* value = gdk_clipboard_read_value_finish(GDK_CLIPBOARD(source), res, nullptr);
    if (value && G_VALUE_HOLDS(value, GDK_TYPE_PIXBUF)) {
        GdkPixbuf* image = GDK_PIXBUF(g_value_get_object(value));
        g_object_ref(G_OBJECT(image));
        listener->clipboardPasteImage(image);
    } else {
        g_warning("Trying to paste image, but GdkPixbuf is null");
    }
}

static void pasteClipboardContents(GObject* source, GAsyncResult* res, ClipboardListener* listener) {
    xoj::util::GObjectSPtr<GInputStream> data(gdk_clipboard_read_finish(GDK_CLIPBOARD(source), res, nullptr, nullptr),
                                              xoj::util::adopt);

    constexpr size_t CHUNK_SIZE = 4096;
    char chunk[CHUNK_SIZE];
    gssize length = 0;
    std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    auto bytes_read = g_input_stream_read(data.get(), chunk, CHUNK_SIZE, nullptr, nullptr);
    while (bytes_read == CHUNK_SIZE) {
        stream.write(chunk, bytes_read);
        length += bytes_read;
        bytes_read = g_input_stream_read(data.get(), chunk, CHUNK_SIZE, nullptr, nullptr);
    }
    if (bytes_read == -1) {
        g_warning("Error pasting clipboard content: could not read input stream");
        return;
    }
    length += bytes_read;
    stream.write(chunk, bytes_read);

    stream.flush();

    ObjectInputStream in;
    if (in.read(std::move(stream), as_unsigned(length))) {
        listener->clipboardPasteXournal(in);
    }
}

static void pasteClipboardText(GObject* source, GAsyncResult* res, ClipboardListener* listener) {
    auto string = xoj::util::OwnedCString::assumeOwnership(
            gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), res, nullptr));
    if (string) {
        listener->clipboardPasteText(string.get());
    }
}

static bool dragAndDropReceived(GtkDropTarget* target, const GValue* value, double x, double y,
                                ClipboardListener* listener) {
    // Call the appropriate setter depending on the type of data that we received
    if (G_VALUE_HOLDS(value, G_TYPE_FILE)) {
        GFile* file = G_FILE(g_value_get_object(value));

        xoj::util::GObjectSPtr<GCancellable> cancel(g_cancellable_new(), xoj::util::adopt);
        auto cancelTimeout = g_timeout_add(
                3000,
                +[](gpointer cancel) -> gboolean {
                    g_cancellable_cancel(G_CANCELLABLE(cancel));
                    g_warning("Timeout... Cancel loading file");
                    return false;
                },
                cancel.get());

        xoj::util::GErrorGuard err{};
        xoj::util::GObjectSPtr<GFileInputStream> in(g_file_read(file, cancel.get(), xoj::util::out_ptr(err)),
                                                    xoj::util::adopt);
        if (err) {
            if (!g_error_matches(err.get(), G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                g_source_remove(cancelTimeout);
            }
            return false;
        }

        xoj::util::GObjectSPtr<GdkPixbuf> pixbuf(
                gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in.get()), cancel.get(), xoj::util::out_ptr(err)),
                xoj::util::adopt);
        if (err) {
            if (!g_error_matches(err.get(), G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                g_source_remove(cancelTimeout);
            }
            return false;
        }
        g_input_stream_close(G_INPUT_STREAM(in.get()), cancel.get(), xoj::util::out_ptr(err));
        if (err) {
            if (!g_error_matches(err.get(), G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                g_source_remove(cancelTimeout);
            }
            return false;
        }

        if (pixbuf) {
            listener->clipboardPasteImage(pixbuf.get());
        }
        if (!g_cancellable_is_cancelled(cancel.get())) {
            g_source_remove(cancelTimeout);
        }
    } else if (G_VALUE_HOLDS(value, GDK_TYPE_PIXBUF)) {
        listener->clipboardPasteImage(GDK_PIXBUF(g_value_get_object(value)));
    } else if (G_VALUE_HOLDS(value, G_TYPE_STRING)) {
        listener->clipboardPasteText(g_value_get_string(value));
    } else {
        return false;
    }

    return true;
}


ClipboardListener::~ClipboardListener() = default;

ClipboardHandler::ClipboardHandler(ClipboardListener* listener, GtkWidget* widget) {
    this->listener = listener;
    this->clipboard = gtk_widget_get_clipboard(widget);

    this->handlerId = g_signal_connect(
            this->clipboard, "changed",
            G_CALLBACK(+[](GdkClipboard*, gpointer d) { static_cast<ClipboardHandler*>(d)->checkFormats(); }), this);

    this->listener->clipboardCutCopyEnabled(false);


    GtkDropTarget* target = gtk_drop_target_new(G_TYPE_INVALID, GDK_ACTION_COPY);
    GType types[3] = {G_TYPE_FILE, GDK_TYPE_PIXBUF, G_TYPE_STRING};
    gtk_drop_target_set_gtypes(target, types, 3);
    g_signal_connect(target, "drop", xoj::util::wrap_for_g_callback_v<dragAndDropReceived>, this->listener);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(target));
}

ClipboardHandler::~ClipboardHandler() { g_signal_handler_disconnect(this->clipboard, this->handlerId); }

auto ClipboardHandler::paste() -> bool {
    checkFormats();
    if (containsXopp) {
        const char* mimetypes[2] = {CLIPBOARD_MIMETYPE_XOPP, nullptr};
        gdk_clipboard_read_async(clipboard, mimetypes, 0, nullptr, xoj::util::wrap_v<pasteClipboardContents>,
                                 this->listener);
        return true;
    }
    if (containsImage) {
        gdk_clipboard_read_value_async(clipboard, GDK_TYPE_PIXBUF, 0, nullptr, xoj::util::wrap_v<pasteClipboardImage>,
                                       this->listener);
        return true;
    }
    if (containsText) {
        gdk_clipboard_read_text_async(clipboard, nullptr, xoj::util::wrap_v<pasteClipboardText>, this->listener);
        return true;
    }
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

static auto svgWriteFunction(GString* string, const unsigned char* data, unsigned int length) -> cairo_status_t {
    g_string_append_len(string, reinterpret_cast<const gchar*>(data), length);
    return CAIRO_STATUS_SUCCESS;
}

auto ClipboardHandler::copy() -> bool {
    if (!this->selection) {
        return false;
    }

    auto xoppBytes = [&]() {
        // prepare xournal++ contents
        ObjectOutputStream out(new BinObjectEncoding());

        out.writeString(PROJECT_STRING);

        this->selection->serialize(out);
        return xoj::util::GBytesSPtr(g_string_free_to_bytes(out.stealData()), xoj::util::adopt);
    }();

    auto text = [&]() {
        // prepare text contents
        std::multiset<Text*, decltype(&ElementCompareFunc)> textElements(ElementCompareFunc);

        for (Element* e: this->selection->getElements()) {
            if (e->getType() == ELEMENT_TEXT) {
                textElements.insert(dynamic_cast<Text*>(e));
            }
        }

        std::string text{};
        for (Text* t: textElements) {
            if (!text.empty()) {
                text += "\n";
            }
            text += t->getText();
        }
        return text;
    }();

    auto image = [&]() {
        // prepare image contents: PNG
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

        xoj::util::GObjectSPtr<GdkPixbuf> image(gdk_pixbuf_get_from_surface(surfacePng, 0, 0, width, height),
                                                xoj::util::adopt);

        cairo_surface_destroy(surfacePng);
        return image;
    }();

    auto svgBytes = [&]() {
        // prepare image contents: SVG
        GString* svgString = g_string_sized_new(1048576);  // 1MB

        cairo_surface_t* surfaceSVG =
                cairo_svg_surface_create_for_stream(reinterpret_cast<cairo_write_func_t>(svgWriteFunction), svgString,
                                                    selection->getWidth(), selection->getHeight());
        cairo_t* crSVG = cairo_create(surfaceSVG);

        cairo_translate(crSVG, -selection->getOriginalXOnView(), -selection->getOriginalYOnView());
        xoj::view::ElementContainerView view(this->selection);
        view.draw(xoj::view::Context::createDefault(crSVG));

        cairo_surface_destroy(surfaceSVG);
        cairo_destroy(crSVG);
        return xoj::util::GBytesSPtr(g_string_free_to_bytes(svgString), xoj::util::adopt);
    }();

    constexpr size_t N_PROVIDERS = 5;
    GdkContentProvider* provs[N_PROVIDERS] = {
            gdk_content_provider_new_typed(GDK_TYPE_PIXBUF, image.get()),
            gdk_content_provider_new_typed(G_TYPE_STRING, text.c_str()),
            gdk_content_provider_new_for_bytes(CLIPBOARD_MIMETYPE_XOPP, xoppBytes.get()),
            gdk_content_provider_new_for_bytes(CLIPBOARD_MIMETYPE_SVG, svgBytes.get()),
            gdk_content_provider_new_for_bytes(CLIPBOARD_MIMETYPE_SVG_XML, svgBytes.get())};
    xoj::util::GObjectSPtr<GdkContentProvider> content(gdk_content_provider_new_union(provs, N_PROVIDERS),
                                                       xoj::util::adopt);
    gdk_clipboard_set_content(clipboard, content.get());

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

void ClipboardHandler::checkFormats() {
    auto* formats = gdk_clipboard_get_formats(clipboard);
    containsXopp = gdk_content_formats_contain_mime_type(formats, CLIPBOARD_MIMETYPE_XOPP);
    containsImage = gdk_content_formats_contain_gtype(formats, GDK_TYPE_PIXBUF);
    containsText = gdk_content_formats_contain_gtype(formats, G_TYPE_STRING);
    listener->clipboardPasteEnabled(containsText || containsXopp || containsImage);
}
