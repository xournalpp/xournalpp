#include "util/PageRange.h"

#include <cctype>
#include <cstdlib>

auto PageRange::isSeparator(char c) -> bool { return (c == ',' || c == ';' || c == ':'); }


// Todo (rename): what does this? Neither name or code is explaining.
auto PageRange::parse(std::string_view str, size_t pageCount) -> PageRangeVector {
    PageRangeVector data;

    if (str.empty()) {
        return data;
    }

    size_t start = 0;
    size_t end = 0;
    char* next = nullptr;
    const char* p = str.data();
    while (*p) {
        while (isspace(*p)) {
            p++;
        }

        if (*p == '-') {
            // a half-open range like -2
            start = 1;
        } else {
            start = static_cast<size_t>(strtol(p, &next, 10));
            if (start < 1) {
                start = 1;
            }
            p = next;
        }

        end = start;

        while (isspace(*p)) {
            p++;
        }

        if (*p == '-') {
            p++;
            end = static_cast<size_t>(strtol(p, &next, 10));
            if (next == p)  // a half-open range like 2-
            {
                end = pageCount;
            } else if (end < start) {
                end = start;
            }
        }

        data.emplace_back(start - 1, end - 1);

        // Skip until end or separator
        while (*p && !isSeparator(*p)) {
            p++;
        }

        // if not at end, skip separator
        if (*p) {
            p++;
        }
    }

    return data;
}
