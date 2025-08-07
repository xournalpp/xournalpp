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
#include <vector>    // for vector

#include <gio/gio.h>  // for GFile
#include <glib.h>     // for g_free, GError, g_error_free, g_filename_fro...

#include "util/raii/GObjectSPtr.h"
#include "util/safe_casts.h"  // for as_signed

#include "filesystem.h"  // for path, u8path


namespace Util {
/**
 * Read a file to a string
 *
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 * @param openmode Mode to open the file
 *
 * @return contents if the file was read, std::nullopt if not
 */
[[maybe_unused]] [[nodiscard]] std::optional<std::string> readString(fs::path const& path, bool showErrorToUser = true,
                                                                     std::ios_base::openmode openmode = std::ios::in);

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
[[nodiscard]] bool hasPdfFileExt(const fs::path& path);

/**
 * @return true if this file has a png extension
 */
[[nodiscard]] bool hasPngFileExt(const fs::path& path);

/**
 * Check if a path is absolute.
 *
 * Use this over fs::path::is_absolute to avoid a bug in libstdc++
 * regarding UNC paths on Windows.
 */
[[nodiscard]] bool isAbsolute(const fs::path& path);

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
[[maybe_unused]] [[nodiscard]] xoj::util::GObjectSPtr<GFile> toGFile(fs::path const& path);

[[maybe_unused]] [[nodiscard]] fs::path fromGFilename(const char* path);
[[maybe_unused]] [[nodiscard]] std::string toGFilename(fs::path const& path);

void openFileWithDefaultApplication(const fs::path& filename);

[[maybe_unused]] [[nodiscard]] bool isChildOrEquivalent(fs::path const& path, fs::path const& base);

[[maybe_unused]] bool safeRenameFile(fs::path const& from, fs::path const& to);
[[maybe_unused]] void safeReplaceExtension(fs::path& p, const char* nexExtension);

[[maybe_unused]] fs::path ensureFolderExists(const fs::path& p);

enum class PathStorageMode { AS_ABSOLUTE_PATH, AS_RELATIVE_PATH };
/**
 * A Xournalpp file may include references to other PDF, PNG, etc. files on disk. This function converts an asset path
 * to a string destined for storage (e.g. in a .xopp file).
 *
 * If mode == ABSOLUTE, the path is made absolute before being converted to string
 * If mode == RELATIVE, this function assumes the .xopp file is in the directory `base` and it attempts to convert
 *      `assetPath` to a relative path, relative to `base`.
 *      If it is unable to create a relative path, it will return the path unchanged
 *
 * The result if a utf-8 string with '/' as directory delimiter
 */
[[nodiscard]] std::string normalizeAssetPath(const fs::path& assetPath, const fs::path& base, PathStorageMode mode);

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
[[maybe_unused]] [[nodiscard]] fs::path getStateSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getConfigFile(const fs::path& relativeFileName = "");
[[maybe_unused]] [[nodiscard]] fs::path getCacheFile(const fs::path& relativeFileName = "");
[[maybe_unused]] [[nodiscard]] fs::path getTmpDirSubfolder(const fs::path& subfolder = "");
[[maybe_unused]] [[nodiscard]] fs::path getAutosaveFilepath();
[[maybe_unused]] [[nodiscard]] fs::path getGettextFilepath(fs::path const& localeDir);
[[maybe_unused]] [[nodiscard]] fs::path getDataPath();
[[maybe_unused]] [[nodiscard]] fs::path getLocalePath();
[[maybe_unused]] [[nodiscard]] fs::path getExePath();  ///< folder containing the executable
fs::path getBuiltInPaletteDirectoryPath();
fs::path getCustomPaletteDirectoryPath();

/**
 * List all files in a directory sorted alphabetically
 *
 * If the directory does not exist it returns an empty list.
 * @param directory to search
 * @return files in directory
 */
std::vector<fs::path> listFilesSorted(fs::path directory);
}  // namespace Util
