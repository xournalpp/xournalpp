#include "util/PathUtil.h"

#include <cstdlib>      // for system
#include <fstream>      // for ifstream, char_traits, basic_ist...
#include <iterator>     // for begin
#include <string_view>  // for basic_string_view, operator""sv
#include <utility>      // for move

#include <config-paths.h>  // for PACKAGE_DATA_DIR
#include <glib.h>          // for gchar, g_free, g_filename_to_uri

#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/StringUtils.h"        // for replace_pair, StringUtils
#include "util/Util.h"               // for getPid, execInUiThread
#include "util/XojMsgBox.h"          // for XojMsgBox
#include "util/i18n.h"               // for FS, _F, FORMAT_STR
#include "util/raii/GLibGuards.h"

#include "config.h"  // for PROJECT_NAME

using namespace xoj::util;

#ifdef GHC_FILESYSTEM
// Fix of ghc::filesystem bug (path::operator/=() won't support string_views)
constexpr auto const* CONFIG_FOLDER_NAME = "xournalpp";
#else
using namespace std::string_view_literals;
constexpr auto CONFIG_FOLDER_NAME = "xournalpp"sv;
#endif

#ifdef _WIN32
#include <windows.h>


auto Util::getLongPath(const fs::path& path) -> fs::path {
    DWORD wLongPathSz = GetLongPathNameW(path.c_str(), nullptr, 0);  // Includes `\0`

    if (wLongPathSz == 0) {
        return path;
    }

    std::wstring wLongPath(wLongPathSz - 1, L'\0');
    wLongPathSz = GetLongPathNameW(path.c_str(), wLongPath.data(), static_cast<DWORD>(wLongPath.size()));
    wLongPath.resize(wLongPathSz);  // Does not include "\0"
    return fs::path(std::move(wLongPath));
}
#else
auto Util::getLongPath(const fs::path& path) -> fs::path { return path; }
#endif

#ifdef __APPLE__
#include "util/Stacktrace.h"
#endif

/**
 * Read a file to a string
 *
 * @param path Path to read
 * @param showErrorToUser Show an error to the user, if the file could not be read
 * @param openmode Mode to open the file
 *
 * @return contents if the file was read, std::nullopt if not
 */
auto Util::readString(fs::path const& path, bool showErrorToUser, std::ios_base::openmode openmode)
        -> std::optional<std::string> {
    try {
        std::string s;
        std::ifstream ifs{path, openmode};
        s.resize(fs::file_size(path));
        ifs.read(s.data(), s.size());
        return {std::move(s)};
    } catch (const fs::filesystem_error& e) {
        if (showErrorToUser) {
            XojMsgBox::showErrorToUser(nullptr, e.what());
        }
    }
    return std::nullopt;
}

auto Util::getEscapedPath(const fs::path& path) -> std::string {
    std::string escaped = path.string();
    StringUtils::replaceAllChars(escaped, {replace_pair('\\', "\\\\"), replace_pair('\"', "\\\"")});
    return escaped;
}

auto Util::hasXournalFileExt(const fs::path& path) -> bool {
    auto extension = StringUtils::toLowerCase(path.extension().string());
    return extension == ".xoj" || extension == ".xopp";
}

auto Util::hasPdfFileExt(const fs::path& path) -> bool {
    return StringUtils::toLowerCase(path.extension().string()) == ".pdf";
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

    return Util::fromGFilename(g_filename_from_uri(uri.c_str(), nullptr, nullptr));
}

auto Util::toUri(const fs::path& path) -> std::optional<std::string> {
    using namespace xoj::util;
    GErrorGuard error{};

    auto uri = OwnedCString::assumeOwnership(
            g_filename_to_uri(toGFilename(fs::absolute(path)).c_str(), nullptr, out_ptr(error)));

    if (error != nullptr) {
        g_warning("Util::toUri: could not parse path to URI, error: %s\n", error->message);
        return std::nullopt;
    }

    if (!uri) {
        g_warning("Util::toUri: path results in empty URI");
        return std::nullopt;
    }

    return {uri.get()};
}

auto Util::fromGFile(GFile* file) -> fs::path { return Util::fromGFilename(g_file_get_path(file)); }

auto Util::toGFile(fs::path const& path) -> GFile* {
    // Todo: Return smart pointer
    auto str = path.u8string();
    size_t filename_s{};
    GErrorGuard g;
    auto filename = BasicOwnedCString<g_filename>::assumeOwnership(
            g_filename_from_utf8(str.c_str(), as_signed(str.size()), nullptr, &filename_s, xoj::util::out_ptr(g)));
    if (g || !filename)
        return nullptr;
    return g_file_new_for_path(filename.get());
}


void Util::openFileWithDefaultApplication(const fs::path& filename) {
#ifdef __APPLE__
    constexpr auto const OPEN_PATTERN = "open \"{1}\"";
#elif _WIN32  // note the underscore: without it, it's not msdn official!
    constexpr auto const OPEN_PATTERN = "start \"\" \"{1}\"";
    /**
     * start command requires a (possibly empty) title when there are quotes around the command
     * https://stackoverflow.com/questions/27261692/how-do-i-use-quotes-in-cmd-start
     */
#else         // linux, unix, ...
    constexpr auto const OPEN_PATTERN = "xdg-open \"{1}\"";
#endif

    std::string command = FS(FORMAT_STR(OPEN_PATTERN) % Util::getEscapedPath(filename));
    if (system(command.c_str()) != 0) {
        std::string msg = FS(_F("File couldn't be opened. You have to do it manually:\n"
                                "URL: {1}") %
                             filename.u8string());
        XojMsgBox::showErrorToUser(nullptr, msg);
    }
}

auto Util::getGettextFilepath(fs::path const& localeDir) -> fs::path {
    /// documentation of g_getenv is wrong, its UTF-8, see #5640 no its not, only on windows
    std::basic_string_view gettextEnv = g_getenv((g_filename const*)("TEXTDOMAINDIR"));

    xoj::util::GErrorGuard errorGuard;
    gsize pSize{0};
    auto* gettextEnvU8 = g_filename_to_utf8(gettextEnv.data(), as_signed(gettextEnv.size()), nullptr, &pSize,
                                            xoj::util::out_ptr(errorGuard));
    if (errorGuard || !gettextEnvU8) {
        g_warning("Util::getGettextFilepath: Failed to convert g_filename to utf8 with error code: %d\n%s",
                  errorGuard->code, errorGuard->message);
        return localeDir;
    }

    // Only consider first path in environment variable
    auto dir = [&]() {
        if (!gettextEnvU8)
            return localeDir;
        std::string_view directories{gettextEnvU8, pSize};
        size_t firstDot = directories.find(G_SEARCHPATH_SEPARATOR);
        if (firstDot != std::string::npos) {
            return fs::u8path(directories.substr(0, firstDot));
        }
        return fs::u8path(directories);
    }();
    g_debug("TEXTDOMAINDIR = %s, Platform-specific locale dir = %s, chosen directory = %s", gettextEnvU8,
            localeDir.u8string().c_str(), dir.u8string().c_str());
    return dir;
}

auto Util::getAutosaveFilepath() -> fs::path {
    fs::path p(getCacheSubfolder("autosaves"));
    p /= std::to_string(getPid()) + ".xopp";
    return p;
}

auto Util::getConfigFolder() -> fs::path { return (fromGFilename(g_get_user_config_dir()) /= CONFIG_FOLDER_NAME); }

auto Util::getConfigSubfolder(const fs::path& subfolder) -> fs::path {
    fs::path p = getConfigFolder();
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getCacheSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = fromGFilename(g_get_user_cache_dir());
    p /= CONFIG_FOLDER_NAME;
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getDataSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = fromGFilename(g_get_user_data_dir());
    p /= CONFIG_FOLDER_NAME;
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
    auto p = fromGFilename(g_get_tmp_dir());
    p /= FS(_F("xournalpp-{1}") % Util::getPid());
    p /= subfolder;
    return Util::ensureFolderExists(p);
}

auto Util::ensureFolderExists(const fs::path& p) -> fs::path {
    try {
        fs::create_directories(p);
    } catch (const fs::filesystem_error& fe) {
        Util::execInUiThread([=]() {
            std::string msg = FS(_F("Could not create folder: {1}\nFailed with error: {2}") % p.u8string() % fe.what());
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
        } catch (const fs::filesystem_error& fe) {
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
    } catch (const fs::filesystem_error& fe) {
        // Attempt copy and delete
        g_warning("Renaming file %s to %s failed with %s. This may happen when source and target are on different "
                  "filesystems. Attempt to copy the file.",
                  fe.path1().string().c_str(), fe.path2().string().c_str(), fe.what());
        fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        fs::remove(from);
    }
    return true;
}

auto Util::getDataPath() -> fs::path {
#ifdef _WIN32
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(nullptr, szFileName, MAX_PATH);
    auto exePath = std::string(szFileName);
    std::string::size_type pos = exePath.find_last_of("\\/");
    fs::path p = exePath.substr(0, pos);
    p = p / ".." / "share" / PROJECT_NAME;
    return p;
#elif defined(__APPLE__)
    fs::path p = Stacktrace::getExePath().parent_path();
    if (fs::exists(p / "Resources")) {
        p = p / "Resources";
    }
    return p;
#else
    fs::path p = PACKAGE_DATA_DIR;
    p /= PROJECT_NAME;
    return p;
#endif
}

auto Util::getLocalePath() -> fs::path {
#ifdef _WIN32
    return getDataPath() / ".." / "locale";
#elif defined(__APPLE__)
    return getDataPath() / "share" / "locale";
#else
    return getDataPath() / ".." / "locale";
#endif
}
