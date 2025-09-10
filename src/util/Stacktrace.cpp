#include "util/Stacktrace.h"

#include "config-features.h"
#ifdef ENABLE_CPPTRACE
#include <cpptrace/cpptrace.hpp>
#endif

Stacktrace::Stacktrace() = default;

Stacktrace::~Stacktrace() = default;


void Stacktrace::printStacktrace(std::ostream& stream) {
#ifdef ENABLE_CPPTRACE
    cpptrace::generate_trace().print(stream);
#else
    stream << "No stacktrace library or symbols available" << std::endl;
#endif
}

void Stacktrace::printStacktrace() { printStacktrace(std::cerr); }
