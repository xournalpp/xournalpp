#include "util/Stacktrace.h"

#include <iostream>  // for cerr, ostream, operator<<, endl

// for stacktrace (https://github.com/Febbe/stacktrace), just a workaround until c++23 is out:
#include <fbbe/stacktrace.h>  // for fbbe::stacktrace,

void xoj::util::printStacktrace(std::ostream& stream, fbbe::stacktrace const& stacktrace) {
    stream << stacktrace << std::endl;
}
void xoj::util::printStacktrace(fbbe::stacktrace const& stacktrace) { printStacktrace(std::cerr, stacktrace); }
