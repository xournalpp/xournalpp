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

// TODO: after C++20 migrate to std::u8string_view
using ustring_view = std::basic_string_view<unsigned char>;

/*
 * parse: parse settings from XML
 */
template <typename T>
T parse(std::string_view strView, T defaultValue = T{});

/*
 * Operator that converts string literals to xmlChar* (unsigned char*)
 */
inline const xmlChar* operator""_xml(const char* ch, size_t);

/*
 * Operator that converts string literals to std::basic_string<unsigned char> for comparisons
 */
inline ustring_view operator""_xmlsv(const char* ch, size_t);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue = T{});

inline const xmlChar* operator""_xml(const char* ch, size_t) { return reinterpret_cast<const xmlChar*>(ch); }

inline ustring_view operator""_xmlsv(const char* ch, size_t) { return reinterpret_cast<const unsigned char*>(ch); }

template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue) {
    const xmlChar* prop = xmlGetProp(node, reinterpret_cast<const xmlChar*>(property.c_str()));

    if (prop == nullptr) {
        return parse<T>({}, defaultValue);
    }

    const std::string_view str(reinterpret_cast<const char*>(prop));

    return parse<T>(str, defaultValue);
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
        return Color(static_cast<uint32_t>(std::stoul(str)));
    }

    return defaultValue;
}
