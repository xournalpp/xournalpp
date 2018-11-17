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

class Stacktrace
{
private:
	Stacktrace();
	virtual ~Stacktrace();

public:
	static void printStracktrace();
	static void printStracktrace(std::ostream& stream);

private:
	static void errorCallback(std::ostream* stream, const char* msg, int errnum);
	static int fullCallback(std::ostream* stream, uintptr_t pc, const char* filename, int lineno, const char* function);
};
