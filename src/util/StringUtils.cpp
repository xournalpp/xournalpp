#include "util/StringUtils.h"

#include <array>
#include <cstring>
#include <locale>
#include <sstream>  // std::istringstream
#include <utility>

#include <glib.h>

#include "util/safe_casts.h"


using std::string;
using std::vector;
using std::wstring;

auto StringUtils::toLowerCase(const string& input) -> string {
    char* lower = g_utf8_strdown(input.c_str(), strict_cast<gssize>(input.size()));
    string lowerStr = lower;
    g_free(lower);
    return lowerStr;
}

void StringUtils::replaceAllChars(string& input, const vector<replace_pair>& replaces) {
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
    while (std::getline(tokenStream, token, delimiter)) { tokens.push_back(token); }
    return tokens;
}

auto StringUtils::startsWith(const string& str, const string& start) -> bool {
    return str.compare(0, start.length(), start) == 0;
}

auto StringUtils::endsWith(const string& str, const string& end) -> bool {
    if (end.size() > str.size()) {
        return false;
    }

    return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

const string TRIM_CHARS = "\t\n\v\f\r ";

auto StringUtils::ltrim(string str) -> string {
    str.erase(0, str.find_first_not_of(TRIM_CHARS));
    return str;
}

auto StringUtils::rtrim(string str) -> string {
    str.erase(str.find_last_not_of(TRIM_CHARS) + 1);
    return str;
}

auto StringUtils::trim(string str) -> string { return ltrim(rtrim(std::move(str))); }

auto StringUtils::iequals(const string& a, const string& b) -> bool {
    gchar* ca = g_utf8_casefold(a.c_str(), strict_cast<gssize>(a.size()));
    gchar* cb = g_utf8_casefold(b.c_str(), strict_cast<gssize>(b.size()));
    int result = strcmp(ca, cb);
    g_free(ca);
    g_free(cb);


    return result == 0;
}

static auto formatMessage(const char* whatArg, size_t index, int r) -> std::string {

    constexpr std::array results = {"ok", "partial", "error", "noconv"};

    std::ostringstream os;
    os << whatArg << ": at index " << index << ", result=" << results.at(static_cast<unsigned int>(r));
    return os.str();
}

StringCvt::ConversionError::ConversionError(const char* whatArg, size_t index, int result):
        runtime_error(formatMessage(whatArg, index, result)), index_(index), result_(result) {}

#ifdef _WIN32

namespace {

class u16u8cvt final: private std::codecvt<char16_t, char, std::mbstate_t> {
    u16u8cvt() = default;

    /**
     * @brief Can hold at least one valid code point encoded in UTF-16.
     *
     * Can hold two BMP code points, one BMP code point and the high surrogate of a non-BMP code point or one complete
     * non-BMP code point (high and low surrogate).
     */
    using u16codepoint = std::array<char16_t, 2>;

public:
    static auto u8(const std::wstring_view& wstr) -> string {
        if (wstr.empty())
            return "";

        const u16u8cvt cvt;
        std::mbstate_t mbstate{};
        string u8str(wstr.length() * 3 + 1, '\0');  // Worst case: 1 char16 -> 3 char (upper part of BMP, <=U+FFFF);
                                                    // code points using 4 UTF-8 code units (>=U+10000) would have used
                                                    // 2 UTF-16 code units.
        char* to_next = u8str.data();

        for (auto it = wstr.cbegin(); it != wstr.cend();) {
            const auto next = it + 1;
            const bool has_next = next != wstr.cend();
            const u16codepoint u16cp{static_cast<char16_t>(*it & 0xFFFF),
                                     has_next ? static_cast<char16_t>(*next & 0xFFFF) : u'\0'};
            const char16_t* from_next = u16cp.data();

            const auto r = cvt.out(mbstate, &*u16cp.cbegin(), &*u16cp.cend() - !has_next, from_next, to_next,
                                   &*u8str.end(), to_next);
            const auto u16count = from_next - u16cp.data();

            if (u16count && (r != error || u16count == 1)) {
                // The second code unit in u16cp might actually be a high surrogate of the next code point, which will
                // produce an error because the low surrogate is missing.
                // This is fine as long as the first code unit in u16cp was successfully converted. The following
                // surrogate pair will be converted on next iteration.
                it += u16count;
                continue;
            }
            assert(r != partial);
            throw StringCvt::ConversionError("UTF-16 -> UTF-8 conversion failed",
                                             static_cast<size_t>(it - wstr.cbegin()), r);
        }
        u8str.resize(static_cast<size_t>(to_next - u8str.data()));
        return u8str;
    }

    static auto u16(const std::string_view& u8str) -> wstring {
        if (u8str.empty())
            return L"";

        const u16u8cvt cvt;
        std::mbstate_t mbstate{};
        wstring wstr;
        const char* from_next = u8str.data();

        do {
            u16codepoint u16cp{};
            char16_t* to_next = u16cp.data();

            const auto r =
                    cvt.in(mbstate, from_next, &*u8str.cend(), from_next, &*u16cp.begin(), &*u16cp.end(), to_next);
            const auto u16count = static_cast<size_t>(to_next - u16cp.data());

            if (u16count && r != error) {
                wstr.push_back(static_cast<wchar_t>(u16cp.at(0) & 0xFFFF));
                if (u16count > 1)
                    wstr.push_back(static_cast<wchar_t>(u16cp.at(1) & 0xFFFF));

                continue;
            }
            assert(r != partial);
            throw StringCvt::ConversionError("UTF-8 -> UTF-16 conversion failed",
                                             static_cast<size_t>(from_next - u8str.data()), r);
        } while (from_next != &*u8str.cend());

        return wstr;
    }
};

}  // namespace

auto StringCvt::u8(const std::wstring_view& wstr) -> string { return u16u8cvt::u8(wstr); }

auto StringCvt::u16(const std::string_view& u8str) -> wstring { return u16u8cvt::u16(u8str); }

#endif  // _WIN32
