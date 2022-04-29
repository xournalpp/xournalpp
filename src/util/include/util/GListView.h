/*
 * Xournal++
 *
 * GListView, a c++ wrapper around GList
 * Provides a begin iterator and a Sentinel
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <tuple>

#include "gtk/gtk.h"

template <typename T>

struct GListView {
    struct GListViewIter {
        constexpr auto operator++() -> GListViewIter& {
            this->list = list->next;
            return *this;
        }

        constexpr auto operator++(int) -> GListViewIter {
            GListViewIter ret = *this;
            ++(*this);
            return ret;
        }

        constexpr friend auto operator==(GListViewIter const& lhs, GListViewIter const& rhs) -> bool {
            auto tier = [](GList const& list) { return std::tie(list.data, list.next, list.prev); };
            return (lhs.list == rhs.list) ||
                   (lhs.list != nullptr && rhs.list != nullptr && tier(*lhs.list) == tier(*rhs.list));
        }

        constexpr friend auto operator!=(GListViewIter const& lhs, GListViewIter const& rhs) -> bool {
            return !(lhs == rhs);
        }

        constexpr T& operator*() { return *static_cast<T*>(list->data); }

        constexpr T* operator->() { return static_cast<T*>(list->data); }

        GList* list{};
    };

    struct GListViewSentinel {
        constexpr friend bool operator==(GListViewIter const& iter, GListViewSentinel const&) {
            return iter.list == nullptr;
        }
        constexpr friend bool operator==(GListViewSentinel const&, GListViewIter const& iter) {
            return iter.list == nullptr;
        }
        constexpr friend bool operator!=(GListViewIter const& iter, GListViewSentinel const&) {
            return iter.list != nullptr;
        }
        constexpr friend bool operator!=(GListViewSentinel const&, GListViewIter const& iter) {
            return iter.list != nullptr;
        }
    };

    constexpr GListView(GList* list): list(list) {}

    constexpr auto begin() -> GListViewIter { return {list}; }
    // /// Slow, iterates over all elements to find the end
    // Todo(if required): constexpr auto rbegin() -> GListViewRevIter { return {g_list_last(list)}; }

    constexpr auto end() -> GListViewSentinel { return {}; }
    // Todo(if required): constexpr auto rend() -> GListViewSentinel { return {}; }

    /// Convenient way to iterate over a GListView, DO NOT REVERSE IT
    constexpr auto end_iter() -> GListViewIter { return nullptr; }

private:
    GList* list;
};