/*
 * Xournal++
 *
 * helper methods to load a .xoj / .xopp document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#include "LoadHandlerHelper.h"

#include <cstdint>    // for uint32_t
#include <cstdlib>    // for strtol, strtoull
#include <cstring>    // for strcmp, size_t, strlen
#include <string>     // for allocator, string

#include <glib.h>  // for g_error_new, G_MARKUP_ERROR, G_M...

#include "util/Color.h"
#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/i18n.h"               // for FC, _F, _

#include "LoadHandler.h"  // for LoadHandler, getAttribInt, getAt...

#define error(...)                                                                                     \
    if (loadHandler->error == nullptr) {                                                               \
        loadHandler->error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__); \
    }

struct PredefinedColor {
    const char* name;
    const Color rgb;
};

constexpr PredefinedColor PREDEFINED_COLORS[] = {
        {"black", Colors::black},
        {"blue", Colors::xopp_royalblue},
        {"red", Colors::red},
        {"green", Colors::green},
        {"gray", Colors::gray},
        {"lightblue", Colors::xopp_deepskyblue},
        {"lightgreen", Colors::lime},
        {"magenta", Colors::magenta},
        {"orange", Colors::xopp_darkorange},
        {"yellow", Colors::yellow},
        {"white", Colors::white}};

auto LoadHandlerHelper::parseBackgroundColor(LoadHandler* loadHandler) -> Color {
    const char* sColor = LoadHandlerHelper::getAttrib("color", false, loadHandler);

    Color color = Colors::white;
    if (strcmp("blue", sColor) == 0) {
        color = Colors::xopp_paleturqoise;
    } else if (strcmp("pink", sColor) == 0) {
        color = Colors::xopp_pink;
    } else if (strcmp("green", sColor) == 0) {
        color = Colors::xopp_aquamarine;
    } else if (strcmp("orange", sColor) == 0) {
        color = Colors::xopp_lightsalmon;
    } else if (strcmp("yellow", sColor) == 0) {
        color = Colors::xopp_khaki;
    } else {
        LoadHandlerHelper::parseColor(sColor, color, loadHandler);
    }

    return color;
}

auto LoadHandlerHelper::parseColor(const char* text, Color& color, LoadHandler* loadHandler) -> bool {
    if (text == nullptr) {
        error("%s", _("Attribute color not set!"));
        return false;
    }

    if (text[0] == '#') {
        gchar* ptr = nullptr;
        auto c = uint32_t(g_ascii_strtoull(&text[1], &ptr, 16));
        if (ptr != text + strlen(text)) {
            error("%s", FC(_F("Unknown color value \"{1}\"") % text));
            return false;
        }

        color = c >> 8U;

        return true;
    }


    for (auto& i: PREDEFINED_COLORS) {
        if (!strcmp(text, i.name)) {
            color = i.rgb;
            return true;
        }
    }
    error("%s", FC(_F("Color \"{1}\" unknown (not defined in default color list)!") % text));
    return false;
}


auto LoadHandlerHelper::getAttrib(const char* name, bool optional, LoadHandler* loadHandler) -> const char* {
    const char** aName = loadHandler->attributeNames;
    const char** aValue = loadHandler->attributeValues;

    while (*aName != nullptr) {
        if (!strcmp(*aName, name)) {
            return *aValue;
        }
        aName++;
        aValue++;
    }

    if (!optional) {
        g_warning("Parser: attribute %s not found!", name);
    }
    return nullptr;
}

auto LoadHandlerHelper::getAttribDouble(const char* name, LoadHandler* loadHandler) -> double {
    const char* attrib = getAttrib(name, false, loadHandler);

    if (attrib == nullptr) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as double, the value is nullptr") % name));
        return 0;
    }

    char* ptr = nullptr;
    double val = g_ascii_strtod(attrib, &ptr);
    if (ptr == attrib) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as double, the value is \"{2}\"") % name % attrib));
    }

    return val;
}

auto LoadHandlerHelper::getAttribInt(const char* name, LoadHandler* loadHandler) -> int {
    const char* attrib = getAttrib(name, false, loadHandler);

    if (attrib == nullptr) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is nullptr") % name));
        return 0;
    }

    char* ptr = nullptr;
    int val = strtol(attrib, &ptr, 10);
    if (ptr == attrib) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is \"{2}\"") % name % attrib));
    }

    return val;
}

auto LoadHandlerHelper::getAttribInt(const char* name, bool optional, LoadHandler* loadHandler, int& rValue) -> bool {
    const char* attrib = getAttrib(name, optional, loadHandler);

    if (attrib == nullptr) {
        if (!optional) {
            g_warning("Parser: attribute %s not found!", name);
        }
        return false;
    }

    char* ptr = nullptr;
    int val = strtol(attrib, &ptr, 10);
    if (ptr == attrib) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as int, the value is \"{2}\"") % name % attrib));
    }

    rValue = val;

    return true;
}

auto LoadHandlerHelper::getAttribSizeT(const char* name, LoadHandler* loadHandler) -> size_t {
    const char* attrib = getAttrib(name, false, loadHandler);

    if (attrib == nullptr) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is nullptr") % name));
        return 0;
    }

    char* ptr = nullptr;
    size_t val = g_ascii_strtoull(attrib, &ptr, 10);
    if (ptr == attrib) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is \"{2}\"") % name % attrib));
    }

    return val;
}

auto LoadHandlerHelper::getAttribSizeT(const char* name, bool optional, LoadHandler* loadHandler, size_t& rValue)
        -> bool {
    const char* attrib = getAttrib(name, optional, loadHandler);

    if (attrib == nullptr) {
        if (!optional) {
            g_warning("Parser: attribute %s not found!", name);
        }
        return false;
    }

    char* ptr = nullptr;
    size_t val = strtoull(attrib, &ptr, 10);
    if (ptr == attrib) {
        error("%s", FC(_F("Attribute \"{1}\" could not be parsed as size_t, the value is \"{2}\"") % name % attrib));
    }

    rValue = val;

    return true;
}
