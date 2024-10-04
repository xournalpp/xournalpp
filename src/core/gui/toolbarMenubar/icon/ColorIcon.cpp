#include "ColorIcon.h"

#include <cmath>  // for M_PI
#include <iomanip>
#include <sstream>

#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"
#include "util/serdesstream.h"

static auto newGdkPixbuf(Color color, bool circle) -> xoj::util::GObjectSPtr<GdkPixbuf> {
    auto stream = serdes_stream<std::stringstream>();
    stream << "<svg width=\"24\" height=\"24\" stroke-width=\"2\" stroke-linecap=\"round\" "
              "stroke-linejoin=\"round\" xmlns=\"http://www.w3.org/2000/svg\">";
    if (circle) {
        stream << "<circle cx=\"12\" cy=\"12\" r=\"10\" ";
    } else {
        stream << "<rect width=\"20\" height=\"20\" x=\"2\" y=\"2\" rx=\"1\" ry=\"1\" ";
    }
    stream << "style=\"stroke:#000000;stroke-opacity:1;fill:#" << std::hex << std::setw(6) << std::setfill('0')
           << (uint32_t(color) & 0x00ffffff) << "\"/></svg>";

    std::string str = stream.str();  // Keep this alive as long as gstream is
    xoj::util::GObjectSPtr<GInputStream> gstream(g_memory_input_stream_new_from_data(str.c_str(), -1, nullptr),
                                                 xoj::util::adopt);

    return xoj::util::GObjectSPtr<GdkPixbuf>(gdk_pixbuf_new_from_stream(gstream.get(), NULL, NULL), xoj::util::adopt);
}

namespace ColorIcon {
/**
 * Create a new GtkImage with preview color
 */
auto newGtkImage(Color color, bool circle) -> GtkWidget* {
    return gtk_image_new_from_pixbuf(newGdkPixbuf(color, circle).get());
}

/**
 * Create a new GdkPaintable* with preview color
 */
auto newGdkPaintable(Color color, bool circle) -> xoj::util::GObjectSPtr<GdkPaintable> {
    return xoj::util::GObjectSPtr<GdkPaintable>(
            GDK_PAINTABLE(gdk_texture_new_for_pixbuf(newGdkPixbuf(color, circle).get())), xoj::util::adopt);
}
};  // namespace ColorIcon
