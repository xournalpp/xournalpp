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

#include <charconv>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

#include <libxml/tree.h>
#include <libxml/xmlstring.h>  // for xmlStrCmp, xmlChar

#include "util/Color.h"

#include "SettingsEnums.h"

/*
 * parse: parse settings from XML
 */
template <typename T>
T parse(std::string_view strView, T defaultValue = T{});

template <typename T>
void setParsed(T& target, const std::string_view value, T defaultValue = T{});

/*
 * Operator that converts string literals to xmlChar* (unsigned char*)
 */
inline const xmlChar* operator""_xml(const char* ch, size_t);

/*
 * Operator that compares string literals and xmlChar* (unsigned char*)
 */
inline bool operator==(const xmlChar* lhs, std::string_view rhs);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(const xmlNode* node, const std::string& property, T defaultValue = T{});

/* === END OF PROTOTYPES === */

inline const xmlChar* operator""_xml(const char* ch, size_t) { return reinterpret_cast<const xmlChar*>(ch); }

inline bool operator==(const xmlChar* lhs, const std::string_view rhs) {
    if (lhs == nullptr)
        return false;

    return xmlStrcmp(lhs, reinterpret_cast<const xmlChar*>(rhs.data())) == 0;
}

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
std::string xmlGet(const xmlNode* node, const std::string& property, std::string defaultValue) {
    const xmlChar* prop = xmlGetProp(node, reinterpret_cast<const xmlChar*>(property.c_str()));

    if (prop == nullptr) {
        return "";
    }

    const std::string str{reinterpret_cast<const char*>(prop)};

    return str;
}

template <typename T>
T parse(const std::string_view strView, T defaultValue) {
    if (strView.empty()) {
        return defaultValue;
    }

    if constexpr (std::is_same_v<T, std::string_view>) {
        return strView;
    }

    const auto str = static_cast<std::string>(strView);

    T val = defaultValue;

    if constexpr (std::is_same_v<T, std::string>) {
        val = str;
    } else if constexpr (std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long>) {
        val = static_cast<T>(std::stoul(str));
    } else if constexpr (std::is_same_v<T, int>) {
        val = std::stoi(str);
    } else if constexpr (std::is_same_v<T, double>) {
        double d;

        const auto [ptr, ec] = std::from_chars(str.c_str(), str.c_str() + sizeof(str), d);

        if (ec == std::errc::invalid_argument) {
            g_error("Invalid argument");
        }
        if (ec == std::errc()) {
            val = d;
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        val = str == "true";
    } else if constexpr (std::is_same_v<T, Color> || std::is_same_v<T, ColorU8>) {
        val = Color(static_cast<uint32_t>(std::stoull(str)));
    } else if constexpr (std::is_same_v<T, std::u8string>) {
        val = xoj::util::utf8(str).str();
    } else if constexpr (std::is_same_v<T, fs::path>) {
        val = fs::path{xoj::util::utf8(str)};
    } else if constexpr (std::is_same_v<T, AttributeType>) {
        val = stringToAttributeType(strView);
    } else if constexpr (std::is_same_v<T, XojFont>) {
        val = str;
    } else {
        const std::string err = std::format("Xournalpp does not support the required type: {}", typeid(T).name());
        throw(std::runtime_error{err});
    }

    return val;
}

/*
 * Wrapper to reduce types deduction
 */
template <typename T>
void setParsed(T& target, const std::string_view value, T defaultValue) {
    target = parse<T>(value, defaultValue);
}
