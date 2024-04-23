#include "ColorIcon.h"

#include <cmath>  // for M_PI
#include <iomanip>
#include <sstream>

#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"
#include "util/serdesstream.h"

static auto newGdkPixbuf(Color color, bool circle, std::optional<Color> secondaryColor)
        -> xoj::util::GObjectSPtr<GdkPixbuf> {
    auto stream = serdes_stream<std::stringstream>();
    stream << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" height=\"48\" stroke-width=\"4\" "
              "stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke=\"#000000\" stroke-opacity=\"1\">";
    if (circle) {
        stream << "<circle cx=\"24\" cy=\"24\" r=\"20\" ";
    } else {
        stream << "<rect width=\"40\" height=\"40\" x=\"4\" y=\"4\" rx=\"2\" ry=\"2\" ";
    }
    stream << "style=\"fill:#" << std::hex << std::setw(6) << std::setfill('0') << (uint32_t(color) & 0x00ffffff)
           << "\"/>";

    if (secondaryColor) {
        stream << "<path d=\"M 46 22 A 22 22 0 0 0 22 46 L 46 46 Z\" "
               << "style=\"fill:#" << std::hex << std::setw(6) << std::setfill('0')
               << (uint32_t(*secondaryColor) & 0x00ffffff) << "\"/>";
    }
    stream << "</svg>";

    std::string str = stream.str();  // Keep this alive as long as gstream is
    xoj::util::GObjectSPtr<GInputStream> gstream(g_memory_input_stream_new_from_data(str.c_str(), -1, nullptr),
                                                 xoj::util::adopt);

    return xoj::util::GObjectSPtr<GdkPixbuf>(gdk_pixbuf_new_from_stream(gstream.get(), nullptr, nullptr),
                                             xoj::util::adopt);
}

namespace ColorIcon {
/**
 * Create a new GtkImage with preview color
 */
auto newGtkImage(Color color, bool circle, std::optional<Color> secondaryColor) -> GtkWidget* {
    return gtk_image_new_from_pixbuf(newGdkPixbuf(color, circle, secondaryColor).get());
}

/**
 * Create a new GdkPaintable* with preview color
 */
auto newGdkPaintable(Color color, bool circle, std::optional<Color> secondaryColor)
        -> xoj::util::GObjectSPtr<GdkPaintable> {
    return xoj::util::GObjectSPtr<GdkPaintable>(
            GDK_PAINTABLE(gdk_texture_new_for_pixbuf(newGdkPixbuf(color, circle, secondaryColor).get())),
            xoj::util::adopt);
}
};  // namespace ColorIcon
