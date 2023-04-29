/*
 * Xournal++
 *
 * assert
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <string>

#ifdef NDEBUG
#define xoj_assert(expr) static_cast<void>(0)
#define xoj_assert(expr, msg) static_cast<void>(0)
#else
namespace xoj::util {
[[noreturn]] void assertFailure(const char* expr, const std::string& msg, const char* fileName, int line,
                                const char* funcName);
};

#define xoj_assert_message(expr, msg)                 \
    (static_cast<bool>(expr) ? static_cast<void>(0) : \
                               xoj::util::assertFailure(#expr, msg, __FILE__, __LINE__, __func__))
#define xoj_assert(expr) xoj_assert_message(expr, "")
#endif
