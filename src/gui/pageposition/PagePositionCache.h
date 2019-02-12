/*
 * Xournal++
 *
 * Cache for Query the page osition handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class PagePositionCache
{
public:
	PagePositionCache();
	virtual ~PagePositionCache();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * PagePositionCache ID
	 */
	int ppId;

	friend class PagePositionHandler;
};
