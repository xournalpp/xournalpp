#include "filesystem.h"

// Compilation speed up instead of including the whole header
namespace Util {
auto getExePath() -> fs::path;
}

static auto getExePathImpl() -> fs::path;

auto Util::getExePath() -> fs::path {
    static fs::path exePath = getExePathImpl();
    return exePath;
}

#ifdef _WIN32
#include <array>     // for array
#include <memory>    // for std::unique_ptr
#include <optional>  // for std::optional

#include <Windows.h>  // for GetModuleFileNameW
#include <glib.h>     // for g_error

static auto getExePathImpl() -> fs::path {
    auto fn = [](wchar_t* buffer, DWORD max_size) -> std::optional<fs::path> {
        auto size = GetModuleFileNameW(nullptr, buffer, max_size);
        if (size < max_size) {
            return fs::path{buffer, std::next(buffer, size)};
        }
        return {};
    };
    {
        std::array<wchar_t, MAX_PATH + 1> szFileName;  // NOLINT(cppcoreguidelines-pro-type-member-init)
        if (auto p = fn(szFileName.data(), szFileName.size()); p.has_value()) {
            return std::move(p.value());
        }
    }
    // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    auto szFileName = std::make_unique<wchar_t[]>(65536);
    if (auto p = fn(szFileName.get(), 65536)) {
        return std::move(p.value());
    }
    g_error("Could not get executable path!");
    return {};
}

#else

#ifdef __APPLE__

#include <cstdint>  // for uint32_t

#include <glib.h>         // for g_error
#include <mach-o/dyld.h>  // for _NSGetExecutablePath

static auto getExePathImpl() -> fs::path {
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
    return {};
}

#else

static auto getExePathImpl() -> fs::path { return fs::read_symlink("/proc/self/exe"); }

#endif
#endif
