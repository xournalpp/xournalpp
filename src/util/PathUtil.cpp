#include "util/PathUtil.h"

#include <algorithm>
#include <cstdlib>      // for system
#include <fstream>      // for ifstream, char_traits, basic_ist...
#include <iterator>     // for begin
#include <sstream>      // for stringstream
#include <string_view>  // for basic_string_view, operator""sv
#include <type_traits>  // for remove_reference<>::type
#include <utility>      // for move
#include <variant>

#include <config-paths.h>  // for PACKAGE_DATA_DIR
#include <glib.h>          // for gchar, g_free, g_filename_to_uri

#include "util/PlaceholderString.h"  // for PlaceholderString
#include "util/StringUtils.h"        // for replace_pair, StringUtils
#include "util/Util.h"               // for getPid, execInUiThread
#include "util/XojMsgBox.h"          // for XojMsgBox
#include "util/i18n.h"               // for FS, _F, FORMAT_STR
#include "util/raii/CStringWrapper.h"
#include "util/safe_casts.h"  // for as_signed
#include "util/utf8_view.h"   // for utf8_view

#include "config.h"  // for PROJECT_NAME

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>  // for readlink, ssize_t
#ifdef __APPLE__
#include <glib.h>
#include <mach-o/dyld.h>
#else
#include <climits>  // for PATH_MAX
#endif
#endif

#ifdef GHC_FILESYSTEM
// Fix of ghc::filesystem bug (path::operator/=() won't support string_views)
constexpr auto const* CONFIG_FOLDER_NAME = "xournalpp";
#else
using namespace std::string_view_literals;
constexpr auto CONFIG_FOLDER_NAME = "xournalpp"sv;
#endif

#ifdef _WIN32
auto Util::getLongPath(const fs::path& path) -> fs::path {
    auto asWString = path.wstring();
    DWORD wLongPathSz = GetLongPathNameW(asWString.c_str(), nullptr, 0);

    if (wLongPathSz == 0) {
        return path;
    }

    std::wstring wLongPath(wLongPathSz, L'\0');
    GetLongPathNameW(asWString.c_str(), wLongPath.data(), static_cast<DWORD>(wLongPath.size()));
    wLongPath.pop_back();
    return {std::move(wLongPath)};
}
#else
auto Util::getLongPath(const fs::path& path) -> fs::path { return path; }
#endif


#ifdef _WIN32
fs::path Util::getExePath() {
    char szFileName[MAX_PATH + 1];
    GetModuleFileNameA(nullptr, szFileName, MAX_PATH + 1);

    return fs::path{szFileName}.parent_path();
}
#else
#ifdef __APPLE__
fs::path Util::getExePath() {
    char c;
    uint32_t size = 0;
    _NSGetExecutablePath(&c, &size);

    char* path = new char[size + 1];
    if (_NSGetExecutablePath(path, &size) == 0) {
        fs::path p(path);
        delete[] path;
        return p.parent_path();
    }

    g_error("Could not get executable path!");

    delete[] path;
    return "";
}
#else
auto Util::getExePath() -> fs::path {
#ifndef PATH_MAX
    // This is because PATH_MAX is (per posix) not defined if there is
    // no limit, e.g., on GNU Hurd. The "right" workaround is to not use
    // PATH_MAX, instead stat the link and use stat.st_size for
    // allocating the buffer.
#define PATH_MAX 4096
#endif
    std::array<char, PATH_MAX> result{};
    ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);
    return fs::path{std::string(result.data(), as_unsigned(std::max(ssize_t{0}, count)))}.parent_path();
}
#endif
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
        std::stringstream buffer;
        std::ifstream ifs{path, openmode};
        buffer << ifs.rdbuf();
        return buffer.str();
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

auto Util::hasPngFileExt(const fs::path& path) -> bool {
    return StringUtils::toLowerCase(path.extension().string()) == ".png";
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

Util::GFilename::GFilename(const fs::path& p) {
#ifndef _WIN32  // On Windows, g_filename are always in UTF8
    if (!g_get_filename_charsets(nullptr)) {
        // g_filename are NOT utf-8 encoded
        GError* err = nullptr;
        value = xoj::util::OwnedCString::assumeOwnership(
                g_filename_from_utf8(char_cast(p.u8string().c_str()), -1, nullptr, nullptr, &err));
        if (err) {
            g_warning("Failed to convert g_filename from utf8 with error code: %d\n%s", err->code, err->message);
            g_error_free(err);
            value = u8"";
        }
    } else
#endif
    {
        value = p.u8string();
    }
}

Util::GFilename::GFilename(const char* p): value(p) {}

auto Util::GFilename::assumeOwnerhip(char* p) -> GFilename {
    GFilename f;
    f.value = xoj::util::OwnedCString::assumeOwnership(p);
    return f;
}
const char* Util::GFilename::c_str() const {
    if (const auto* p = get_if<xoj::util::OwnedCString>(&value); p) {
        return p->get();
    } else if (const auto* q = get_if<const char*>(&value); q) {
        return *q;
    }
    return char_cast(get<std::u8string>(value).c_str());
}

std::optional<fs::path> Util::GFilename::toPath() const {
#ifndef _WIN32  // On Windows, g_filename are always in UTF8
    if (!g_get_filename_charsets(nullptr)) {
        // g_filename are NOT utf-8 encoded
        GError* err = nullptr;
        auto utf8 =
                xoj::util::OwnedCString::assumeOwnership(g_filename_to_utf8(this->c_str(), -1, nullptr, nullptr, &err));
        if (err) {
            g_warning("Failed to convert g_filename to utf8 with error code: %d\n%s", err->code, err->message);
            g_error_free(err);
            return std::nullopt;
        }
        if (utf8) {
            return fs::path(xoj::util::utf8(utf8.get()));
        } else {
            // Conversion failed?
            g_warning("Failed to convert g_filename to utf8: the resulting string is empty");
            return std::nullopt;
        }
    } else
#endif
    {
        if (const char* p = this->c_str(); p) {
            return fs::path(xoj::util::utf8(p));
        } else {
            return std::nullopt;
        }
    }
}

// Uri must be ASCII-encoded!
auto Util::fromUri(const std::string& uri) -> std::optional<fs::path> {
    if (!StringUtils::startsWith(uri, "file://")) {
        return std::nullopt;
    }
    return GFilename::assumeOwnerhip(g_filename_from_uri(uri.c_str(), nullptr, nullptr)).toPath();
}

auto Util::toUri(const fs::path& path) -> std::optional<std::string> {
    GError* error{};
    char* uri = [&] {
        if (path.is_absolute()) {
            return g_filename_to_uri(GFilename(path).c_str(), nullptr, &error);
        }
        return g_filename_to_uri(GFilename(fs::absolute(path)).c_str(), nullptr, &error);
    }();

    if (error != nullptr) {
        g_warning("Util::toUri: could not parse path to URI, error: %s\n", error->message);
        g_error_free(error);
        return std::nullopt;
    }

    if (!uri) {
        g_warning("Util::toUri: path results in empty URI");
        return std::nullopt;
    }

    std::string uriString(uri);
    g_free(uri);
    return {std::move(uriString)};
}

auto Util::fromGFile(GFile* file) -> fs::path {
    return GFilename(g_file_peek_path(file)).toPath().value_or(fs::path());
}

auto Util::toGFile(fs::path const& path) -> xoj::util::GObjectSPtr<GFile> {
    return xoj::util::GObjectSPtr<GFile>(g_file_new_for_path(GFilename(path).c_str()), xoj::util::adopt);
}

auto Util::fromGFilename(const char* path) -> fs::path { return GFilename(path).toPath().value_or(fs::path()); }

auto Util::toGFilename(fs::path const& path) -> GFilename { return GFilename(path); }

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
    /// documentation of g_getenv is wrong, its UTF-8, see #5640
    const char* gettextEnv = g_getenv("TEXTDOMAINDIR");

    auto dir = [&]() -> std::optional<fs::path> {
        if (gettextEnv) {
            // Only consider first path in environment variable
            std::string_view directories(gettextEnv);
            size_t firstDot = directories.find(G_SEARCHPATH_SEPARATOR);
            if (firstDot != std::string::npos) {
                return GFilename::assumeOwnerhip(g_strndup(gettextEnv, firstDot)).toPath();
            } else {
                return GFilename(gettextEnv).toPath();
            }
        } else {
            return std::nullopt;
        }
    }();
    g_debug("TEXTDOMAINDIR = %s, Platform-specific locale dir = %s, chosen directory = %s", gettextEnv,
            localeDir.string().c_str(), dir.value_or(localeDir).string().c_str());
    return dir.value_or(localeDir);
}

auto Util::getAutosaveFilepath() -> fs::path {
    fs::path p(getCacheSubfolder("autosaves"));
    p /= std::to_string(getPid()) + ".xopp";
    return p;
}

auto Util::getConfigFolder() -> fs::path {
    auto p = GFilename(g_get_user_config_dir()).toPath().value_or(fs::path());
    return (p /= CONFIG_FOLDER_NAME);
}

auto Util::getConfigSubfolder(const fs::path& subfolder) -> fs::path {
    fs::path p = getConfigFolder();
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getCacheSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = GFilename(g_get_user_cache_dir()).toPath().value_or(fs::path());
    p /= CONFIG_FOLDER_NAME;
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

auto Util::getDataSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = GFilename(g_get_user_data_dir()).toPath().value_or(fs::path());
    p /= CONFIG_FOLDER_NAME;
    p /= subfolder;

    return Util::ensureFolderExists(p);
}

static auto buildUserStateDir() -> fs::path {
#if _WIN32
    // Windows: state directory is same as data directory (and the path is necessarily in utf-8)
    return fs::path(xoj::util::utf8(g_get_user_data_dir()));
#else
    // Unix: $XDG_STATE_HOME or ~/.local/state
    const char* xdgStateHome = std::getenv("XDG_STATE_HOME");
    if (xdgStateHome && xdgStateHome[0]) {
        // environment variable exists and is non-empty
        return Util::GFilename(xdgStateHome).toPath().value_or(fs::path());
    }

    auto path = Util::GFilename(g_get_home_dir()).toPath().value_or(fs::path());
    return path / ".local/state";
#endif
}

static auto getUserStateDir() -> const fs::path& {
    // The GLib function g_get_user_state_dir is not supported on GLib < 2.72,
    // so we implement our version here.

    // Cache fs::path so it is only computed once.
    static std::optional<const fs::path> userStateDir;
    if (!userStateDir.has_value()) {
        userStateDir.emplace(buildUserStateDir());
    }
    return *userStateDir;
}

auto Util::getStateSubfolder(const fs::path& subfolder) -> fs::path {
    auto p = getUserStateDir();
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
    auto p = GFilename(g_get_tmp_dir()).toPath().value_or(fs::path());
    p /= FS(_F("xournalpp-{1}") % Util::getPid());
    p /= subfolder;
    return Util::ensureFolderExists(p);
}

auto Util::ensureFolderExists(const fs::path& p) -> fs::path {
    try {
        fs::create_directories(p);
    } catch (const fs::filesystem_error& fe) {
        std::string msg = FS(_F("Could not create folder: {1}\nFailed with error: {2}") % p.u8string() % fe.what());
        Util::execInUiThread([msg = std::move(msg)]() { XojMsgBox::showErrorToUser(nullptr, msg); });
    }
    return p;
}

auto Util::normalizeAssetPath(const fs::path& asset, const fs::path& base, PathStorageMode mode) -> std::u8string {
    try {
        if (mode == PathStorageMode::AS_RELATIVE_PATH) {
            fs::path basenormal = base.empty() ? fs::current_path() : fs::absolute(base).lexically_normal();
            return fs::absolute(asset).lexically_proximate(basenormal).lexically_normal().generic_u8string();
        } else {
            xoj_assert(mode == PathStorageMode::AS_ABSOLUTE_PATH);
            return fs::absolute(asset).lexically_normal().generic_u8string();
        }
    } catch (const fs::filesystem_error& fe) {
        g_warning("Could not normalize path: %s\nFailed with error: %s", char_cast(asset.u8string().c_str()),
                  fe.what());
        return asset.generic_u8string();
    }
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
                  char_cast(fe.path1().u8string().c_str()), char_cast(fe.path2().u8string().c_str()), fe.what());
        fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        fs::remove(from);
    }
    return true;
}

void Util::safeReplaceExtension(fs::path& p, const char* newExtension) {
    try {
        p.replace_extension(newExtension);
    } catch (const fs::filesystem_error& fe) {
        g_warning("Could not replace extension of file \"%s\"! Failed with %s", char_cast(p.u8string().c_str()),
                  fe.what());
    }
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
    fs::path p = getExePath().parent_path();
    if (fs::exists(p / "Resources")) {
        p = p / "Resources";
    } else {
        p = PACKAGE_DATA_DIR;
        p /= PROJECT_NAME;
    }
    return p;
#else
    fs::path p = PACKAGE_DATA_DIR;
    p /= PROJECT_NAME;
    return p;
#endif
}

auto Util::getLocalePath() -> fs::path {
#ifdef __APPLE__
    fs::path p = getExePath().parent_path();
    if (fs::exists(p / "Resources")) {
        return p / "Resources" / "share" / "locale";
    }
#endif

    return getDataPath() / ".." / "locale";
}

auto Util::getBuiltInPaletteDirectoryPath() -> fs::path { return getDataPath() / "palettes"; }

auto Util::getCustomPaletteDirectoryPath() -> fs::path { return getConfigSubfolder("palettes"); }

auto Util::listFilesSorted(fs::path directory) -> std::vector<fs::path> {
    std::vector<fs::path> filePaths{};
    if (!exists(directory)) {
        g_warning("Directory %s does not exist.", char_cast(directory.u8string().c_str()));
        return filePaths;
    }

    for (const fs::directory_entry& p: fs::directory_iterator(directory)) {
        filePaths.push_back(p.path());
    }
    std::sort(filePaths.begin(), filePaths.end());
    return filePaths;
}

#ifdef XOURNALPP_WRAP_STD_FS_ABSOLUTE

static bool isUncPath(const std::filesystem::path& path) {
    const auto& str = path.native();
    const auto isSep = [](wchar_t c) { return c == L'\\' || c == L'/'; };

    return str.size() >= 3 && isSep(str[0]) && isSep(str[1]) && !isSep(str[2]);
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif  // __clang__

// The real std::filesystem::absolute
extern "C" std::filesystem::path __real__ZNSt10filesystem8absoluteERKNS_7__cxx114pathE(
        const std::filesystem::path& path);

// Wrap std::filesystem::absolute(std::filesystem::__cxx11::path const&)
extern "C" std::filesystem::path __wrap__ZNSt10filesystem8absoluteERKNS_7__cxx114pathE(
        const std::filesystem::path& path) {
    // Check for UNC paths like \\server\foo\bar. These are always absolute.
    // This works around a libstdc++ bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99333
    if (isUncPath(path)) {
        return path;
    }
    return __real__ZNSt10filesystem8absoluteERKNS_7__cxx114pathE(path);
}

// The real std::filesystem::weakly_canonical
extern "C" std::filesystem::path __real__ZNSt10filesystem16weakly_canonicalERKNS_7__cxx114pathE(
        const std::filesystem::path& path);

// Wrap std::filesystem::weakly_canonical(std::filesystem::__cxx11::path const&)
extern "C" std::filesystem::path __wrap__ZNSt10filesystem16weakly_canonicalERKNS_7__cxx114pathE(
        const std::filesystem::path& path) {
    if (isUncPath(path)) {
        auto normal = path.lexically_normal();
        if (std::wstring_view{path.native()}.substr(1) == normal.native()) {
            // Assume the path is already canonical. This does not account for symbolic links.
            // However, we know that the call for weakly_canonical will fail, because the first
            // directory separator has been removed.
            return path;
        }
    }
    return __real__ZNSt10filesystem16weakly_canonicalERKNS_7__cxx114pathE(path);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__

#endif  // XOURNALPP_WRAP_STD_FS_ABSOLUTE
