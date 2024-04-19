#include "util/Util.h"

#include <array>    // for array
#include <cstdlib>  // for system
#include <string>   // for allocator, string
#include <utility>  // for move
#include <vector>   // for vector

#include <gdk/gdk.h>  // for gdk_cairo_set_source_rgba, gdk_t...

#include "util/Color.h"              // for argb_to_GdkRGBA, rgb_to_GdkRGBA
#include "util/OutputStream.h"       // for OutputStream
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/XojMsgBox.h"          // for XojMsgBox
#include "util/i18n.h"               // for FS, _F

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <unistd.h>  // for getpid, pid_t
#endif


void Util::cairo_set_source_rgbi(cairo_t* cr, Color color, double alpha) {
    auto rgba = argb_to_GdkRGBA(color, alpha);
    gdk_cairo_set_source_rgba(cr, &rgba);
}

void Util::cairo_set_source_argb(cairo_t* cr, Color color) {
    auto rgba = argb_to_GdkRGBA(color);
    gdk_cairo_set_source_rgba(cr, &rgba);
}

auto Util::getPid() -> PID {
#if defined(_MSC_VER)
    return GetCurrentProcessId();
#else
    return ::getpid();
#endif
}

auto Util::paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void*) -> gboolean {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
    cairo_fill(cr);
    return false;
}

void Util::cairo_set_dash_from_vector(cairo_t* cr, const std::vector<double>& dashes, double offset) {
    cairo_set_dash(cr, dashes.data(), static_cast<int>(dashes.size()), offset);
}

void Util::writeCoordinateString(OutputStream* out, double xVal, double yVal) {
    std::array<char, G_ASCII_DTOSTR_BUF_SIZE> coordString{};
    g_ascii_formatd(coordString.data(), G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, xVal);
    out->write(coordString.data());
    out->write(" ");
    g_ascii_formatd(coordString.data(), G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, yVal);
    out->write(coordString.data());
}

void Util::systemWithMessage(const char* command) {
    if (auto errc = std::system(command); errc != 0) {
        std::string msg = FS(_F("Error {1} executing system command: {2}") % errc % command);
        XojMsgBox::showErrorToUser(nullptr, msg);
    }
}

bool Util::isFlatpakInstallation() { return fs::exists("/.flatpak-info"); }
