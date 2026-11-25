#pragma once

#include <cstdint>  // for uint8_t
#include <memory>   // for unique_ptr

#include <gdk/gdk.h>  // for GdkRectangle
#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkButton, GtkOverlay

#include "pdf/base/XojPdfPage.h"    // for XojPdfPageSelectionStyle
#include "util/Color.h"             // for Color
#include "util/raii/GObjectSPtr.h"  // for GObjectSPtr

class MainWindow;
class XojPageView;
class PdfElemSelection;

class PdfFloatingToolbox {
public:
    PdfFloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay);
    PdfFloatingToolbox& operator=(const PdfFloatingToolbox&) = delete;
    PdfFloatingToolbox(const PdfFloatingToolbox&) = delete;
    PdfFloatingToolbox& operator=(PdfFloatingToolbox&&) = delete;
    PdfFloatingToolbox(PdfFloatingToolbox&&) = delete;
    ~PdfFloatingToolbox();

public:
    /// Show the toolbox at the provided coordinates (in widget-coordinates).
    void show(int x, int y, PdfElemSelection* selection);

    /// Hide the floating toolbox widget.
    void hide();

    /// Copy Selection to the Clipboard.
    void copyTextToClipboard();

private:
    void show();

    static gboolean getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                       PdfFloatingToolbox* self);


    /// Generic callback for the next 3 functions
    template <auto fun>
    void cb();

    static void strikethroughCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void underlineCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void highlightCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void copyTextCb(GtkButton* button, PdfFloatingToolbox* pft);

private:
    GtkWidget* floatingToolbox;
    MainWindow* theMainWindow;

    /// The overlay that the toolbox should be displayed in.
    xoj::util::GObjectSPtr<GtkOverlay> overlay;

    struct {
        int x;
        int y;
    } position;
    PdfElemSelection* selection = nullptr;  ///< The selected text the toolbox is shown for
};
