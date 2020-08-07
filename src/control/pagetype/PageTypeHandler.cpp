#include "PageTypeHandler.h"

#include <utility>

#include "gui/GladeSearchpath.h"

#include "XojMsgBox.h"
#include "i18n.h"

PageTypeHandler::PageTypeHandler(GladeSearchpath* gladeSearchPath) {
    auto file = gladeSearchPath->findFile("", "pagetemplates.ini");

    if (!parseIni(file) || this->types.size() < 5) {

        string msg = FS(_F("Could not load pagetemplates.ini file"));
        XojMsgBox::showErrorToUser(nullptr, msg);

        // On failure load the hardcoded and predefined values
        addPageTypeInfo(_("Plain"), PageTypeFormat::Plain, "");
        addPageTypeInfo(_("Ruled"), PageTypeFormat::Ruled, "");
        addPageTypeInfo(_("Ruled with vertical line"), PageTypeFormat::Lined, "");
        addPageTypeInfo(_("Staves"), PageTypeFormat::Staves, "");
        addPageTypeInfo(_("Graph"), PageTypeFormat::Graph, "");
        addPageTypeInfo(_("Dotted"), PageTypeFormat::Dotted, "");
        addPageTypeInfo(_("Isometric Dotted"), PageTypeFormat::IsoDotted, "");
        addPageTypeInfo(_("Isometric Graph"), PageTypeFormat::IsoGraph, "");
    }

    // Special types
    addPageTypeInfo(_("Copy current"), PageTypeFormat::Copy, "");
    addPageTypeInfo(_("With PDF background"), PageTypeFormat::Pdf, "");
    addPageTypeInfo(_("Image"), PageTypeFormat::Image, "");
}

PageTypeHandler::~PageTypeHandler() {
    for (PageTypeInfo* t: types) {
        delete t;
    }
    types.clear();
}

auto PageTypeHandler::parseIni(fs::path const& filepath) -> bool {
    GKeyFile* config = g_key_file_new();
    g_key_file_set_list_separator(config, ',');
    if (!g_key_file_load_from_file(config, filepath.u8string().c_str(), G_KEY_FILE_NONE, nullptr)) {
        g_key_file_free(config);
        return false;
    }

    gsize lenght = 0;
    gchar** groups = g_key_file_get_groups(config, &lenght);

    for (gsize i = 0; i < lenght; i++) {
        loadFormat(config, groups[i]);
    }

    g_strfreev(groups);
    g_key_file_free(config);
    return true;
}

void PageTypeHandler::loadFormat(GKeyFile* config, const char* group) {
    string strName;
    gchar* name = g_key_file_get_locale_string(config, group, "name", nullptr, nullptr);
    if (name != nullptr) {
        strName = name;
        g_free(name);
    }

    string strFormat;
    gchar* format = g_key_file_get_string(config, group, "format", nullptr);
    if (format != nullptr) {
        strFormat = format;
        g_free(format);
    }

    string strConfig;
    gchar* cconfig = g_key_file_get_string(config, group, "config", nullptr);
    if (cconfig != nullptr) {
        strConfig = cconfig;
        g_free(cconfig);
    }

    addPageTypeInfo(strName, getPageTypeFormatForString(strFormat), strConfig);
}

void PageTypeHandler::addPageTypeInfo(string name, PageTypeFormat format, string config) {
    auto pt = new PageTypeInfo();
    pt->name = std::move(name);
    pt->page.format = format;
    pt->page.config = std::move(config);

    this->types.push_back(pt);
}

auto PageTypeHandler::getPageTypes() -> vector<PageTypeInfo*>& { return this->types; }

auto PageTypeHandler::getPageTypeFormatForString(const string& format) -> PageTypeFormat {
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
    if (format == ":copy") {
        return PageTypeFormat::Copy;
    }
    return PageTypeFormat::Ruled;
}

auto PageTypeHandler::getStringForPageTypeFormat(const PageTypeFormat& format) -> string {
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
        case PageTypeFormat::Copy:
            return ":copy";
    }
    return "ruled";
}
