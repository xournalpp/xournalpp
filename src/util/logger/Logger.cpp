#include "Logger.h"

#include <XournalType.h>

#ifdef XOJ_CALL_LOG_ENABLED

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string NowTime()
{
	const int MAX_LEN = 200;
	char buffer[MAX_LEN];
	if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0)
		return "Error in NowTime()";

	char result[100] = {0};
	static DWORD first = GetTickCount();
	sprintf_s(result, 100, "%s.%06ld", buffer, (long) (GetTickCount() - first));
	return result;
}

#else

#include <sys/time.h>

inline std::string NowTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	char buffer[100];
	tm r;
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&tv.tv_sec, &r));
	char result[100];
	sprintf(result, "%s.%06ld", buffer, (long) tv.tv_usec);
	return result;
}

#endif //WIN32

Log::Log() { }

Log::~Log() { }

std::ofstream logfile;

void Log::trace(const char* callType, const char* clazz, const char* function, long obj)
{
	std::ostringstream os;

	os << NowTime() << " - ";
	os << callType << " " << clazz << ":" << function << " (" << obj << ")" << std::endl;

	logfile << os.str();
}

void Log::initlog()
{
	logfile.open("xournalCalls.log");
}

void Log::closelog()
{
	logfile.close();
}

#endif // XOJ_CALL_LOG_ENABLED
