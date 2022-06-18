#include "util/logger/Logger.h"

#include <ctime>  // for localtime_r, strftime, tm

#include "config-dev.h"  // for DEV_CALL_LOG


#ifdef DEV_CALL_LOG

#include <cstdio>   // for snprintf
#include <fstream>  // for operator<<, basic_ostream, ofstream, ostring...
#include <sstream>  // for stringstream
#include <string>   // for char_traits, operator<<, allocator, string

#ifdef _WIN32

#include <windows.h>

inline std::string NowTime() {
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0) {
        return "Error in NowTime()";
    }

    char result[100] = {0};
    static DWORD first = GetTickCount();
    sprintf_s(result, 100, "%s.%06ld", buffer, (long)(GetTickCount() - first));
    return result;
}

#else

#include <sys/time.h>  // for gettimeofday, timeval

inline std::string NowTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    char buffer[100];
    tm r;
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&tv.tv_sec, &r));
    char result[100];
    snprintf(result, sizeof(result), "%s.%06ld", buffer, (long)tv.tv_usec);
    return result;
}

#endif  //_WIN32

Log::Log() {}

Log::~Log() {}

std::ofstream logfile;

void Log::trace(const char* callType, const char* clazz, const char* function, long obj) {
    std::ostringstream os;

    os << NowTime() << " - ";
    os << callType << " " << clazz << ":" << function << " (" << obj << ")" << std::endl;

    logfile << os.str();
}

void Log::initlog() { logfile.open("xournalCalls.log"); }

void Log::closelog() { logfile.close(); }

#endif  // DEV_CALL_LOG
