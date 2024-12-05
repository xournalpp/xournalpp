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

#include <gio/gio.h>  // for GFile
#include <glib.h>     // for g_filename_fro...

#include "util/raii/CStringWrapper.h"
#include "util/raii/GLibGuards.h"
#include "util/safe_casts.h"  // for as_signed

#include "filesystem.h"  // for path, u8path


namespace Util {

using g_filename = ::g_filename;
using filename = std::basic_string<g_filename>;

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

[[maybe_unused]] [[nodiscard]] inline fs::path fromGFilename(g_filename const* path) {
    using namespace xoj::util;
    if (path == nullptr)
        return "";

    gsize pSize{0};
    GErrorGuard err{};
    auto u8Path = OwnedCString::assumeOwnership(
            g_filename_to_utf8(path, as_signed(std::strlen((char*)path)), nullptr, &pSize, out_ptr(err)));
    if (err) {
        g_message("Failed to convert g_filename to utf8 with error code: %d\n%s", err->code, err->message);
        return {};
    }
    return fs::u8path(u8Path.get(), u8Path.get() + pSize);
}

[[maybe_unused]] [[nodiscard]] inline fs::path fromGFilename(g_filename* path, bool owned = true) {
    using namespace xoj::util;
    using OwnedCString = BasicOwnedCString<g_filename>;
    if (path == nullptr)
        return "";
    OwnedCString guard = !owned ? OwnedCString() : OwnedCString::assumeOwnership(path);
    return fromGFilename(&std::as_const(*path));
}

[[maybe_unused]] [[nodiscard]] inline filename toGFilename(fs::path const& path) {
    using namespace xoj::util;
    using OwnedCString = BasicOwnedCString<g_filename>;
    auto u8path = path.u8string();
    gsize pSize{0};
    GErrorGuard err{};
    auto local = OwnedCString::assumeOwnership(
            g_filename_from_utf8(u8path.c_str(), as_signed(u8path.size()), nullptr, &pSize, xoj::util::out_ptr(err)));
    if (err) {
        g_message("Failed to convert g_filename from utf8 with error code: %d\n%s", err->code, err->message);
        return {};
    }
    auto ret = filename{local.get(), pSize};
    return ret;
}

void openFileWithDefaultApplication(const fs::path& filename);

[[maybe_unused]] [[nodiscard]] bool isChildOrEquivalent(fs::path const& path, fs::path const& base);

[[maybe_unused]] bool safeRenameFile(fs::path const& from, fs::path const& to);

[[maybe_unused]] fs::path ensureFolderExists(const fs::path& p);

/**
 * Convert to platform compatible path. Call this before
 * passing a path to another program.
 */
[[nodiscard]] fs::path getLongPath(const fs::path& path);

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
[[maybe_unused]] [[nodiscard]] fs::path getGettextFilepath(fs::path const& localeDir);
[[maybe_unused]] [[nodiscard]] fs::path getDataPath();
[[maybe_unused]] [[nodiscard]] fs::path getLocalePath();

}  // namespace Util
