/*
 * Xournal++
 *
 * Utiltiy for loops
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <iterator>

/**
 *	for_first_then_each
 *	loop over the container from begin to end but handles the first element
 *	differently.
 */
template <typename Iter, typename Fun1, typename Fun2>
void for_first_then_each(Iter begi, Iter endi, Fun1 f1, Fun2&& f2) {
    if (begi == endi)
        return;
    f1(*begi);
    std::for_each(std::next(begi), endi, std::forward<Fun2&&>(f2));
};
