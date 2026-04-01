/*
 * Xournal++
 *
 * Position entry for undo / redo handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <utility>

#include "model/Element.h"

class Layer;

template <class T>
struct PageLayerPosEntry {
    // TODO (cpp20): constructor could be removed with C++20
    explicit PageLayerPosEntry(Layer* layer, std::unique_ptr<T> element, typename T::Index pos):
            pos(pos),
            layer(layer),  //
            element(element.get()),
            elementOwn(std::move(element)) {}
    explicit PageLayerPosEntry(Layer* layer, T* element, typename T::Index pos):
            pos(pos),
            layer(layer),  //
            element(element) {}

    typename T::Index pos{};
    Layer* layer{};
    T* element{};
    mutable std::unique_ptr<Element> elementOwn{};
};

template <typename T>
constexpr auto operator<(const PageLayerPosEntry<T>& lhs, const PageLayerPosEntry<T>& rhs) -> bool {
    return lhs.pos < rhs.pos;
}
