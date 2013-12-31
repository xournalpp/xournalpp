#include "Stacktrace.h"
#include <execinfo.h>
#include <stdlib.h>

/**
 * This code uses addr2line
 *
 * another solution would be backtrace-symbols.c from cairo/util, but its really complicated
 */

#include <glib.h>

char* exeName = NULL;

Stacktrace::Stacktrace()
{
}

Stacktrace::~Stacktrace()
{
}

void Stacktrace::setExename(const char* name)
{
	if (exeName)
	{
		g_free(exeName);
	}
	exeName = g_strdup(name);
}

void Stacktrace::printStracktrace(FILE* fp)
{
	void* trace[32];
	char** messages = (char**) NULL;
	char buff[2048];

	int trace_size = backtrace(trace, 32);
	messages = backtrace_symbols(trace, trace_size);

	// skip first stack frame (points here)
	for (int i = 1; i < trace_size; ++i)
	{
		fprintf(fp, "[bt] #%d %s\n", i, messages[i]);

		char syscom[1024];

		sprintf(syscom, "addr2line %p -e %s", trace[i], exeName);
		FILE* fProc = popen(syscom, "r");
		while (fgets(buff, sizeof(buff), fProc) != NULL)
		{
			fprintf(fp, "%s", buff);
		}
		pclose(fProc);
	}
}

void Stacktrace::printStracktrace()
{
	printStracktrace(stderr);
}
