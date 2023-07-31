/*
 * Xournal++
 *
 * Prints a Stacktrace
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <ostream>

#include <fbbe/stacktrace.h>

#include "filesystem.h"

namespace xoj::util {

void printStacktrace(fbbe::stacktrace const& stacktrace = fbbe::stacktrace::current());
void printStacktrace(std::ostream& stream, fbbe::stacktrace const& stacktrace = fbbe::stacktrace::current());

};  // namespace xoj::util
