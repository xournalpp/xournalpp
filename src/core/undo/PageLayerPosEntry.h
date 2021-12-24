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

class Layer;

template <class T>
struct PageLayerPosEntry {
    // TODO: constructor could be removed with C++20
    PageLayerPosEntry<T>(Layer* layer, T* element, int pos): layer(layer), element(element), pos(pos) {}

    Layer* layer;
    T* element;
    int pos;
};

template <typename T>
constexpr auto operator<(const PageLayerPosEntry<T>& lhs, const PageLayerPosEntry<T>& rhs) -> bool {
    return lhs.pos < rhs.pos;
}
