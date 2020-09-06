#include "PathUtil.h"

#include <array>
#include <fstream>

#include <glib.h>

#include "StringUtils.h"
#include "Util.h"
#include "XojMsgBox.h"
#include "i18n.h"

/**
 * Read a file to a string
 *
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 *
 * @return contents if the file was read, std::nullopt if not
 */
auto Util::readString(fs::path const& path, bool showErrorToUser) -> std::optional<std::string> {
    try {
        std::string s;
        std::ifstream ifs{path};
        s.resize(fs::file_size(path));
        ifs.read(s.data(), s.size());
        return {std::move(s)};
    } catch (fs::filesystem_error const& e) {
        if (showErrorToUser) {
            XojMsgBox::showErrorToUser(nullptr, e.what());
        }
    }
    return std::nullopt;
}

auto Util::getEscapedPath(const fs::path& path) -> string {
    string escaped = path.string();
    StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});

    return escaped;
}

auto Util::hasXournalFileExt(const fs::path& path) -> bool {
    auto extension = path.extension();
    return extension == ".xoj" || extension == ".xopp";
}

auto Util::clearExtensions(fs::path& path, const std::string& ext) -> void {
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

// Uri must be ASCII-encoded!
auto Util::fromUri(const std::string& uri) -> std::optional<fs::path> {
    if (!StringUtils::startsWith(uri, "file://")) {
        return std::nullopt;
    }

    gchar* filename = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
    if (filename == nullptr) {
        return std::nullopt;
    }
    auto p = fs::u8path(filename);
    g_free(filename);

    return {std::move(p)};
}

auto Util::toUri(const fs::path& path) -> std::optional<std::string> {
    GError* error{};
    char* uri = g_filename_to_uri(path.u8string().c_str(), nullptr, &error);

    if (error != nullptr) {
        g_warning("Could not load preview image, error: %s\n", error->message);
        g_error_free(error);
        return std::nullopt;
    }

    if (!uri) {
        g_warning("Could not load preview image");
        return std::nullopt;
    }

    string uriString(uri);
    g_free(uri);
    return {std::move(uriString)};
}

auto Util::fromGFile(GFile* file) -> fs::path {
    char* p = g_file_get_path(file);
    auto ret = p ? fs::u8path(p) : fs::path{};
    g_free(p);
    return ret;
}

auto Util::toGFile(fs::path const& path) -> GFile* { return g_file_new_for_path(path.u8string().c_str()); }


void Util::openFileWithDefaultApplication(const fs::path& filename) {
#ifdef __APPLE__
    constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif _WIN32  // note the underscore: without it, it's not msdn official!
    constexpr auto const OPEN_PATTERN = "start \"{1}\"";
#else         // linux, unix, ...
    constexpr auto const OPEN_PATTERN = "xdg-open \"{1}\"";
#endif

    string command = FS(FORMAT_STR(OPEN_PATTERN) % Util::getEscapedPath(filename));
    if (system(command.c_str()) != 0) {
        string msg = FS(_F("File couldn't be opened. You have to do it manually:\n"
                           "URL: {1}") %
                        filename.string());
        XojMsgBox::showErrorToUser(nullptr, msg);
    }
}

void Util::openFileWithFilebrowser(const fs::path& filename) {
#ifdef __APPLE__
    constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif _WIN32
    constexpr auto const OPEN_PATTERN = "explorer.exe /n,/e,\"{1}\"";
#else  // linux, unix, ...
    constexpr auto const OPEN_PATTERN = R"(nautilus "file://{1}" || dolphin "file://{1}" || konqueror "file://{1}" &)";
#endif
    string command = FS(FORMAT_STR(OPEN_PATTERN) % Util::getEscapedPath(filename));
    if (system(command.c_str()) != 0) {
        string msg = FS(_F("File couldn't be opened. You have to do it manually:\n"
                           "URL: {1}") %
                        filename.string());
        XojMsgBox::showErrorToUser(nullptr, msg);
    }
}

auto Util::getAutosaveFilepath() -> fs::path {
    fs::path p(getConfigSubfolder("autosave"));
    p /= std::to_string(getPid()) + ".xopp";
    return p;
}

auto Util::getConfigFolder() -> fs::path {
    auto p = fs::u8path(g_get_user_config_dir());
    p /= g_get_prgname();
    return p;
}

auto Util::getConfigSubfolder(const fs::path& subfolder) -> fs::path {
    fs::path p = getConfigFolder();
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getCacheSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = fs::u8path(g_get_user_cache_dir());
    p /= g_get_prgname();
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getDataSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = fs::u8path(g_get_user_data_dir());
    p /= g_get_prgname();
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getConfigFile(const fs::path& relativeFileName) -> fs::path {
    fs::path p = getConfigSubfolder(relativeFileName.parent_path());
    p /= relativeFileName.filename();
    return p;
}

auto Util::getCacheFile(const fs::path& relativeFileName) -> fs::path {
    fs::path p = getCacheSubfolder(relativeFileName.parent_path());
    p /= relativeFileName.filename();
    return p;
}

auto Util::getTmpDirSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = fs::u8path(g_get_tmp_dir());
    p /= FS(_F("xournalpp-{1}") % Util::getPid());
    p /= subfolder;
    return Util::ensureFolderExists(p);
}

auto Util::ensureFolderExists(const fs::path& p) -> fs::path {
    try {
        fs::create_directories(p);
    } catch (fs::filesystem_error const& fe) {
        Util::execInUiThread([=]() {
            string msg = FS(_F("Could not create folder: {1}\nFailed with error: {2}") % p.string() % fe.what());
            g_warning("%s %s", msg.c_str(), fe.what());
            XojMsgBox::showErrorToUser(nullptr, msg);
        });
    }
    return p;
}

auto Util::isChildOrEquivalent(fs::path const& path, fs::path const& base) -> bool {
    auto safeCanonical = [](fs::path const& p) {
        try {
            return fs::weakly_canonical(p);
        } catch (fs::filesystem_error const& fe) {
            g_warning("Util::isChildOrEquivalent: Error resolving paths, failed with %s.\nFalling back to "
                      "lexicographical path",
                      fe.what());
            return p;
        }
    };
    auto relativePath = safeCanonical(path).lexically_relative(safeCanonical(base));
    return !relativePath.empty() && *std::begin(relativePath) != "..";
}

bool Util::safeRenameFile(fs::path const& from, fs::path const& to) {
    if (!fs::is_regular_file(from)) {
        return false;
    }

    // Due to https://github.com/xournalpp/xournalpp/issues/1122,
    // we first attempt to move the file with fs::rename.
    // If this fails, we then copy and delete the source, as
    // discussed in the issue
    // Use target default perms; the source partition may have different file
    // system attributes than the target, and we don't want anything bad in the
    // autosave directory

    // Attempt move
    try {
        fs::remove(to);
        fs::rename(from, to);
    } catch (fs::filesystem_error const& fe) {
        // Attempt copy and delete
        g_warning("Renaming file %s to %s failed with %s. This may happen when source and target are on different "
                  "filesystems. Attempt to copy the file.",
                  fe.path1().string().c_str(), fe.path2().string().c_str(), fe.what());
        fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        fs::remove(from);
    }
    return true;
}
