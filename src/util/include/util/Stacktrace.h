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

class Stacktrace final {
private:
    Stacktrace();
    ~Stacktrace();

public:
    static void printStacktrace();
    static void printStacktrace(std::ostream& stream);
};
