#include "Stacktrace.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <execinfo.h>
#include <unistd.h>
#include <limits.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <Path.h>


#include <iostream>
using std::endl;

/**
 * This code uses addr2line
 *
 * Another solution would be backtrace-symbols.c from cairo/util, but its really complicated
 */

Stacktrace::Stacktrace() { }

Stacktrace::~Stacktrace() { }

#ifdef _WIN32
std::string Stacktrace::getExePath()
{
	char szFileName[MAX_PATH + 1];
	GetModuleFileNameA(nullptr, szFileName, MAX_PATH + 1);

	return szFileName;
}
void Stacktrace::printStracktrace(std::ostream& stream)
{
	// Stracktrace is currently not implemented for Windows
	// Currently this is only needed for developing, so this is no issue
}
#else

#ifdef __APPLE__

std::string Stacktrace::getExePath()
{
	char c;
	uint32_t size = 0;
	_NSGetExecutablePath(&c, &size);

	char* path = new char[size + 1];
	if (_NSGetExecutablePath(path, &size) == 0)
	{
		Path p(path);
		delete[] path;
		return p.getParentPath().str();
	}

	g_error("Could not executable path!");

	delete[] path;
	return "";
}
#else
std::string Stacktrace::getExePath()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
}
#endif

void Stacktrace::printStracktrace(std::ostream& stream)
{
	void* trace[32];
	char** messages = (char**) nullptr;
	char buff[2048];

	int trace_size = backtrace(trace, 32);
	messages = backtrace_symbols(trace, trace_size);

	std::string exeName = getExePath();

	// skip first stack frame (points here)
	for (int i = 1; i < trace_size; ++i)
	{
		stream << "[bt] #" << i << " " << messages[i] << endl;

		char syscom[1024];

		sprintf(syscom, "addr2line %p -e %s", trace[i], exeName.c_str());
		FILE* fProc = popen(syscom, "r");
		while (fgets(buff, sizeof(buff), fProc) != nullptr)
		{
			stream << buff;
		}
		pclose(fProc);
	}
}
#endif

void Stacktrace::printStracktrace()
{
	printStracktrace(std::cerr);
}
