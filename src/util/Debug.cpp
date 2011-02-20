#include "Debug.h"
#include "Stacktrace.h"

Debug::Debug() {
}

Debug::~Debug() {
}

void Debug::warning(const char * msg, ...) {
	va_list args;
	va_start(args, format);

	fprintf(stderr, "WARNING **: ");
	g_vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");

	Stacktrace::printStracktrace();
}
