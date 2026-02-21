#include "ColorIcon.h"

#include <iomanip>
#include <memory>
#include <sstream>

extern "C" {
#include <gvdb/gvdb-builder.h>
}

#include "gui/toolbarMenubar/model/ColorPalette.h"
#include "util/Recolor.h"
#include "util/glib_casts.h"
#include "util/raii/GObjectSPtr.h"
#include "util/serdesstream.h"

namespace ColorIcon {

static constexpr auto ICON_THEME_RESOURCE_PATH = "/org/xournalpp/colors/icons/";
static constexpr auto FULL_RESOURCE_PATH = "/org/xournalpp/colors/icons/scalable/actions/";

static constexpr auto CIRCLE_ICON_CORE_NAME = "circle";
static constexpr auto SQUARE_ICON_CORE_NAME = "square";
static constexpr auto ICON_EXTENSION = ".svg";

static std::string colorToHex(Color c) {
    auto s = serdes_stream<std::stringstream>();
    s << std::hex << std::setw(6) << std::setfill('0') << (uint32_t(c) & 0x00ffffff);
    return s.str();
};

static GvdbItem* createParent(GHashTable* table) {
    std::string path = FULL_RESOURCE_PATH;
    auto* parent = gvdb_hash_table_insert(table, path.c_str());
    // We need to add the whole hierarchy
    auto* ancestor = parent;
    for (auto it = std::next(path.rbegin()); it != path.rend(); it++) {
        if (*it == '/') {
            it[-1] = '\0';  // The c-string ends here now
            auto* item = gvdb_hash_table_insert(table, path.c_str());
            gvdb_item_set_parent(ancestor, item);
            ancestor = item;
        }
    }
    return parent;
}

void createPaletteIconResources(const Palette& palette, const std::optional<Recolor>& recolor) {
    // Create the suitable GResource
    auto* table = gvdb_hash_table_new(nullptr, nullptr);
    auto* parent = createParent(table);

    auto makeSVG = [](const std::string& colorAsHex, const std::string& secColorAsHex, bool circle) {
        auto stream = serdes_stream<std::stringstream>();

        stream << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"48\" height=\"48\" stroke-width=\"4\" "
                  "stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke=\"#000000\" stroke-opacity=\"1\">";
        if (circle) {
            stream << "<circle cx=\"24\" cy=\"24\" r=\"20\" ";
        } else {
            stream << "<rect width=\"40\" height=\"40\" x=\"4\" y=\"4\" rx=\"2\" ry=\"2\" ";
        }
        stream << "style=\"fill:#" << colorAsHex << "\"/>";

        if (!secColorAsHex.empty()) {
            stream << "<path d=\"M 46 22 A 22 22 0 0 0 22 46 L 46 46 Z\" style=\"fill:#" << secColorAsHex << "\"/>";
        }
        stream << "</svg>";
        return stream.str();
    };

    auto makeGVariant = [](std::string&& content) {
        auto c = std::make_unique<std::string>(std::move(content));
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("(uuay)"));
        g_variant_builder_add(&builder, "u", content.size()); /* Size */
        g_variant_builder_add(&builder, "u", 0);              /* Flags (compression or so) - none for us */
        g_variant_builder_add_value(&builder,
                                    g_variant_new_from_data(G_VARIANT_TYPE("ay"), c->c_str(), c->size() + 1, TRUE,
                                                            xoj::util::destroy_cb<std::string>, c.get()));
        c.release();  // Now owned by the GVariant
        return g_variant_builder_end(&builder);
    };

    for (const auto& c: palette.getColors()) {
        auto colorAsHex = colorToHex(c.getColor());
        auto key = FULL_RESOURCE_PATH + colorAsHex + CIRCLE_ICON_CORE_NAME + ICON_EXTENSION;
        auto* item = gvdb_hash_table_insert(table, key.c_str());
        gvdb_item_set_value(item, makeGVariant(makeSVG(colorAsHex, "", true)));
        gvdb_item_set_parent(item, parent);
        // And the square icon
        key = FULL_RESOURCE_PATH + colorAsHex + SQUARE_ICON_CORE_NAME + ICON_EXTENSION;
        item = gvdb_hash_table_insert(table, key.c_str());
        gvdb_item_set_value(item, makeGVariant(makeSVG(colorAsHex, "", false)));
        gvdb_item_set_parent(item, parent);
    }
    if (recolor) {
        // Also add icons with the converted color in a corner
        // We still need the normal icons in various places (e.g. Toolbar customization or Palette settings)
        for (const auto& c: palette.getColors()) {
            auto colorAsHex = colorToHex(c.getColor());
            auto secColorAsHex = colorToHex(recolor->convertColor(c.getColor()));
            auto key = FULL_RESOURCE_PATH + colorAsHex + CIRCLE_ICON_CORE_NAME + secColorAsHex + ICON_EXTENSION;
            auto* item = gvdb_hash_table_insert(table, key.c_str());
            gvdb_item_set_value(item, makeGVariant(makeSVG(colorAsHex, secColorAsHex, true)));
            gvdb_item_set_parent(item, parent);
        }
    }

    // Now add our icons to the available resources
    GBytes* bytes = gvdb_table_get_contents(table, G_BYTE_ORDER != G_LITTLE_ENDIAN);
    g_hash_table_destroy(table);

    GError* err = nullptr;
    GResource* res = g_resource_new_from_data(bytes, &err);
    g_bytes_unref(bytes);
    g_resources_register(res);
    g_resource_unref(res);
    if (err) {
        g_warning("Error when creating color icon cache: %s\n", err->message);
    }

    // By calling this we ensure the path is known AND make the theme update itself
    gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), ICON_THEME_RESOURCE_PATH);
}

static std::string getName(Color color, std::optional<Color> secondaryColor, bool circle) {
    auto colorAsHex = colorToHex(color);
    auto secColorAsHex = secondaryColor ? colorToHex(secondaryColor.value()) : std::string();
    return colorAsHex + (circle ? CIRCLE_ICON_CORE_NAME : SQUARE_ICON_CORE_NAME) + secColorAsHex;
}

/**
 * Create a new GtkImage with preview color
 */
auto newGtkImage(Color color, int size, bool circle, std::optional<Color> secondaryColor) -> GtkWidget* {
    auto name = getName(color, secondaryColor, circle);
    GtkWidget* w = gtk_image_new_from_icon_name(name.c_str(),
                                                circle ? GTK_ICON_SIZE_SMALL_TOOLBAR : GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_widget_show(w);
    return w;
}

/**
 * Create a new GdkPixbuf* with preview color
 */
auto newGdkPixbuf(Color color, int size, bool circle, std::optional<Color> secondaryColor)
        -> xoj::util::GObjectSPtr<GdkPixbuf> {
    auto name = getName(color, secondaryColor, circle);
    return xoj::util::GObjectSPtr<GdkPixbuf>(
            gdk_pixbuf_new_from_resource_at_scale((FULL_RESOURCE_PATH + name + ICON_EXTENSION).c_str(), size, size,
                                                  false, nullptr),
            xoj::util::adopt);
}
};  // namespace ColorIcon
