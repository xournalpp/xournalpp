/*
 * Xournal++
 *
 * Windows UTF-8 Arguments Helper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include "util/Win32ArgsUtil.h"

#ifdef _WIN32

#include <string>
#include <vector>

#include <windows.h>

// Static storage for UTF-8 converted arguments
// Must remain valid for the lifetime of the program
static std::vector<std::string> utf8ArgStorage;
static std::vector<char*> utf8ArgPointers;

bool convertWin32ArgsToUtf8(int& argc, char**& argv) {
    int wideArgCount = 0;
    wchar_t** wideArgs = CommandLineToArgvW(GetCommandLineW(), &wideArgCount);

    if (!wideArgs) {
        return false;
    }

    utf8ArgStorage.reserve(wideArgCount);
    utf8ArgPointers.reserve(wideArgCount + 1);

    for (int i = 0; i < wideArgCount; i++) {
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, nullptr, 0, nullptr, nullptr);
        if (utf8Size > 0) {
            utf8ArgStorage.emplace_back(utf8Size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, utf8ArgStorage.back().data(), utf8Size, nullptr, nullptr);
            utf8ArgPointers.push_back(utf8ArgStorage.back().data());
        }
    }
    utf8ArgPointers.push_back(nullptr);

    LocalFree(wideArgs);

    argc = wideArgCount;
    argv = utf8ArgPointers.data();

    return true;
}

#endif
