#include "ClipboardHandler.h"

#include <set>      // for multiset, operator!=
#include <utility>  // for move
#include <vector>   // for vector

#include <cairo-svg.h>      // for cairo_svg_surface_c...
#include <cairo.h>          // for cairo_create, cairo...
#include <gdk/gdkpixbuf.h>  // for gdk_pixbuf_get_from_surface
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

#ifdef G_OS_WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using std::string;

ClipboardListener::~ClipboardListener() = default;

ClipboardHandler::ClipboardHandler(ClipboardListener* listener, GtkWidget* widget) {
    this->listener = listener;
    this->clipboard = gtk_widget_get_clipboard(widget);

    this->handlerId = g_signal_connect(this->clipboard, "owner-change", G_CALLBACK(&ownerChangedCallback), this);

    this->listener->clipboardCutCopyEnabled(false);

    gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
                                   reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
}

ClipboardHandler::~ClipboardHandler() { g_signal_handler_disconnect(this->clipboard, this->handlerId); }

static GdkAtom atomXournal = gdk_atom_intern_static_string("application/xournal");

auto ClipboardHandler::paste() -> bool {
    /* The content flags below may be outdated: the owner-change signal is not emitted at all on MacOS and under X11
     * with no XFIXES extension, and is unreliable on Windows as well. So request the target list again and dispatch
     * the paste from receivedClipboardContents, once the fresh flags are available.
     * See https://docs.gtk.org/gdk3/struct.EventOwnerChange.html and
     * https://gitlab.gnome.org/GNOME/gtk/-/issues/1757 */
    this->pastePending = true;
    gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
                                   reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
    return true;
}

auto ClipboardHandler::cut() -> bool {
    bool result = this->copy();
    this->listener->deleteSelection();

    return result;
}

auto ElementCompareFunc(const Element* a, const Element* b) -> bool {
    const auto& pa = a->getOrigin();
    const auto& pb = b->getOrigin();
    return pa.y < pb.y || (pa.y == pb.y && pa.x < pb.x);
}

static GdkAtom atomSvg1 = gdk_atom_intern_static_string("image/svg");
static GdkAtom atomSvg2 = gdk_atom_intern_static_string("image/svg+xml");

static bool targetsIncludeConvertibleImage(GtkSelectionData* selectionData) {
    GdkAtom* targets = nullptr;
    gint n_targets = 0;
    bool result = false;
    if (gtk_selection_data_get_targets(selectionData, &targets, &n_targets)) {
        for (int i = 0; i < n_targets; i++) {
            if (targets[i] == gdk_atom_intern_static_string("image/png") ||
                targets[i] == gdk_atom_intern_static_string("image/jpeg") ||
                targets[i] == gdk_atom_intern_static_string("image/gif")) {
                result = true;
                break;
            }
        }
        g_free(targets);
    }
    return result;
}

#ifdef G_OS_WIN32
/* GTK3's Win32 backend fails to convert paletted (1/4/8 bpp) CF_DIB data to a GdkPixbuf: the palette is
 * ignored and the resulting image is solid black. This happens e.g. with screenshots copied from
 * FastStone Capture. As long as the clipboard offers no PNG/JPEG/GIF target (which GTK converts
 * correctly), read the raw DIB and convert it ourselves. */
static unsigned int maskShift(unsigned int mask) {
    unsigned int shift = 0;
    while (mask != 0u && (mask & 1u) == 0u) {
        mask >>= 1u;
        shift++;
    }
    return shift;
}

static GdkPixbuf* nativeClipboardImage() {
    if (!OpenClipboard(nullptr)) {
        return nullptr;
    }
    GdkPixbuf* result = nullptr;
    HANDLE handle = GetClipboardData(CF_DIB);
    if (handle == nullptr) {
        handle = GetClipboardData(CF_DIBV5);
    }
    if (handle != nullptr) {
        SIZE_T size = GlobalSize(handle);
        const BITMAPINFOHEADER* bih = static_cast<const BITMAPINFOHEADER*>(GlobalLock(handle));
        if (bih != nullptr && size >= sizeof(BITMAPINFOHEADER) &&
            (bih->biCompression == BI_RGB || bih->biCompression == BI_BITFIELDS)) {
            const int width = bih->biWidth;
            const int height = bih->biHeight < 0 ? -bih->biHeight : bih->biHeight;
            const int bpp = bih->biBitCount;
            const bool bottomUp = bih->biHeight > 0;
            if (width > 0 && height > 0 && (bpp == 1 || bpp == 4 || bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32)) {
                const auto* base = reinterpret_cast<const BYTE*>(bih);
                unsigned int redMask = 0, greenMask = 0, blueMask = 0, alphaMask = 0;
                unsigned int paletteEntries = 0;
                if (bpp <= 8) {
                    paletteEntries = bih->biClrUsed != 0 ? bih->biClrUsed : (1u << bpp);
                }
                unsigned int offset = bih->biSize + paletteEntries * 4;
                if (bpp == 32 || bpp == 16) {
                    if (bih->biCompression == BI_BITFIELDS) {
                        if (bih->biSize >= 108) {  // BITMAPV4HEADER and up: masks are part of the header
                            const auto* v4 = reinterpret_cast<const BITMAPV4HEADER*>(bih);
                            redMask = v4->bV4RedMask;
                            greenMask = v4->bV4GreenMask;
                            blueMask = v4->bV4BlueMask;
                            alphaMask = v4->bV4AlphaMask;
                        } else {  // classic 40-byte header: 3 DWORD masks follow it
                            memcpy(&redMask, base + offset, 4);
                            memcpy(&greenMask, base + offset + 4, 4);
                            memcpy(&blueMask, base + offset + 8, 4);
                            offset += 12;
                        }
                    }
                    if (redMask == 0u || greenMask == 0u || blueMask == 0u) {
                        redMask = bpp == 32 ? 0x00FF0000u : 0x7C00u;
                        greenMask = bpp == 32 ? 0x0000FF00u : 0x03E0u;
                        blueMask = bpp == 32 ? 0x000000FFu : 0x001Fu;
                    }
                }
                const unsigned int rShift = maskShift(redMask), gShift = maskShift(greenMask);
                const unsigned int bShift = maskShift(blueMask), aShift = maskShift(alphaMask);
                const LONG stride = ((static_cast<LONG>(width) * bpp + 31) / 32) * 4;
                if (offset + static_cast<unsigned int>(stride) * height <= size) {
                    const bool hasAlpha = bpp == 32 && alphaMask != 0u;
                    result = gdk_pixbuf_new(GDK_COLORSPACE_RGB, hasAlpha, 8, width, height);
                    if (result != nullptr) {
                        const BYTE* pixels = base + offset;
                        auto* dst = reinterpret_cast<BYTE*>(gdk_pixbuf_get_pixels(result));
                        const int dstStride = gdk_pixbuf_get_rowstride(result);
                        const int nCh = gdk_pixbuf_get_n_channels(result);
                        for (int y = 0; y < height; y++) {
                            const BYTE* src =
                                    pixels + static_cast<unsigned int>(bottomUp ? height - 1 - y : y) * stride;
                            BYTE* out = dst + static_cast<unsigned int>(y) * dstStride;
                            for (int x = 0; x < width; x++) {
                                switch (bpp) {
                                    case 1:
                                    case 4:
                                    case 8: {
                                        const unsigned int bitPos = bpp == 8 ? 0 : (bpp == 4 ? 4 * (x % 2) : 7 - x % 8);
                                        const unsigned int idx = (src[x * bpp / 8] >> bitPos) & ((1u << bpp) - 1u);
                                        const BYTE* entry = base + bih->biSize + idx * 4;
                                        out[x * nCh + 0] = entry[2];
                                        out[x * nCh + 1] = entry[1];
                                        out[x * nCh + 2] = entry[0];
                                        break;
                                    }
                                    case 16: {
                                        unsigned int v;
                                        memcpy(&v, src + x * 2, 2);
                                        const unsigned int rMax = redMask >> rShift, gMax = greenMask >> gShift;
                                        const unsigned int bMax = blueMask >> bShift;
                                        out[x * nCh + 0] =
                                                static_cast<BYTE>((((v & redMask) >> rShift) * 255u + rMax / 2) / rMax);
                                        out[x * nCh + 1] = static_cast<BYTE>(
                                                (((v & greenMask) >> gShift) * 255u + gMax / 2) / gMax);
                                        out[x * nCh + 2] = static_cast<BYTE>(
                                                (((v & blueMask) >> bShift) * 255u + bMax / 2) / bMax);
                                        break;
                                    }
                                    case 24: {
                                        out[x * nCh + 0] = src[x * 3 + 2];
                                        out[x * nCh + 1] = src[x * 3 + 1];
                                        out[x * nCh + 2] = src[x * 3 + 0];
                                        break;
                                    }
                                    case 32: {
                                        unsigned int v;
                                        memcpy(&v, src + x * 4, 4);
                                        out[x * nCh + 0] = static_cast<BYTE>((v & redMask) >> rShift);
                                        out[x * nCh + 1] = static_cast<BYTE>((v & greenMask) >> gShift);
                                        out[x * nCh + 2] = static_cast<BYTE>((v & blueMask) >> bShift);
                                        if (nCh == 4) {
                                            out[x * nCh + 3] = alphaMask != 0u ?
                                                                       static_cast<BYTE>((v & alphaMask) >> aShift) :
                                                                       255u;
                                        }
                                        break;
                                    }
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (bih != nullptr) {
            GlobalUnlock(handle);
        }
    }
    CloseClipboard();
    return result;
}
#endif  // G_OS_WIN32

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


    static void getFunction(GtkClipboard* clipboard, GtkSelectionData* selection, guint info,
                            ClipboardContents* contents) {
        GdkAtom target = gtk_selection_data_get_target(selection);

        if (target == gdk_atom_intern_static_string("UTF8_STRING")) {
            gtk_selection_data_set_text(selection, contents->text.c_str(), -1);
        } else if (target == gdk_atom_intern_static_string("image/png") ||
                   target == gdk_atom_intern_static_string("image/jpeg") ||
                   target == gdk_atom_intern_static_string("image/gif")) {
            gtk_selection_data_set_pixbuf(selection, contents->image);
        } else if (atomSvg1 == target || atomSvg2 == target) {
            gtk_selection_data_set(selection, target, 8, reinterpret_cast<guchar const*>(contents->svg.c_str()),
                                   static_cast<gint>(contents->svg.length()));
        } else if (atomXournal == target) {
            gtk_selection_data_set(selection, target, 8, reinterpret_cast<guchar*>(contents->str->str),
                                   static_cast<gint>(contents->str->len));
        }
    }

    static void clearFunction(GtkClipboard* clipboard, ClipboardContents* contents) { delete contents; }

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

    std::multiset<const Text*, decltype(&ElementCompareFunc)> textElements(ElementCompareFunc);

    for (const Element* e: this->selection->getElementsView()) {
        if (e->getType() == ELEMENT_TEXT) {
            textElements.insert(dynamic_cast<const Text*>(e));
        }
    }

    string text{};
    for (const Text* t: textElements) {
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

    GtkTargetList* list = gtk_target_list_new(nullptr, 0);
    GtkTargetEntry* targets = nullptr;
    int n_targets = 0;

    // if we have text elements...
    if (!text.empty()) {
        gtk_target_list_add_text_targets(list, 0);
    }
    // we always copy an image to clipboard
    gtk_target_list_add_image_targets(list, 0, true);
    gtk_target_list_add(list, atomSvg1, 0, 0);
    gtk_target_list_add(list, atomSvg2, 0, 0);
    gtk_target_list_add(list, atomXournal, 0, 0);

    targets = gtk_target_table_new_from_list(list, &n_targets);

    auto* contents = new ClipboardContents(text, image, svgString->str, out.stealData());

    gtk_clipboard_set_with_data(this->clipboard, targets, static_cast<guint>(n_targets),
                                reinterpret_cast<GtkClipboardGetFunc>(ClipboardContents::getFunction),
                                reinterpret_cast<GtkClipboardClearFunc>(ClipboardContents::clearFunction), contents);
    gtk_clipboard_set_can_store(this->clipboard, nullptr, 0);

    gtk_target_table_free(targets, n_targets);
    gtk_target_list_unref(list);

    g_string_free(svgString, true);

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

void ClipboardHandler::ownerChangedCallback(GtkClipboard* clip, GdkEvent* event, ClipboardHandler* handler) {
    if (gdk_event_get_event_type(event) == GDK_OWNER_CHANGE) {
        handler->clipboardUpdated(event->owner_change.selection);
    }
}

void ClipboardHandler::clipboardUpdated(GdkAtom atom) {
    gtk_clipboard_request_contents(clipboard, gdk_atom_intern_static_string("TARGETS"),
                                   reinterpret_cast<GtkClipboardReceivedFunc>(receivedClipboardContents), this);
}

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
    handler->containsConvertibleImageTarget = targetsIncludeConvertibleImage(selectionData);

    handler->listener->clipboardPasteEnabled(handler->containsText || handler->containsXournal ||
                                             handler->containsImage);

    if (handler->pastePending) {
        handler->pastePending = false;
        handler->dispatchPaste();
    }
}

void ClipboardHandler::dispatchPaste() {
    // Priority: xournal native format > text > image
    if (this->containsXournal) {
        gtk_clipboard_request_contents(this->clipboard, atomXournal,
                                       reinterpret_cast<GtkClipboardReceivedFunc>(pasteClipboardContents), this);
    } else if (this->containsText) {
        gtk_clipboard_request_text(this->clipboard, reinterpret_cast<GtkClipboardTextReceivedFunc>(pasteClipboardText),
                                   this);
    } else if (this->containsImage) {
#ifdef G_OS_WIN32
        if (!this->containsConvertibleImageTarget) {
            // GTK's Win32 backend renders paletted DIBs black - use the native conversion instead
            GdkPixbuf* img = nativeClipboardImage();
            if (img != nullptr) {
                this->listener->clipboardPasteImage(img);
                g_object_unref(img);
                return;
            }
            g_warning("Native clipboard image conversion failed, falling back to GTK");
        }
#endif
        gtk_clipboard_request_image(this->clipboard,
                                    reinterpret_cast<GtkClipboardImageReceivedFunc>(pasteClipboardImage), this);
    }
}
