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

#include <glib.h>

template <class ListType, typename T>
struct ListView {
    struct GListViewIter {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using difference_type = std::ptrdiff_t;

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
            return lhs.list == rhs.list;
        }

        constexpr friend auto operator!=(GListViewIter const& lhs, GListViewIter const& rhs) -> bool {
            return !(lhs == rhs);
        }

        constexpr T& operator*() { return *static_cast<T*>(list->data); }

        constexpr T* operator->() { return static_cast<T*>(list->data); }

        ListType* list{};
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

    constexpr ListView(ListType* list): list(list) {}

    constexpr auto begin() -> GListViewIter { return {list}; }
    // /// Slow, iterates over all elements to find the end
    // Todo(if required): constexpr auto rbegin() -> GListViewRevIter { return {g_list_last(list)}; }

    constexpr auto end() -> GListViewSentinel { return {}; }
    // Todo(if required): constexpr auto rend() -> GListViewSentinel { return {}; }

    /// Convenient way to iterate over a GListView, DO NOT REVERSE IT
    constexpr auto end_iter() -> GListViewIter { return {nullptr}; }

private:
    ListType* list;
};

template <typename T>
using GListView = ListView<GList, T>;
template <typename T>
using GSListView = ListView<GSList, T>;
