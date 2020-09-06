/*
 * Xournal++
 *
 * Helper for reading / writing files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>

#include <gio/gio.h>

#include "filesystem.h"

namespace Util {
/**
 * Read a file to a string
 *
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 *
 * @return contents if the file was read, std::nullopt if not
 */
[[maybe_unused]] [[nodiscard]] std::optional<std::string> readString(fs::path const& path, bool showErrorToUser = true);

/**
 * Get escaped path, all " and \ are escaped
 */
[[maybe_unused]] [[nodiscard]] std::string getEscapedPath(const fs::path& path);

/**
 * @return true if this file has .xopp or .xoj extension
 */
[[maybe_unused]] [[nodiscard]] bool hasXournalFileExt(const fs::path& path);

/**
 * Clear the the last known xournal extension (last .xoj, .xopp etc.)
 *
 * @param ext An extension to clear additionally, eg .pdf (would also clear
 *  .pdf.xopp etc.)
 */
void clearExtensions(fs::path& path, const std::string& ext = "");

// Uri must be ASCII-encoded!
[[maybe_unused]] [[nodiscard]] std::optional<fs::path> fromUri(const std::string& uri);

[[maybe_unused]] [[nodiscard]] std::optional<std::string> toUri(const fs::path& path);


[[maybe_unused]] [[nodiscard]] fs::path fromGFile(GFile* file);
[[maybe_unused]] [[nodiscard]] GFile* toGFile(fs::path const& path);

[[maybe_unused]] [[nodiscard]] inline fs::path fromGtkFilename(char* path) {
    if (path == nullptr) {
        return {};
    }
    auto ret = fs::path{path};
    g_free(path);
    return ret;
}

void openFileWithDefaultApplication(const fs::path& filename);
void openFileWithFilebrowser(const fs::path& filename);

[[maybe_unused]] [[nodiscard]] bool isChildOrEquivalent(fs::path const& path, fs::path const& base);

[[maybe_unused]] bool safeRenameFile(fs::path const& from, fs::path const& to);

[[maybe_unused]] fs::path ensureFolderExists(const fs::path& p);


/**
 * Return the configuration folder path (may not be guaranteed to exist).
 */
[[maybe_unused]] [[nodiscard]] fs::path getConfigFolder();
[[maybe_unused]] [[nodiscard]] fs::path getConfigSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getCacheSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getDataSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getConfigFile(const fs::path& relativeFileName = "");
[[maybe_unused]] [[nodiscard]] fs::path getCacheFile(const fs::path& relativeFileName = "");
[[maybe_unused]] [[nodiscard]] fs::path getTmpDirSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getAutosaveFilepath();

}  // namespace Util
