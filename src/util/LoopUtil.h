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

#include <iterator>

template <typename Container, typename Function_first, typename Function_others>
void for_first_then_each(Container container, Function_first function_first, Function_others function_others)
{
	if (cbegin(container) == cend(container)) return;
	function_first(*cbegin(container));
	for (auto it = std::next(cbegin(container)); it != cend(container); it++)
	{
		function_others(*it);
	};
};
