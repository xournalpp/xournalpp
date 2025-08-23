/*
 * Xournal++
 *
 * libxml wrapper functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>
#include <type_traits>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>

#include "util/Color.h"

#include "xmlutils.h"


/*
 * xTo: parse xmlChar* (unsigned char*) string to template type
 */
template <typename T>
T parse(const xmlChar* xml, T defaultValue = T{});

/*
 * cast: cast settings parsed from XML
 */
template <typename T>
T cast(std::string_view strView, T defaultValue = T{});

/*
 * Operator that converts string literals to xmlChar* (unsigned char*)
 */
inline const xmlChar* operator""_xml(const char* ch, size_t);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(const xmlNode* node, const char* property, T defaultValue = T{});

template <typename T>
T parse(const xmlChar* xml, T defaultValue) {
    if (xml == nullptr) {
        return defaultValue;
    }

    return cast(reinterpret_cast<const char*>(xml), defaultValue);
}

inline const xmlChar* operator""_xml(const char* ch, size_t) { return reinterpret_cast<const xmlChar*>(ch); }

template <typename T>
T xmlGet(const xmlNode* node, const char* property, T defaultValue) {
    xmlChar* str = xmlGetProp(node, reinterpret_cast<const xmlChar*>(property));
    const T ret = parse<T>(str, defaultValue);

    xmlFree(str);

    return ret;
}


template <typename T>
T cast(const std::string_view strView, T defaultValue) {
    if (strView.empty()) {
        return defaultValue;
    }

    const auto str = static_cast<std::string>(strView);

    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    }

    if constexpr (std::is_same_v<T, int>) {
        return stoi(str);
    }

    if constexpr (std::is_same_v<T, unsigned int>) {
        return static_cast<unsigned int>(stoul(str));
    }

    if constexpr (std::is_same_v<T, double>) {
        return g_ascii_strtod(str.c_str(), nullptr);
    }

    if constexpr (std::is_same_v<T, bool>) {
        return str == "true";
    }

    if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, ColorU8>) {
        return Color(static_cast<uint32_t>(stoul(str)));
    }

    return defaultValue;
}
