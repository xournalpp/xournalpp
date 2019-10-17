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

class Element;
class PageHandler;
class Range;
class Rectangle;

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
	PageHandler* handler = nullptr;
};
