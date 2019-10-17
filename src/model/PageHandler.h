/*
 * Xournal++
 *
 * Page handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <list>

class Element;
class PageListener;
class Range;
class Rectangle;

class PageHandler
{
public:
	PageHandler();
	virtual ~PageHandler();

public:
	void fireRectChanged(Rectangle& rect);
	void fireRangeChanged(Range &range);
	void fireElementChanged(Element* elem);
	void firePageChanged();

private:
	void addListener(PageListener* l);
	void removeListener(PageListener* l);

private:
	std::list<PageListener*> listener;

	friend class PageListener;
};
