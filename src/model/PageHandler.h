/*
 * Xournal++
 *
 * Page handler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __PAGEHANDLER_H__
#define __PAGEHANDLER_H__

#include <XournalType.h>

#include <glib.h>

class PageListener;
class Rectangle;
class Element;
class Range;

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
	XOJ_TYPE_ATTRIB;


	GList* listener;

	friend class PageListener;
};

#endif
