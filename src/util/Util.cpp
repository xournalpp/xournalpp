#include "Util.h"

#include <array>
#include <cstdlib>
#include <utility>

#include <unistd.h>

#include "PathUtil.h"
#include "StringUtils.h"
#include "XojMsgBox.h"
#include "config-dev.h"
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

auto Util::rgb_to_GdkRGBA(const uint32_t color) -> GdkRGBA { return Util::argb_to_GdkRGBA(0xFF000000U | color); }

auto Util::argb_to_GdkRGBA(const uint32_t color) -> GdkRGBA {
    // clang-format off
    return {((color >> 16U) & 0xFFU) / 255.0,
            ((color >> 8U) & 0xFFU) / 255.0,
            (color & 0xFFU) / 255.0,
            ((color >> 24U) & 0xFFU) / 255.0};
    // clang-format on
}

void Util::cairo_set_source_rgbi(cairo_t* cr, int color) {
    auto rgba = rgb_to_GdkRGBA(color);
    cairo_set_source_rgb(cr, rgba.red, rgba.green, rgba.blue);
}

// Splits the double into a equal sized distribution between [0,256[ and rounding down
// inspired by, which isn't completely correct:
// https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255
constexpr double MAXCOLOR = 256.0 - std::numeric_limits<double>::epsilon() * 128;

inline auto float_to_int_color(const double color) -> uint32_t {
    static_assert(MAXCOLOR < 256.0, "MAXCOLOR isn't smaler than 256");
    return static_cast<uint32_t>(color * MAXCOLOR);
}

auto Util::gdkrgba_to_hex(const GdkRGBA& color) -> uint32_t {  // clang-format off
	return float_to_int_color(color.alpha) << 24U |
	       float_to_int_color(color.red)  << 16U |
	       float_to_int_color(color.green) << 8U |
	       float_to_int_color(color.blue);
                                                               // clang-format on
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
