#include "PageTypeHandler.h"

#include <algorithm>
#include <utility>

#include "gui/GladeSearchpath.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"

static void addPageTypeInfo(const std::string& name, PageTypeFormat format, const std::string& config,
                            std::vector<std::unique_ptr<PageTypeInfo>>& types) {
    auto pt = std::make_unique<PageTypeInfo>();
    pt->name = std::move(name);
    pt->page.format = format;
    pt->page.config = std::move(config);

    types.emplace_back(std::move(pt));
}

PageTypeHandler::PageTypeHandler(GladeSearchpath* gladeSearchPath) {
    auto file = gladeSearchPath->findFile("", "pagetemplates.ini");

    if (!parseIni(file) || this->types.size() < 5) {

        std::string msg = FS(_F("Could not load pagetemplates.ini file"));
        XojMsgBox::showErrorToUser(nullptr, msg);

        // On failure load the hardcoded and predefined values
        addPageTypeInfo(_("Plain"), PageTypeFormat::Plain, "", types);
        addPageTypeInfo(_("Ruled"), PageTypeFormat::Ruled, "", types);
        addPageTypeInfo(_("Ruled with vertical line"), PageTypeFormat::Lined, "", types);
        addPageTypeInfo(_("Staves"), PageTypeFormat::Staves, "", types);
        addPageTypeInfo(_("Graph"), PageTypeFormat::Graph, "", types);
        addPageTypeInfo(_("Dotted"), PageTypeFormat::Dotted, "", types);
        addPageTypeInfo(_("Isometric Dotted"), PageTypeFormat::IsoDotted, "", types);
        addPageTypeInfo(_("Isometric Graph"), PageTypeFormat::IsoGraph, "", types);
    }

    // Special types
    addPageTypeInfo(_("With PDF background"), PageTypeFormat::Pdf, "", specialTypes);
    addPageTypeInfo(_("Image"), PageTypeFormat::Image, "", specialTypes);
}

PageTypeHandler::~PageTypeHandler() = default;

auto PageTypeHandler::parseIni(fs::path const& filepath) -> bool {
    GKeyFile* config = g_key_file_new();
    g_key_file_set_list_separator(config, ',');
    if (!g_key_file_load_from_file(config, filepath.u8string().c_str(), G_KEY_FILE_NONE, nullptr)) {
        g_key_file_free(config);
        return false;
    }

    gsize length = 0;
    gchar** groups = g_key_file_get_groups(config, &length);

    for (gsize i = 0; i < length; i++) { loadFormat(config, groups[i]); }

    g_strfreev(groups);
    g_key_file_free(config);
    return true;
}

void PageTypeHandler::loadFormat(GKeyFile* config, const char* group) {
    std::string strName;
    gchar* name = g_key_file_get_locale_string(config, group, "name", nullptr, nullptr);
    if (name != nullptr) {
        strName = name;
        g_free(name);
    }

    std::string strFormat;
    gchar* format = g_key_file_get_string(config, group, "format", nullptr);
    if (format != nullptr) {
        strFormat = format;
        g_free(format);
    }

    std::string strConfig;
    gchar* cconfig = g_key_file_get_string(config, group, "config", nullptr);
    if (cconfig != nullptr) {
        strConfig = cconfig;
        g_free(cconfig);
    }

    addPageTypeInfo(strName, getPageTypeFormatForString(strFormat), strConfig, types);
}

auto PageTypeHandler::getPageTypes() -> const std::vector<std::unique_ptr<PageTypeInfo>>& { return this->types; }
auto PageTypeHandler::getSpecialPageTypes() -> const std::vector<std::unique_ptr<PageTypeInfo>>& {
    return this->specialTypes;
}

auto PageTypeHandler::getInfoOn(const PageType& pt) const -> const PageTypeInfo* {
    const auto& vector = pt.isSpecial() ? specialTypes : types;
    auto it = std::find_if(vector.begin(), vector.end(), [&](const auto& info) { return info->page == pt; });
    return it == vector.end() ? nullptr : it->get();
}

auto PageTypeHandler::getPageTypeFormatForString(const std::string& format) -> PageTypeFormat {
    if (format == "plain") {
        return PageTypeFormat::Plain;
    }
    if (format == "ruled") {
        return PageTypeFormat::Ruled;
    }
    if (format == "lined") {
        return PageTypeFormat::Lined;
    }
    if (format == "staves") {
        return PageTypeFormat::Staves;
    }
    if (format == "graph") {
        return PageTypeFormat::Graph;
    }
    if (format == "dotted") {
        return PageTypeFormat::Dotted;
    }
    if (format == "isodotted") {
        return PageTypeFormat::IsoDotted;
    }
    if (format == "isograph") {
        return PageTypeFormat::IsoGraph;
    }
    if (format == ":pdf") {
        return PageTypeFormat::Pdf;
    }
    if (format == ":image") {
        return PageTypeFormat::Image;
    }
    g_warning("PageTypeHandler::getPageTypeFormatForString: unknown PageType: \"%s\". Replacing with "
              "PageTypeFormat::Plain",
              format.c_str());
    return PageTypeFormat::Plain;
}

auto PageTypeHandler::getStringForPageTypeFormat(const PageTypeFormat& format) -> std::string {
    switch (format) {
        case PageTypeFormat::Plain:
            return "plain";
        case PageTypeFormat::Ruled:
            return "ruled";
        case PageTypeFormat::Lined:
            return "lined";
        case PageTypeFormat::Staves:
            return "staves";
        case PageTypeFormat::Graph:
            return "graph";
        case PageTypeFormat::Dotted:
            return "dotted";
        case PageTypeFormat::IsoDotted:
            return "isodotted";
        case PageTypeFormat::IsoGraph:
            return "isograph";
        case PageTypeFormat::Pdf:
            return ":pdf";
        case PageTypeFormat::Image:
            return ":image";
    }
    g_warning("PageTypeHandler::getStringForPageTypeFormat: unknown PageType: %d. Replacing with "
              "PageTypeFormat::Ruled",
              static_cast<int>(format));
    return "ruled";
}
