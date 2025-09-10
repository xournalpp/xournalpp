#include "util/Assert.h"

#include <cstdlib>

#include <glib.h>

#ifndef NDEBUG
namespace xoj::util {
void assertFailure(const char* expr, const std::string& msg, const char* fileName, int line, const char* funcName) {
    g_critical("Assertion failed: %s\n%s    in function %s\n    at line %d of %s", expr,
               (!msg.empty() ? std::string("    Message: ") + msg + "\n" : "").c_str(), funcName, line, fileName);
    std::abort();
}
};  // namespace xoj::util
#endif
