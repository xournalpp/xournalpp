/*
 * Xournal++
 *
 * Input stream exception
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <exception>

class InputStreamException : public std::exception
{
public:
	InputStreamException(string message, string filename, int line);
	virtual ~InputStreamException();

public:
	virtual const char* what();

private:
	string message;
};
