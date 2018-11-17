#include "Stacktrace.h"

#include <iostream>

#include <string.h>
#include <backtrace.h>
#include <cxxabi.h>

using std::endl;

Stacktrace::Stacktrace() { }

Stacktrace::~Stacktrace() { }

void Stacktrace::printStracktrace(std::ostream& stream)
{
	stream << "Backtrace: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	struct backtrace_state* lbstate = backtrace_create_state(nullptr, 1, (backtrace_error_callback)errorCallback, &stream);
	backtrace_full(lbstate, 0, (backtrace_full_callback)fullCallback, (backtrace_error_callback)errorCallback, &stream);
	stream << "Backtrace end <<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
}

int Stacktrace::fullCallback(std::ostream* stream, uintptr_t pc, const char* filename, int lineno, const char* function)
{
	int demangle_status = 0;
	char *realname = abi::__cxa_demangle(function, 0, 0, &demangle_status);

	if (demangle_status != 0 && function != NULL)
	{
		realname = ::strdup(function);
	}

	*stream << "0x" << std::hex << (unsigned long) pc << "\t";
	*stream << (realname == nullptr ? "???" : realname) << "\t";
	*stream << (filename == nullptr ? "???" : filename) << ":";
	*stream << std::dec << lineno << endl;

	free(realname);

	if (function == NULL)
	{
		return 0;
	}
	return strcmp(function, "main") == 0 ? 1 : 0;
}

void Stacktrace::errorCallback(std::ostream* stream, const char* msg, int errnum)
{
	*stream << "Something went wrong in libbacktrace: " << msg << endl;
}

void Stacktrace::printStracktrace()
{
	printStracktrace(std::cerr);
}
