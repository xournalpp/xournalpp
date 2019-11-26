#include "Stacktrace.h"

#include <array>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <execinfo.h>
#include <unistd.h>
#include <climits>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
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

#include <Path.h>

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
auto Stacktrace::getExePath() -> std::string
{
	std::array<char, PATH_MAX> result{};
	ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);
	return std::string(result.data(), (count > 0) ? count : 0);
}
#endif

void Stacktrace::printStracktrace(std::ostream& stream)
{
	std::array<void*, 32> trace{};
	std::array<char, 2048> buff{};

	int trace_size = backtrace(trace.data(), trace.size());
	char** messages = backtrace_symbols(trace.data(), trace_size);

	std::string exeName = getExePath();

	// skip first stack frame (points here)
	for (int i = 1; i < trace_size; ++i)
	{
		stream << "[bt] #" << i << " " << messages[i] << endl;

		std::array<char, 1024> syscom{};

		sprintf(syscom.data(), "addr2line %p -e %s", trace[i], exeName.c_str());
		FILE* fProc = popen(syscom.data(), "r");
		while (fgets(buff.data(), buff.size(), fProc) != nullptr)
		{
			stream << buff.data();
		}
		pclose(fProc);
	}
}
#endif

void Stacktrace::printStracktrace()
{
	printStracktrace(std::cerr);
}
