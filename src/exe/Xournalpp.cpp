/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#include "control/CrashHandler.h"  // for installCrashHandlers
#include "control/XournalMain.h"   // for run

#ifdef __APPLE__
#include "osx/setup-env.h"
#endif

#ifdef _WIN32
#include <cstdlib>
#include <string>
#include <vector>

#include <windows.h>

#include "win32/console.h"
#endif

auto main(int argc, char* argv[]) -> int {
#ifdef _WIN32
    // Convert Windows wide-character arguments to UTF-8
    // Required for proper handling of Unicode filenames (e.g., ä, ö, ü, 中文)
    int wideArgCount = 0;
    wchar_t** wideArgs = CommandLineToArgvW(GetCommandLineW(), &wideArgCount);

    std::vector<std::string> utf8ArgStorage;
    std::vector<char*> utf8ArgPointers;

    if (wideArgs) {
        utf8ArgStorage.reserve(wideArgCount);
        utf8ArgPointers.reserve(wideArgCount + 1);

        for (int i = 0; i < wideArgCount; i++) {
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, nullptr, 0, nullptr, nullptr);
            if (utf8Size > 0) {
                utf8ArgStorage.emplace_back(utf8Size - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, utf8ArgStorage.back().data(), utf8Size, nullptr,
                                    nullptr);
                utf8ArgPointers.push_back(utf8ArgStorage.back().data());
            }
        }
        utf8ArgPointers.push_back(nullptr);

        LocalFree(wideArgs);

        argc = wideArgCount;
        argv = utf8ArgPointers.data();
    }
#endif

#ifdef _WIN32
    // Convert Windows wide-character arguments to UTF-8
    // Required for proper handling of Unicode filenames (e.g., ä, ö, ü, 中文)
    int wideArgCount = 0;
    wchar_t** wideArgs = CommandLineToArgvW(GetCommandLineW(), &wideArgCount);

    std::vector<std::string> utf8ArgStorage;
    std::vector<char*> utf8ArgPointers;

    if (wideArgs) {
        utf8ArgStorage.reserve(wideArgCount);
        utf8ArgPointers.reserve(wideArgCount + 1);

        for (int i = 0; i < wideArgCount; i++) {
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, nullptr, 0, nullptr, nullptr);
            if (utf8Size > 0) {
                utf8ArgStorage.emplace_back(utf8Size - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, utf8ArgStorage.back().data(), utf8Size, nullptr,
                                    nullptr);
                utf8ArgPointers.push_back(utf8ArgStorage.back().data());
            }
        }
        utf8ArgPointers.push_back(nullptr);

        LocalFree(wideArgs);

        argc = wideArgCount;
        argv = utf8ArgPointers.data();
    }
#endif

#ifdef _WIN32
    // Attach to the console here. Otherwise, gspawn-win32-helper will create annoying console popups.
    attachConsole();
#endif

    // init crash handler
    installCrashHandlers();

#ifdef DEV_CALL_LOG
    Log::initlog();
#endif

#ifdef _WIN32
    // Switch to the FontConfig backend for Pango - See #3371
    _putenv_s("PANGOCAIRO_BACKEND", "fc");
#endif

#ifdef __APPLE__
    // Setup the environment variables, in particular so that the pixbuf loaders are found
    setupEnvironment();
#endif

    // Use this two line to test the crash handler...
    // int* crash = nullptr;
    // *crash = 0;

    int result = XournalMain::run(argc, argv);

#ifdef DEV_CALL_LOG
    Log::closelog();
#endif

    return result;
}
