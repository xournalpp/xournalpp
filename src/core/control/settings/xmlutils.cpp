#include "xmlutils.h"

#include <string>
#include <type_traits>

#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>

#include "util/Color.h"

#include "filesystem.h"

using namespace std;

template <typename T>
xmlChar* str2xmlChar(const T& str) {
    if constexpr (is_same_v<T, string>)
        return reinterpret_cast<xmlChar*>(str.c_str());
    else if constexpr (std::is_convertible_v<T, xmlChar*>)
        return reinterpret_cast<xmlChar*>(str);
    else
        return nullptr;
}

template <typename T>
T parse(const xmlChar* xml, T defaultValue) {
    if (xml == nullptr)
        return defaultValue;

    return cast(reinterpret_cast<const char*>(xml), defaultValue);
}

template <typename T>
T cast(const string& str, T defaultValue) {
    if (str.empty())
        return defaultValue;

    if (str.empty())
        return defaultValue;

    if constexpr (is_same_v<T, int>)
        return stoi(str);

    if constexpr (is_same_v<T, unsigned int>)
        return static_cast<unsigned int>(stoul(str));

    if constexpr (is_same_v<T, double>)
        return g_ascii_strtod(str.c_str(), nullptr);

    if constexpr (is_same_v<T, bool>)
        return str == "true";

    if constexpr (is_same_v<T, Color> || is_same_v<T, ColorU8>)
        return Color(static_cast<uint32_t>(stoul(str)));

    return defaultValue;
}


template <typename T>
T xmlGet(const xmlNodePtr node, const char* property, T defaultValue) {
    xmlChar* str = xmlGetProp(node, reinterpret_cast<const xmlChar*>(property));
    xmlFree(str);

    return parse<T>(str, defaultValue);
}
