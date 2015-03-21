#include "Stacktrace.h"
#include <execinfo.h>

/**
 * This code uses addr2line
 *
 * another solution would be backtrace-symbols.c from cairo/util, but its really complicated
 */

string exeName = "";

Stacktrace::Stacktrace() { }

Stacktrace::~Stacktrace() { }

void Stacktrace::setExename(string name) {
    exeName = name;
}

using namespace std;

void Stacktrace::printStracktrace(ostream& stream) {
    void* trace[32];
    char** messages = (char**) NULL;
    char buff[2048];

    int trace_size = backtrace(trace, 32);
    messages = backtrace_symbols(trace, trace_size);

    // skip first stack frame (points here)
    for (int i = 1; i < trace_size; ++i) {
        stream << "[bt] #" << i << " " << messages[i] << endl;

        char syscom[1024];

        sprintf(syscom, "addr2line %p -e %s", trace[i], exeName.c_str());
        FILE* fProc = popen(syscom, "r");
        while (fgets(buff, sizeof (buff), fProc) != NULL) {
            stream << buff;
        }
        pclose(fProc);
    }
}

void Stacktrace::printStracktrace() {
    printStracktrace(std::cerr);
}
