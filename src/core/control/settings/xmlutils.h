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
 * reinterpret cast from xmlChar* to char8_t*
 */
inline u8string_view operator""_u8s(const char* xml, size_t);

/*
 * Operator that converts string literals to xmlChar* (unsigned char*)
 */
inline const xmlChar* operator""_xml(const char* ch, size_t);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue = T{});

inline const xmlChar* operator""_xml(const char* ch, size_t) { return reinterpret_cast<const xmlChar*>(ch); }

inline u8string_view operator""_u8s(const char* xml, size_t) { return reinterpret_cast<const char8_t*>(xml); }

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
        return Color(static_cast<uint32_t>(std::stoull(str)));
    }

    if constexpr (std::is_same_v<T, std::u8string>) {
        return xoj::util::utf8(str).str();
    }

    return defaultValue;
}
