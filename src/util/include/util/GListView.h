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
        auto operator++() -> GListViewIter& {
            this->list = list->next;
            return *this;
        }

        auto operator++(int) -> GListViewIter {
            GListViewIter ret = *this;
            ++(*this);
            return ret;
        }

        friend auto operator==(GListViewIter const& lhs, GListViewIter const& rhs) -> bool {
            auto tier = [](GList const& list) { std::tie(list.data, list.next, list.prev); };
            return lhs.list == rhs.list || tier(lhs) == tier(rhs);
        }

        friend auto operator!=(GListViewIter const& lhs, GListViewIter const& rhs) -> bool { return !(lhs == rhs); }

        T& operator*() { return *static_cast<T*>(list->data); }

        T* operator->() { return static_cast<T*>(list->data); }

        GList* list{};
    };

    struct GListViewSentinel {
        friend bool operator==(GListViewIter const& iter, GListViewSentinel const&) { return iter.list == nullptr; }
        friend bool operator==(GListViewSentinel const&, GListViewIter const& iter) { return iter.list == nullptr; }
        friend bool operator!=(GListViewIter const& iter, GListViewSentinel const&) { return iter.list != nullptr; }
        friend bool operator!=(GListViewSentinel const&, GListViewIter const& iter) { return iter.list != nullptr; }
    };

    GListView(GList* list): list(list) {}

    auto begin() -> GListViewIter { return {list}; }

    auto end() -> GListViewSentinel { return {}; }

private:
    GList* list;
};