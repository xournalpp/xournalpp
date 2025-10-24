/*
 * Xournal++
 *
 * Zip Helper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include "util/ZipUtil.h"

#include "util/StringUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

auto ZipUtil::openPath(const fs::path& path, int flags, int* error) -> zip_t* {
#ifdef _WIN32
    // Convert wide string path to UTF-8 for libzip
    const std::wstring widePath = path.wstring();
    const int utf8Size = WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Path(utf8Size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), -1, utf8Path.data(), utf8Size, nullptr, nullptr);
    return zip_open(utf8Path.c_str(), flags, error);
#else
    return zip_open(path.u8string().c_str(), flags, error);
#endif
}
