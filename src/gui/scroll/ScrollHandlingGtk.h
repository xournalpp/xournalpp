/*
 * Xournal++
 *
 * Scroll handling for GTK standard implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ScrollHandling.h"

class ScrollHandlingGtk : public ScrollHandling
{
public:
	ScrollHandlingGtk(GtkScrollable* scrollable);
	virtual ~ScrollHandlingGtk();

public:
	virtual void setLayoutSize(int width, int height);

	virtual int getPrefferedWidth();
	virtual int getPrefferedHeight();

private:
	XOJ_TYPE_ATTRIB;
};
