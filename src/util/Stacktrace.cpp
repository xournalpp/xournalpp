#include "util/Stacktrace.h"

#include <algorithm>  // for max
#include <array>      // for array
#include <cerrno>     // for errno
#include <cstdint>    // for uintptr_t
#include <cstdio>     // for fgets, pclose, popen, snprintf, FILE
#include <cstring>    // for strerror
#include <iostream>   // for operator<<, basic_ostream, basic_ostream::...
#include <string>     // for string

#include "util/safe_casts.h"  // for bit_cast

#ifdef _WIN32
#include <Windows.h>
#else

#include <dlfcn.h>     // for dladdr
#include <execinfo.h>  // for backtrace, backtrace_symbols
#include <unistd.h>    // for readlink, ssize_t

#ifdef __APPLE__
#include <glib.h>
#include <mach-o/dyld.h>
#else
#include <climits>  // for PATH_MAX
#endif
#endif


using std::endl;

/**
 * This code uses addr2line
 *
 * Another solution would be backtrace-symbols.c from cairo/util, but its really complicated
 */

Stacktrace::Stacktrace() = default;

Stacktrace::~Stacktrace() = default;

#ifdef _WIN32
fs::path Stacktrace::getExePath() {
    char szFileName[MAX_PATH + 1];
    GetModuleFileNameA(nullptr, szFileName, MAX_PATH + 1);

    return fs::path{szFileName};
}
void Stacktrace::printStacktrace(std::ostream& stream) {
    // Stacktrace is currently not implemented for Windows
    // Currently this is only needed for developing, so this is no issue
}
#else

#ifdef __APPLE__

#include "filesystem.h"

fs::path Stacktrace::getExePath() {
    char c;
    uint32_t size = 0;
    _NSGetExecutablePath(&c, &size);

    char* path = new char[size + 1];
    if (_NSGetExecutablePath(path, &size) == 0) {
        fs::path p(path);
        delete[] path;
        return p.parent_path();
    }

    g_error("Could not executable path!");

    delete[] path;
    return "";
}
#else
auto Stacktrace::getExePath() -> fs::path {
#ifndef PATH_MAX
    // This is because PATH_MAX is (per posix) not defined if there is
    // no limit, e.g., on GNU Hurd. The "right" workaround is to not use
    // PATH_MAX, instead stat the link and use stat.st_size for
    // allocating the buffer.
#define PATH_MAX 4096
#endif
    std::array<char, PATH_MAX> result{};
    ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);
    return fs::path{std::string(result.data(), std::max(ssize_t{0}, count))};
}
#endif

void Stacktrace::printStacktrace(std::ostream& stream) {
    std::array<void*, 32> trace{};
    std::array<char, 2048> buff{};

    int trace_size = backtrace(trace.data(), trace.size());
    char** messages = backtrace_symbols(trace.data(), trace_size);

    // skip first stack frame (points here)
    for (unsigned int i = 1; i < trace_size; ++i) {
        stream << "[bt] #" << i - 1 << " " << messages[i] << endl;

        Dl_info info;  // NOLINT(cppcoreguidelines-init-variables)
        dladdr(trace[i], &info);

        std::array<char, 1024> syscom{};

        // Todo (cpp20): use std::format instead of snprintf
        snprintf(syscom.data(), syscom.size(), "addr2line %lx -fCpe \"%s\"",
                 xoj::util::bit_cast<std::uintptr_t>(trace[i]) - xoj::util::bit_cast<std::uintptr_t>(info.dli_fbase),
                 info.dli_fname);

        FILE* fProc = popen(syscom.data(), "r");
        if (fProc != nullptr) {
            while (fgets(buff.data(), buff.size(), fProc) != nullptr) {
                stream << buff.data();
            }
            pclose(fProc);
        } else {
            stream << "failed to invoke addr2line: " << std::strerror(errno);
        }
    }

    free(messages);
}
#endif

void Stacktrace::printStacktrace() { printStacktrace(std::cerr); }
