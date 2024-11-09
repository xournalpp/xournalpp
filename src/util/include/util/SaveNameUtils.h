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

#include <string>       // for string

#include "filesystem.h"       // for path

constexpr auto DEFAULT_WILDCARD_START = "%{";
constexpr auto DEFAULT_WILDCARD_END = "}";

// wildcard options
constexpr auto WILDCARD_NAME = "name";  ///< default store name, e.g. original pdf name
constexpr auto WILDCARD_DATE = "date";  ///< current date - Deprecated: prefer using %F instead of %{date}
constexpr auto WILDCARD_TIME = "time";  ///< current time - Deprecated: prefer using %X instead of %{time}

class SaveNameUtils {
public:
    static std::string parseFilenameFromWildcardString(std::string_view wildcardString,
                                                       const fs::path& defaultFilePath);

private:
    static std::string parseWildcard(std::string_view wildcard, const fs::path& defaultFilePath);
};
