#include "PathUtil.h"
#include "StringUtils.h"

#include <array>

#include <glib.h>
#include <glib/gstdio.h>
#include <iostream>

#include "XojMsgBox.h"

/**
 * Read a file to a string
 *
 * @param output Read contents
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 *
 * @return true if the file was read, false if not
 */
auto PathUtil::readString(string& output, fs::path& path, bool showErrorToUser) -> bool {
    gchar* contents = nullptr;
    gsize length = 0;
    GError* error = nullptr;
    if (g_file_get_contents(path.string().c_str(), &contents, &length, &error)) {
        output = contents;
        g_free(contents);
        return true;
    }


    if (showErrorToUser) {
        XojMsgBox::showErrorToUser(nullptr, error->message);
    }

    g_error_free(error);
    return false;
}

auto PathUtil::getEscapedPath(const fs::path& path) -> string {
    string escaped = path.string();
    StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});

    return escaped;
}

auto PathUtil::hasXournalFileExt(const fs::path& path) -> bool {
    auto extension = path.extension();
    return extension == ".xoj" || extension == ".xopp";
}

auto PathUtil::clearExtensions(fs::path& path, const std::string &ext) -> void {
    auto rm_ext = [&path](const std::string ext) {
        if (StringUtils::toLowerCase(path.extension().string()) == StringUtils::toLowerCase(ext)) {
            path.replace_extension("");
        }
    };

    rm_ext(".xoj");
    rm_ext(".xopp");
    if (!ext.empty()) {
        rm_ext(ext);
    }
}

auto PathUtil::fromUri(const std::string &uri) -> fs::path {
    std::cout << "Checking startsWith" << std::endl;
    if (!StringUtils::startsWith(uri, "file://")) {
        return {};
    }

    std::cout << "Getting the filename from URI" << std::endl;
    gchar* filename = g_filename_from_uri(uri.c_str(), nullptr, nullptr);

    std::cout << "Checking if filename is null" << std::endl;
    if (filename == nullptr) {
        return {};
    }
    std::cout << "Generating path from filename" << std::endl;
    fs::path p(filename);
    std::cout << "Freeing filename" << std::endl;
    g_free(filename);

    std::cout << "Returning p" << std::endl;
    return p;
}

auto PathUtil::toUri(const fs::path& path, GError **error) -> std::string {
    char* uri = g_filename_to_uri(path.string().c_str(), nullptr, error);

    if (uri == nullptr) {
        return {};
    }

    string uriString = uri;
    g_free(uri);
    return uriString;
}

#ifndef BUILD_THUMBNAILER
auto PathUtil::fromGFile(GFile* file) -> fs::path {
    char* uri = g_file_get_uri(file);
    fs::path ret{fromUri(uri)};
    g_free(uri);

    return ret;
}

auto PathUtil::toGFile(const fs::path path) -> GFile* {
    return g_file_new_for_path(path.string().c_str());
}
#endif