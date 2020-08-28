#include "Util.h"

#include <array>
#include <cassert>
#include <cstdlib>

#include <unistd.h>

#include "Color.h"
#include "PathUtil.h"
#include "StringUtils.h"
#include "XojMsgBox.h"
#include "i18n.h"

struct CallbackUiData {
    explicit CallbackUiData(std::function<void()> callback): callback(std::move(callback)) {}

    std::function<void()> callback;  // NOLINT
};

/**
 * This method is called in the GTK UI Thread
 */
static auto execInUiThreadCallback(CallbackUiData* cb) -> bool {
    cb->callback();

    delete cb;
    // Do not call again
    return false;
}

/**
 * Execute the callback in the UI Thread.
 *
 * Make sure the container class is not deleted before the UI stuff is finished!
 */
void Util::execInUiThread(std::function<void()>&& callback) {
    gdk_threads_add_idle(reinterpret_cast<GSourceFunc>(execInUiThreadCallback),
                         new CallbackUiData(std::move(callback)));
}

void Util::cairo_set_source_rgbi(cairo_t* cr, Color color) {
    auto rgba = rgb_to_GdkRGBA(color);
    gdk_cairo_set_source_rgba(cr, &rgba);
}

void Util::cairo_set_source_rgbi(cairo_t* cr, Color color, double alpha) {
    auto rgba = argb_to_GdkRGBA(color, alpha);
    gdk_cairo_set_source_rgba(cr, &rgba);
}

auto Util::getPid() -> pid_t { return ::getpid(); }


auto Util::paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void*) -> gboolean {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
    cairo_fill(cr);
    return false;
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
        string msg = FS(_F("Error {1} executing system command: {2}") % errc % command);
        XojMsgBox::showErrorToUser(nullptr, msg);
    }
}
