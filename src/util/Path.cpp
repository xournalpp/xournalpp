#include "Path.h"

#include <cstring>
#include <utility>

#include <glib/gstdio.h>

#include "StringUtils.h"

Path::Path(string path): path(std::move(path)) {}

Path::Path(const char* path): path(path) {}

auto Path::operator=(string p) -> Path& {
    this->path = std::move(p);
    return *this;
}

auto Path::operator=(const char* p) -> Path& {
    this->path = p;
    return *this;
}

auto Path::operator/=(const Path& p) -> Path& { return *this /= p.str(); }

auto Path::operator/=(const string& p) -> Path& {
    if (!path.empty()) {
        path.reserve(path.size() + p.size() + 1);
        path += G_DIR_SEPARATOR_S;
    }
    path += p;
    return *this;
}

auto Path::operator/=(const char* p) -> Path& {
    if (!path.empty()) {
        char c = path.at(path.size() - 1);
        if (c != '/' && c != '\\') {
            path += G_DIR_SEPARATOR_S;
        }
    }
    path += p;
    return *this;
}

auto Path::operator+=(const Path& p) -> Path& { return *this += p.str(); }

auto Path::operator+=(const string& p) -> Path& {
    path += p;
    return *this;
}

auto Path::operator+=(const char* p) -> Path& {
    path += p;
    return *this;
}

auto Path::operator/(const Path& p) const -> Path { return *this / p.str(); }

auto Path::operator/(const string& p) const -> Path {
    Path ret(*this);
    ret /= p;
    return ret;
}

auto Path::operator/(const char* p) const -> Path {
    Path ret(*this);
    ret /= p;
    return ret;
}

auto Path::operator==(const Path& other) const -> bool { return this->path == other.path; }

auto Path::isEmpty() const -> bool { return path.empty(); }

auto Path::exists() const -> bool { return g_file_test(path.c_str(), G_FILE_TEST_EXISTS); }

auto Path::deleteFile() const -> bool { return g_unlink(c_str()) == 0; }

auto Path::hasXournalFileExt() const -> bool { return hasExtension(".xoj") || hasExtension(".xopp"); }

auto Path::hasExtension(const string& ext) const -> bool {
    if (ext.length() >= path.length()) {
        return false;
    }
    auto index = path.find_last_of('.', path.length() - ext.length() - (ext.at(0) == '.' ? 0 : 1));
    if (index == std::string::npos) {
        return false;
    }

    string pathExt = path.substr(path.length() - ext.length());
    return StringUtils::toLowerCase(pathExt) == StringUtils::toLowerCase(ext);
}

void Path::clearExtensions(const string& ext) {
    string plower = StringUtils::toLowerCase(path);
    auto rm_ext = [&](string const& ext) {
        if (StringUtils::endsWith(plower, ext)) {
            auto newLen = plower.length() - ext.size();
            if (newLen < path.length()) {
                this->path = path.substr(0, newLen);
            }
        }
    };
    rm_ext(".xoj");
    rm_ext(".xopp");
    if (!ext.empty()) {
        string extLower = StringUtils::toLowerCase(ext);
        rm_ext(extLower);
        rm_ext(extLower + ".xoj");
        rm_ext(extLower + ".xopp");
    }
}

auto Path::str() const -> const string& { return path; }

auto Path::c_str() const -> const char* { return path.c_str(); }

auto Path::getEscapedPath() const -> string {
    string escaped = path;
    StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});

    return escaped;
}

auto Path::getFilename() const -> string {
    size_t separator = path.find_last_of("/\\");

    if (separator == string::npos) {
        return str();
    }

    return path.substr(separator + 1);
}

auto Path::toUri(GError** error) -> string {
    char* uri = g_filename_to_uri(path.c_str(), nullptr, error);

    if (uri == nullptr) {
        return {};
    }

    string uriString = uri;
    g_free(uri);
    return uriString;
}

#ifndef BUILD_THUMBNAILER

auto Path::toGFile() -> GFile* { return g_file_new_for_path(path.c_str()); }
#endif

auto Path::getParentPath() const -> Path {
    size_t separator = path.find_last_of("/\\");

    if (separator == string::npos) {
        return {};
    }

    return Path{path.substr(0, separator)};
}

auto Path::fromUri(const string& uri) -> Path {
    if (!StringUtils::startsWith(uri, "file://")) {
        return {};
    }

    gchar* filename = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
    Path p(filename);
    g_free(filename);

    return p;
}

#ifndef BUILD_THUMBNAILER
auto Path::fromGFile(GFile* file) -> Path {
    char* uri = g_file_get_uri(file);
    Path ret{fromUri(uri)};
    g_free(uri);

    return ret;
}
#endif
