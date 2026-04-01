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

#include <cstddef>  // for size_t
#include <string>   // for string
#include <vector>   // for vector

struct ElementRangeEntry {
    ElementRangeEntry() = default;                                             // pre c++20 requirement
    ElementRangeEntry(size_t first, size_t last): first(first), last(last) {}  // pre c++20 requirement
    size_t first{};
    size_t last{};
};

using ElementRangeVector = std::vector<ElementRangeEntry>;

namespace ElementRange {
ElementRangeVector parse(const std::string& str, size_t maxCount);
};  // namespace ElementRange

/**
 * Aliases for page and layer ranges
 */
using PageRangeEntry = ElementRangeEntry;
using PageRangeVector = ElementRangeVector;
using LayerRangeEntry = ElementRangeEntry;
using LayerRangeVector = ElementRangeVector;
