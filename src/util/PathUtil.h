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

#include <filesystem>
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
    static bool readString(std::string& output, std::filesystem::path& path, bool showErrorToUser = true);

    static bool copy(const std::filesystem::path& src, const std::filesystem::path& dest);

    /**
     * Get escaped path, all " and \ are escaped
     */
    static std::string getEscapedPath(const std::filesystem::path& path);

    /**
     * @return true if this file has .xopp or .xoj extension
     */
    static bool hasXournalFileExt(const std::filesystem::path& path);

    /**
     * Clear the the last known xournal extension (last .xoj, .xopp etc.)
     *
     * @param ext An extension to clear additionally, eg .pdf (would also clear
     *  .pdf.xopp etc.)
     */
    static void clearExtensions(std::filesystem::path& path, const std::string& ext = "");

    static std::filesystem::path fromUri(const std::string& uri);

    static std::string toUri(const std::filesystem::path& path, GError **error);


    #ifndef BUILD_THUMBNAILER
    static std::filesystem::path fromGFile(GFile *file);

    static GFile* toGFile(const std::filesystem::path);
#endif
};
