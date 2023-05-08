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

#include <iostream>

#include <fbbe/stacktrace.h>

#include "filesystem.h"

namespace Stacktrace {
fs::path getExePath();
void printStracktrace(fbbe::stacktrace const& stacktrace = fbbe::stacktrace::current());
void printStracktrace(std::ostream& stream, fbbe::stacktrace const& stacktrace = fbbe::stacktrace::current());
};  // namespace Stacktrace
