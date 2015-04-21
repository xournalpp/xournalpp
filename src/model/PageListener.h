/*
 * Xournal++
 *
 * Page listener
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class PageHandler;
class Rectangle;
class Element;
class Range;

class PageListener
{
public:
	PageListener();
	virtual ~PageListener();

public:
	void registerListener(PageHandler* handler);
	void unregisterListener();

	virtual void rectChanged(Rectangle& rect) { }
	virtual void rangeChanged(Range &range) { }
	virtual void elementChanged(Element* elem) { }
	virtual void pageChanged() { }

private:
	XOJ_TYPE_ATTRIB;

	PageHandler* handler;
};
