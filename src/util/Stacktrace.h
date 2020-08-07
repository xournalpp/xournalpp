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

#include "filesystem.h"

class Stacktrace {
private:
    Stacktrace();
    virtual ~Stacktrace();

public:
    static fs::path getExePath();
    static void printStracktrace();
    static void printStracktrace(std::ostream& stream);
};
