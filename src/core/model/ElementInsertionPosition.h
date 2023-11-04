/*
 * Xournal++
 *
 * The position (depth) of an element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <limits>
#include <vector>

#include "Element.h"

struct InsertionPosition {
    InsertionPosition() = default;
    explicit InsertionPosition(Element* e, Element::Index pos = std::numeric_limits<Element::Index>::max()):
            e(e), pos(pos) {}
    Element* e;
    Element::Index pos;
};
using InsertionOrder = std::vector<InsertionPosition>;

inline bool operator<(const InsertionPosition& p1, const InsertionPosition& p2) { return p1.pos < p2.pos; }
