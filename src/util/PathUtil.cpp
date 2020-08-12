#include "PathUtil.h"
#include "StringUtils.h"

#include <array>

#include <glib.h>
#include <glib/gstdio.h>

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
auto PathUtil::readString(string& output, std::filesystem::path& path, bool showErrorToUser) -> bool {
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

// TODO use std::filesystem for this
auto PathUtil::copy(const std::filesystem::path& src, const std::filesystem::path& dest) -> bool {
    std::array<char, 16 * 1024> buffer{};  // 16k

    FILE* fpRead = g_fopen(src.string().c_str(), "rbe");
    if (!fpRead) {
        return false;
    }

    FILE* fpWrite = g_fopen(dest.string().c_str(), "wbe");
    if (!fpWrite) {
        fclose(fpRead);
        return false;
    }

    while (!feof(fpRead)) {
        size_t bytes = fread(buffer.data(), 1, buffer.size(), fpRead);
        if (bytes) {
            fwrite(buffer.data(), 1, bytes, fpWrite);
        }
    }

    fclose(fpRead);
    fclose(fpWrite);

    return true;
}

auto PathUtil::getEscapedPath(const std::filesystem::path& path) -> string {
    string escaped = path.string();
    StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});

    return escaped;
}

auto PathUtil::hasXournalFileExt(const std::filesystem::path& path) -> bool {
    auto extension = path.extension();
    return extension == ".xoj" || extension == ".xopp";
}

auto PathUtil::clearExtensions(std::filesystem::path& path, const std::string &ext) -> void {
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

auto PathUtil::fromUri(const std::string &uri) -> std::filesystem::path {
    if (!StringUtils::startsWith(uri, "file://")) {
        return {};
    }

    gchar* filename = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
    if (filename == nullptr) {
        return {};
    }
    std::filesystem::path p(filename);
    g_free(filename);

    return p;
}

auto PathUtil::toUri(const std::filesystem::path& path, GError **error) -> std::string {
    char* uri = g_filename_to_uri(path.string().c_str(), nullptr, error);

    if (uri == nullptr) {
        return {};
    }

    string uriString = uri;
    g_free(uri);
    return uriString;
}

#ifndef BUILD_THUMBNAILER
auto PathUtil::fromGFile(GFile* file) -> std::filesystem::path {
    char* uri = g_file_get_uri(file);
    std::filesystem::path ret{fromUri(uri)};
    g_free(uri);

    return ret;
}

auto PathUtil::toGFile(const std::filesystem::path path) -> GFile* {
    return g_file_new_for_path(path.string().c_str());
}
#endif