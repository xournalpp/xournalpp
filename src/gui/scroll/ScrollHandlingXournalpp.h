/*
 * Xournal++
 *
 * Scroll handling for Xournal workaround without standard GTK Scroll handling
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ScrollHandling.h"

class ScrollHandlingXournalpp : public ScrollHandling
{
public:
	ScrollHandlingXournalpp();
	virtual ~ScrollHandlingXournalpp();

public:
	virtual void setLayoutSize(int width, int height);

	virtual int getPrefferedWidth();
	virtual int getPrefferedHeight();

private:
	XOJ_TYPE_ATTRIB;
};
