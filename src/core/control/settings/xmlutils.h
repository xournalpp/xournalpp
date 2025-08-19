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

#include <libxml/tree.h>


/*
 * xTo: parse xmlChar* (unsigned char*) string to template type
 */
template <typename T>
T parse(const xmlChar* xml = nullptr, T defaultValue = T{});

/*
 * cast: cast settings parsed from XML
 */
template <typename T>
T cast(const std::string& str, T defaultValue = T{});

/*
 * Convert a string (c-style or cpp-style) to xmlChar* (unsigned char*)
 */
template <typename T>
xmlChar* str2xmlChar(const T& str);

/*
 * get a string (c-style or cpp-style) from xmlNodePtr (xmlNode*)
 */
template <typename T>
T xmlGet(xmlNodePtr node, const char* property, T defaultValue = T{});
