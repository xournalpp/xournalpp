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

#include <string_view>
#include <vector>


struct PageRangeEntry final {
    PageRangeEntry(size_t first, size_t last): first(first), last(last){};

    size_t getLast() const { return this->last; }
    size_t getFirst() const { return this->first; }

private:
    size_t first;
    size_t last;
};

using PageRangeVector = std::vector<PageRangeEntry>;

namespace PageRange {
bool isSeparator(char c);
PageRangeVector parse(std::string_view str, size_t pageCount);
};  // namespace PageRange
