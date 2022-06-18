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

#include <exception>  // for exception
#include <string>     // for string


class InputStreamException: public std::exception {
public:
    InputStreamException(const std::string& message, const std::string& filename, int line);
    ~InputStreamException() override;

public:
    virtual const char* what();

private:
    std::string message;
};
