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

constexpr auto DEFAULT_WILDCARD_START = u8"%{";
constexpr auto DEFAULT_WILDCARD_END = u8"}";

// wildcard options
constexpr auto WILDCARD_PDF_NAME = u8"name";  ///< default store name, e.g. original pdf name

constexpr auto WILDCARD_FILE_NAME = u8"file";  ///< name of the file itself

constexpr auto WILDCARD_DATE = u8"date";  ///< current date - Deprecated: prefer using %F instead of %{date}
constexpr auto WILDCARD_TIME = u8"time";  ///< current time - Deprecated: prefer using %X instead of %{time}

class SaveNameUtils {
public:
    static std::u8string parseFilenameFromWildcardString(std::u8string_view wildcardString, const fs::path& PdfPath,
                                                         const fs::path& FilePath);

private:
    static std::u8string parseWildcard(std::u8string_view wildcard, const fs::path& PdfPath, const fs::path& FilePath);
};
