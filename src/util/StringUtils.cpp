#include "util/StringUtils.h"

#include <cstring>
#include <sstream>  // std::istringstream
#include <utility>

#include <glib.h>

#include "util/safe_casts.h"  // for as_signed

#include <random>    
#include <algorithm> 

using std::string;
using std::vector;

bool StringUtils::isLegacy = false;

auto StringUtils::toLowerCase(const string& input) -> string {
    char* lower = g_utf8_strdown(input.c_str(), as_signed(input.size()));
    string lowerStr = lower;
    g_free(lower);
    return lowerStr;
}

std::vector<std::string> StringUtils::uids;

std::string StringUtils::generateRandomUid(int length)
{

    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; ++i) {
        randomString += charset[distribution(generator)];
    }

    return randomString;

}

std::string StringUtils::generateUniqueAlphanumericString() {
    std::string newString;
    while (true) {
        newString = generateRandomUid(8);
        
        auto it = std::find(StringUtils::uids.begin(), StringUtils::uids.end(), newString);
        
        if (it == StringUtils::uids.end()) {
            break;
        }
    }

    StringUtils::uids.push_back(newString);

    return newString;
}

void StringUtils::replaceAllChars(string& input, const std::vector<replace_pair>& replaces) {
    string out;
    bool found = false;
    for (char c: input) {
        for (const replace_pair& p: replaces) {
            if (c == p.first) {
                out += p.second;
                found = true;
                break;
            }
        }
        if (!found) {
            out += c;
        }
        found = false;
    }
    input = out;
}

auto StringUtils::split(const string& input, char delimiter) -> vector<string> {
    vector<string> tokens;
    string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

auto StringUtils::startsWith(std::string_view str, std::string_view start) -> bool {
    return str.compare(0, start.length(), start) == 0;
}

auto StringUtils::endsWith(std::string_view str, std::string_view end) -> bool {
    if (end.size() > str.size()) {
        return false;
    }

    return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

const std::string TRIM_CHARS = "\t\n\v\f\r ";

auto StringUtils::ltrim(std::string str) -> std::string {
    str.erase(0, str.find_first_not_of(TRIM_CHARS));
    return str;
}

auto StringUtils::rtrim(std::string str) -> std::string {
    str.erase(str.find_last_not_of(TRIM_CHARS) + 1);
    return str;
}

auto StringUtils::trim(std::string str) -> std::string { return ltrim(rtrim(std::move(str))); }

auto StringUtils::iequals(const string& a, const string& b) -> bool {
    gchar* ca = g_utf8_casefold(a.c_str(), as_signed(a.size()));
    gchar* cb = g_utf8_casefold(b.c_str(), as_signed(b.size()));
    int result = strcmp(ca, cb);
    g_free(ca);
    g_free(cb);


    return result == 0;
}
