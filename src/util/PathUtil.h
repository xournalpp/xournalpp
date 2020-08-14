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

#include "filesystem.h"
#include <gio/gio.h>

class PathUtil {
    // No instance allowed
private:
    PathUtil();

public:
    /**
     * Read a file to a string
     *
     * @param output Read contents
     * @param path Path to read
     * @param showErrorToUser Show an error to the user, if the file could not be read
     *
     * @return true if the file was read, false if not
     */
    static bool readString(std::string& output, fs::path& path, bool showErrorToUser = true);

    /**
     * Get escaped path, all " and \ are escaped
     */
    static std::string getEscapedPath(const fs::path& path);

    /**
     * @return true if this file has .xopp or .xoj extension
     */
    static bool hasXournalFileExt(const fs::path& path);

    /**
     * Clear the the last known xournal extension (last .xoj, .xopp etc.)
     *
     * @param ext An extension to clear additionally, eg .pdf (would also clear
     *  .pdf.xopp etc.)
     */
    static void clearExtensions(fs::path& path, const std::string& ext = "");

    static fs::path fromUri(const std::string& uri);

    static std::string toUri(const fs::path& path, GError **error);


    #ifndef BUILD_THUMBNAILER
    static fs::path fromGFile(GFile *file);

    static GFile* toGFile(const fs::path);
#endif
};
