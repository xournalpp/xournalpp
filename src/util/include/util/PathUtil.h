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

#include <cstring>   // for strlen, size_t
#include <optional>  // for optional
#include <string>    // for string, allocator, basic_string

#include <gio/gio.h>    // for GFile
#include <glib.h>       // for g_free, GError, g_error_free, g_filename_fro...
#include <sys/types.h>  // for ssize_t

#include "filesystem.h"  // for path, u8path

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
 * @return true if this file has a pdf extension
 */
bool hasPdfFileExt(const fs::path& path);

/**
 * Clear the xournal extensions ignoring case (.xoj, .xopp)
 *
 * @param ext An extension to clear additionally, eg .pdf (would also clear
 *  .PDF.xopp etc.)
 */
void clearExtensions(fs::path& path, const std::string& ext = "");

// Uri must be ASCII-encoded!
[[maybe_unused]] [[nodiscard]] std::optional<fs::path> fromUri(const std::string& uri);

[[maybe_unused]] [[nodiscard]] std::optional<std::string> toUri(const fs::path& path);


[[maybe_unused]] [[nodiscard]] fs::path fromGFile(GFile* file);
[[maybe_unused]] [[nodiscard]] GFile* toGFile(fs::path const& path);

[[maybe_unused]] [[nodiscard]] inline fs::path fromGFilename(char* path, bool owned = true) {
    auto deleter = [path, owned]() {
        if (owned) {
            g_free(path);
        }
    };

    if (path == nullptr) {
        return {};
    }
    size_t pSize{0};
    GError* err{};
    auto* u8Path = g_filename_to_utf8(path, std::strlen(path), nullptr, &pSize, &err);
    if (err) {
        g_message("Failed to convert g_filename to utf8 with error code: %d\n%s", err->code, err->message);
        g_error_free(err);
        deleter();
        return {};
    }
    auto ret = fs::u8path(u8Path, u8Path + pSize);
    g_free(u8Path);
    deleter();
    return ret;
}

[[maybe_unused]] [[nodiscard]] inline std::string toGFilename(fs::path const& path) {
    auto u8path = path.u8string();
    size_t pSize{0};
    GError* err{};
    auto* local = g_filename_from_utf8(u8path.c_str(), ssize_t(u8path.size()), nullptr, &pSize, &err);
    if (err) {
        g_message("Failed to convert g_filename from utf8 with error code: %d\n%s", err->code, err->message);
        g_error_free(err);
        return {};
    }
    auto ret = std::string{local, pSize};
    g_free(local);
    return ret;
}


void openFileWithDefaultApplication(const fs::path& filename);
void openFileWithFilebrowser(const fs::path& filename);

[[maybe_unused]] [[nodiscard]] bool isChildOrEquivalent(fs::path const& path, fs::path const& base);

[[maybe_unused]] bool safeRenameFile(fs::path const& from, fs::path const& to);

[[maybe_unused]] fs::path ensureFolderExists(const fs::path& p);

/**
 * Convert to platform compatible path. Call this before
 * passing a path to another program.
 */
fs::path getLongPath(const fs::path& path);

[[deprecated("can produce invalid strings on windows, use fs::path::native()")]] [[nodiscard]]  //
auto system_single_byte_filename(const fs::path& path) -> std::string;

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
[[maybe_unused]] [[nodiscard]] fs::path getGettextFilepath(const char* localeDir);
[[maybe_unused]] [[nodiscard]] fs::path getDataPath();
[[maybe_unused]] [[nodiscard]] fs::path getLocalePath();

}  // namespace Util
