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
std::optional<std::string> readString(fs::path const& path, bool showErrorToUser = true);

/**
 * Get escaped path, all " and \ are escaped
 */
std::string getEscapedPath(const fs::path& path);

/**
 * @return true if this file has .xopp or .xoj extension
 */
bool hasXournalFileExt(const fs::path& path);

/**
 * Clear the the last known xournal extension (last .xoj, .xopp etc.)
 *
 * @param ext An extension to clear additionally, eg .pdf (would also clear
 *  .pdf.xopp etc.)
 */
void clearExtensions(fs::path& path, const std::string& ext = "");

// Uri must be ASCII-encoded!
std::optional<fs::path> fromUri(const std::string& uri);

std::optional<std::string> toUri(const fs::path& path);


#ifndef BUILD_THUMBNAILER
fs::path fromGFile(GFile* file);

GFile* toGFile(const fs::path);
#endif


void openFileWithDefaultApplication(const fs::path& filename);
void openFileWithFilebrowser(const fs::path& filename);

bool isChildOrEquivalent(fs::path const& path, fs::path const& base);

bool safeRenameFile(fs::path const& from, fs::path const& to);

fs::path ensureFolderExists(const fs::path& p);


/**
 * Return the configuration folder path (may not be guaranteed to exist).
 */
fs::path getConfigFolder();
fs::path getConfigSubfolder(const fs::path& subfolder = "");
fs::path getCacheSubfolder(const fs::path& subfolder = "");
fs::path getDataSubfolder(const fs::path& subfolder = "");
fs::path getConfigFile(const fs::path& relativeFileName = "");
fs::path getCacheFile(const fs::path& relativeFileName = "");
fs::path getTmpDirSubfolder(const fs::path& subfolder = "");
fs::path getAutosaveFilepath();

}  // namespace Util
