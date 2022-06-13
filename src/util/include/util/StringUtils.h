/*
 * Xournal++
 *
 * String utilities
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

typedef std::pair<char, std::string> replace_pair;

class StringUtils {
public:
    static std::string toLowerCase(const std::string& input);
    static void replaceAllChars(std::string& input, const std::vector<replace_pair>& replaces);
    static std::vector<std::string> split(const std::string& input, char delimiter);
    static bool startsWith(const std::string& str, const std::string& start);
    static bool endsWith(const std::string& str, const std::string& end);
    static std::string ltrim(std::string str);
    static std::string rtrim(std::string str);
    static std::string trim(std::string str);
    static bool iequals(const std::string& a, const std::string& b);
};

/**
 * @brief Utilities for converting short-ish strings between different
 * encodings.
 *
 * NOTE: For now only used in context of Win32 API (and so only implemented for
 * Windows), but might evolve into more complete implementation when needed.
 *
 * Encoding assumptions: std::string => UTF-8
 *                       std::wstring => UTF-16
 */
class StringCvt {
public:
    class ConversionError: public std::runtime_error {
    public:
        ConversionError(const char* whatArg, size_t index, int result);

        auto index() -> size_t { return index_; }
        auto result() -> int { return result_; }

    private:
        size_t index_;
        int result_;
    };

#ifdef _WIN32
    static std::string u8(const std::wstring_view& wstr);
    static std::wstring u16(const std::string_view& u8str);
#endif  // _WIN32
};
