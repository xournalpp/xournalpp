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

#include <exception>
#include <string>
#include <vector>


class InputStreamException: public std::exception {
public:
    InputStreamException(const std::string& message, const std::string& filename, int line);
    virtual ~InputStreamException();

public:
    virtual const char* what();

private:
    std::string message;
};
