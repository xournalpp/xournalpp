/*
 * Xournal++
 *
 * Utility for loops
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <utility>

/**
 *	for_first_then_each
 *	loop over the container from begin to end but handles the first element
 *	differently.
 */
template <typename Container, typename Fun1, typename Fun2>
void for_first_then_each(Container&& c, Fun1 f1, Fun2&& f2) {
    auto begi = begin(c);
    auto endi = end(c);
    if (begi == endi)
        return;
    f1(*begi);
    std::for_each(std::next(begi), endi, std::forward<Fun2&&>(f2));
};
