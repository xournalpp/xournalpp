#include "ColorIcon.h"

#include <cmath>  // for M_PI
#include <iomanip>
#include <sstream>

#include "util/raii/CairoWrappers.h"
#include "util/raii/GObjectSPtr.h"
#include "util/serdesstream.h"

namespace ColorIcon {
/**
 * Create a new GtkImage with preview color
 */
auto newGtkImage(Color color, bool circle) -> GtkWidget* {
    xoj::util::GObjectSPtr<GdkPixbuf> img(newGdkPixbuf(color, circle));
    GtkWidget* w = gtk_image_new_from_pixbuf(img.get());
    return w;
}

/**
 * Create a new GdkPixbuf* with preview color
 */
auto newGdkPixbuf(Color color, bool circle) -> xoj::util::GObjectSPtr<GdkPixbuf> {
    auto stream = serdes_stream<std::stringstream>();
    stream << "<svg width=\"240\" height=\"240\" stroke-width=\"20\" stroke-linecap=\"round\" "
              "stroke-linejoin=\"round\" xmlns=\"http://www.w3.org/2000/svg\">";
    if (circle) {
        stream << "<circle cx=\"120\" cy=\"120\" r=\"100\" ";
    } else {
        stream << "<rect width=\"200\" height=\"200\" x=\"20\" y=\"20\" rx=\"10\" ry=\"10\" ";
    }
    stream << "style=\"stroke:#000000;stroke-opacity:1;fill:#" << std::hex << std::setw(6) << std::setfill('0')
           << (uint32_t(color) & 0x00ffffff) << "\"/></svg>";

    std::string str = stream.str();  // Keep this alive as long as gstream is
    xoj::util::GObjectSPtr<GInputStream> gstream(g_memory_input_stream_new_from_data(str.c_str(), -1, nullptr),
                                                 xoj::util::adopt);

    return xoj::util::GObjectSPtr<GdkPixbuf>(gdk_pixbuf_new_from_stream(gstream.get(), NULL, NULL), xoj::util::adopt);
}
};  // namespace ColorIcon
