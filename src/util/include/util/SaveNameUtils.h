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

class SaveNameUtils {
public:
    static std::string parseFilenameFromWildcardString(const std::string& wildcardString, fs::path defaultFilePath, bool attachPdf);

private:
    static std::string parseFilenameWildcard(const std::string& wildcard, fs::path defaultFilePath);
};
