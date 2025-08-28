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
#include <string_view>
#include <type_traits>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>

#include "util/Color.h"

using std::u8string;
using std::u8string_view;

/*
 * parse: parse settings from XML
 */
template <typename T>
T parse(std::string_view strView, T defaultValue = T{});

/*
 * Operator that converts string literals to char8_t* (unsigned char*)
 */
inline const char8_t* operator""_xml(const char* ch, size_t);

/*
 * Operator that converts string literals to std::basic_string<unsigned char> for comparisons
 */
inline u8string operator""_xmlstr(const char* ch, size_t);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue = T{});

inline const char8_t* operator""_xml(const char* ch, size_t) { return reinterpret_cast<const char8_t*>(ch); }

inline u8string operator""_xmlstr(const char* ch, size_t) { return reinterpret_cast<const char8_t*>(ch); }

template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue) {
    xmlChar* str = xmlGetProp(node, reinterpret_cast<const xmlChar*>(property.c_str()));
    const std::string ret{reinterpret_cast<const char*>(str)};

    xmlFree(str);

    return parse<T>(ret, defaultValue);
}

template <typename T>
T parse(const std::string_view strView, T defaultValue) {
    if (strView.empty()) {
        return defaultValue;
    }

    const auto str = static_cast<std::string>(strView);

    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    }

    if constexpr (std::is_same_v<T, int>) {
        return std::stoi(str);
    }

    if constexpr (std::is_same_v<T, unsigned int>) {
        return static_cast<unsigned int>(std::stoul(str));
    }

    if constexpr (std::is_same_v<T, double>) {
        return g_ascii_strtod(str.c_str(), nullptr);
    }

    if constexpr (std::is_same_v<T, bool>) {
        return str == "true";
    }

    if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, ColorU8>) {
        return Color(static_cast<uint32_t>(std::stoull(str)));
    }

    return defaultValue;
}
