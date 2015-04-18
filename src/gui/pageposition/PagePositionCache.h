/*
 * Xournal++
 *
 * Cache for Query the page position handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
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
