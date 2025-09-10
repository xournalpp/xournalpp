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

struct InsertionPositionRef {
    constexpr InsertionPositionRef() = default;
    constexpr explicit InsertionPositionRef(const Element* e,
                                            Element::Index pos = std::numeric_limits<Element::Index>::max()):
            e(e), pos(pos) {}
    const Element* e{};
    Element::Index pos{};

    constexpr friend auto operator<(const InsertionPositionRef& p1, const InsertionPositionRef& p2) -> bool {
        return p1.pos < p2.pos;
    }
};

struct InsertionPosition {
    constexpr InsertionPosition() = default;
    explicit InsertionPosition(ElementPtr e, Element::Index pos = std::numeric_limits<Element::Index>::max()):
            e(std::move(e)), pos(pos) {}

    auto ref() const -> InsertionPositionRef { return InsertionPositionRef{e.get(), pos}; }

    ElementPtr e;
    Element::Index pos{};

    constexpr friend auto operator<(const InsertionPosition& p1, const InsertionPosition& p2) -> bool {
        return p1.pos < p2.pos;
    }
    constexpr friend auto operator<(Element::Index pos, const InsertionPosition& p2) -> bool { return pos < p2.pos; }
    constexpr friend auto operator<(const InsertionPosition& p2, Element::Index pos) -> bool { return p2.pos < pos; }
};

using InsertionOrder = std::vector<InsertionPosition>;
// using InsertionOrderRef = std::span<InsertionPositionRef const>; (cpp20, optional cpp26 (implicit conversion from
// vector))
using InsertionOrderRef = std::vector<InsertionPositionRef>;

inline auto refInsertionOrder(InsertionOrder const& order) -> InsertionOrderRef {
    InsertionOrderRef ref(order.size());
    std::transform(order.begin(), order.end(), ref.begin(), [](auto const& pos) { return pos.ref(); });
    return ref;
}
