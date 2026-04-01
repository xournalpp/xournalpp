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
    const char* what() const noexcept override;

private:
    std::string message;
};
