/*
 * Xournal++
 *
 * A page range for PDF export etc. (e.g. 1-2,5,7)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

struct PageRangeEntry {
    PageRangeEntry() = default;                                             // pre c++20 requirement
    PageRangeEntry(size_t first, size_t last): first(first), last(last) {}  // pre c++20 requirement
    size_t first{};
    size_t last{};
};

using PageRangeVector = std::vector<PageRangeEntry>;

namespace PageRange {
PageRangeVector parse(const std::string& str, size_t pageCount);
};  // namespace PageRange
