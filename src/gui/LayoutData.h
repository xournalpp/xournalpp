/*
 * Xournal++
 *
 * 
 *
 * @author andreas
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class LayoutData
{
public:
	LayoutData();
	~LayoutData();

public:
	int getX();
	void setX(int x);

	int getY();
	void setY(int y);

	int getPageIndex();
	void setPageIndex(int pageIndex);

	int getLayoutAbsoluteX() const;
	int getLayoutAbsoluteY() const;

	int getMarginLeft();
	void setMarginLeft(int left);

	int getMarginTop();
	void setMarginTop(int top);

private:
	XOJ_TYPE_ATTRIB;

	// page position in layout, in px to the layout contents BORDER, not ABSOLUTE
	int x;
	int y;

	// the margin of the layout to the top
	int marginLeft;
	int marginTop;

	// 0: left page, 1: right page, will may be extended later
	int pageIndex;
};
