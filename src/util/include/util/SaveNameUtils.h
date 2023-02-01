/*
 * Xournal++
 *
 * Save-name parsing utility, does wildcard-string to save-name conversions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstring>      // for size_t
#include <string>       // for string

#include "filesystem.h"       // for path

constexpr auto DEFAULT_WILDCARD_START = "%{";
constexpr auto DEFAULT_WILDCARD_END = "}";

// wildcard options
constexpr auto WILDCARD_NAME = "name"; // default store name, e.g. original pdf name
constexpr auto WILDCARD_DATE = "date"; // current date
constexpr auto WILDCARD_TIME = "time"; // current time

class SaveNameUtils {
public:
    static std::string parseFilenameFromWildcardString(const std::string& wildcardString, fs::path defaultFilePath, bool attachPdf);

private:
    static std::string parseWildcard(const std::string& wildcard, fs::path defaultFilePath);
};
