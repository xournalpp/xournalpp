/*
 * Xournal++
 *
 * Paper background type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <string>

using std::string;

struct PageType
{
	/**
	 * Base format
	 */
	string format;

	/**
	 * Arguments for the format
	 */
	string config;
};
