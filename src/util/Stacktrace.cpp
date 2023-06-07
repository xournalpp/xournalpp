#include "util/Stacktrace.h"

#include <algorithm>  // for max
#include <array>      // for array
#include <cstdio>     // for fgets, pclose, popen, snprintf, FILE
#include <iostream>   // for operator<<, basic_ostream, basic_ostream::...

#include <fbbe/stacktrace.h>  // for stacktrace (https://github.com/Febbe/stacktrace), just a workaround until c++23 is out

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


using std::endl;

/**
 * This code uses addr2line
 *
 * Another solution would be backtrace-symbols.c from cairo/util, but its really complicated
 */


#ifdef _WIN32
fs::path Stacktrace::getExePath() {
    char szFileName[MAX_PATH + 1];
    GetModuleFileNameA(nullptr, szFileName, MAX_PATH + 1);

    return fs::path{szFileName};
}
#else

#ifdef __APPLE__

#include <string_view>  // for string_view

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
    return fs::path{std::string_view(result.data(), std::max(ssize_t{0}, count))};
}
#endif


#endif

void Stacktrace::printStracktrace(std::ostream& stream, fbbe::stacktrace const& stacktrace) {
    stream << stacktrace << endl;
}
void Stacktrace::printStracktrace(fbbe::stacktrace const& stacktrace) { printStracktrace(std::cerr, stacktrace); }
