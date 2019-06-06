#include <algorithm>
#include <numeric>
#include "Layout.h"

#include "XournalView.h"

#include "control/Control.h"
#include "widgets/XournalWidget.h"
#include "gui/scroll/ScrollHandling.h"

/**
 * Padding outside the pages, including shadow
 */
const int XOURNAL_PADDING = 10;

/**
 * Padding outside the pages, if additional padding is set
 */
const int XOURNAL_PADDING_FREE_SPACE = 150;

/**
 * Allowance for shadow between page pairs in paired page mode
 */
const int XOURNAL_ROOM_FOR_SHADOW = 3;

/**
 * Padding between the pages
 */
const int XOURNAL_PADDING_BETWEEN = 15;


Layout::Layout(XournalView* view, ScrollHandling* scrollHandling)
		: view(view),
		  scrollHandling(scrollHandling)
{
	XOJ_INIT_TYPE(Layout);

	g_signal_connect(scrollHandling->getHorizontal(), "value-changed", G_CALLBACK(horizontalScrollChanged), this);

	g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(verticalScrollChanged), this);


	lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
	lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
	recalculate();
}

void Layout::horizontalScrollChanged(GtkAdjustment* adjustment, Layout* layout)
{
	XOJ_CHECK_TYPE_OBJ(layout, Layout);
	layout->checkScroll(adjustment, layout->lastScrollHorizontal);
	layout->updateVisibility();
	layout->scrollHandling->scrollChanged();
}

void Layout::verticalScrollChanged(GtkAdjustment* adjustment, Layout* layout)
{
	XOJ_CHECK_TYPE_OBJ(layout, Layout);
	layout->checkScroll(adjustment, layout->lastScrollVertical);
	layout->updateVisibility();
	layout->scrollHandling->scrollChanged();
}

Layout::~Layout()
{
	XOJ_RELEASE_TYPE(Layout);
}

void Layout::checkScroll(GtkAdjustment* adjustment, double& lastScroll)
{
	XOJ_CHECK_TYPE(Layout);

	lastScroll = gtk_adjustment_get_value(adjustment);
}

/**
 * Update Visibilty for each page
 */
void Layout::updateVisibility()
{
	XOJ_CHECK_TYPE(Layout);

	Rectangle visRect = getVisibleRect();

	// step through every possible page position and update using p->setIsVisible()
	// Using initial grid aprox speeds things up by a factor of 5.  See previous git check-in for specifics.
	int x1 = 0;
	int y1 = 0;

	for (int row = 0; row < this->heightRows.size(); ++row)
	{
		int y2 = this->heightRows[row];
		for (int col = 0; col < this->widthCols.size(); ++col)
		{
			int x2 = this->widthCols[col];
			int pageIndex = this->mapper.map(col, row);
			if (pageIndex >= 0)    // a page exists at this grid location
			{

				XojPageView* pageView = this->view->viewPages[pageIndex];


				//check if grid location is visible as an aprox for page visiblity:
				if (!(visRect.x > x2 || visRect.x + visRect.width < x1) // visrect not outside current row/col
				    && !(visRect.y > y2 || visRect.y + visRect.height < y1))
				{
					// now use exact check of page itself:
					Rectangle pageRect = pageView->getRect();
					if (!(visRect.x > pageRect.x + pageRect.width ||
					      visRect.x + visRect.width < pageRect.x) // visrect not outside current page dimensions
					    && !(visRect.y > pageRect.y + pageRect.height || visRect.y + visRect.height < pageRect.y))
					{
						pageView->setIsVisible(true);
					}
					else
					{
						pageView->setIsVisible(false);
					}
				}
				else
				{
					pageView->setIsVisible(false);

				}
			}
			x1 = x2;
		}
		y1 = y2;
	}

}

Rectangle Layout::getVisibleRect()
{
	XOJ_CHECK_TYPE(Layout);

	return Rectangle(gtk_adjustment_get_value(scrollHandling->getHorizontal()),
	                 gtk_adjustment_get_value(scrollHandling->getVertical()),
	                 gtk_adjustment_get_page_size(scrollHandling->getHorizontal()),
	                 gtk_adjustment_get_page_size(scrollHandling->getVertical()));
}

int addif(int lhs, int rhs, bool b)
{
	if (b)
	{
		return lhs + rhs;
	}
	return lhs;
}


void Layout::recalculate()
{
	XOJ_CHECK_TYPE(Layout);

	auto* settings = view->getControl()->getSettings();
	auto len = view->viewPagesLen;
	mapper.configureFromSettings(len, settings);
	auto colCount = mapper.getColumns();
	auto rowCount = mapper.getRows();

	widthCols.assign(colCount, 0);
	heightRows.assign(rowCount, 0);

	//Todo: Mapper invert function  (index -> x,y) it is possible and faster!
	for (size_t r = 0; r < rowCount; r++)
	{
		for (size_t c = 0; c < colCount; c++)
		{
			auto page = mapper.map(c, r);
			if (page != -1)
			{
				auto* v = view->viewPages[page];
				widthCols[c] = std::max(widthCols[c], v->getDisplayWidth());
				heightRows[r] = std::max(heightRows[r], v->getDisplayHeight());
			}
		}
	}

	//add space around the entire page area to accomodate older Wacom tablets with limited sense area.
	auto vPadding = addif(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
	auto hPadding = addif(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

	minWidth = 2 * hPadding + (widthCols.size() - 1) * XOURNAL_PADDING_BETWEEN;
	minHeight = 2 * vPadding + (heightRows.size() - 1) * XOURNAL_PADDING_BETWEEN;

	minWidth = std::accumulate(begin(widthCols), end(widthCols), minWidth);
	minHeight = std::accumulate(begin(heightRows), end(heightRows), minHeight);

	setLayoutSize(minWidth, minHeight);
	valid = true;
}

/**
 * layoutPages
 *  Sets out pages in a grid.
 *  Document pages are assigned to grid positions by the mapper object and may be ordered in a myriad of ways.
 */
void Layout::layoutPages(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	if (!valid)
	{
		recalculate();
	}
	valid = false;

	auto len = this->view->viewPagesLen;
	Settings* settings = this->view->getControl()->getSettings();

	// get from mapper (some may have changed to accomodate paired setting etc.)
	bool isPairedPages = this->mapper.getPairedPages();

	auto rows = this->heightRows.size();
	auto columns = this->widthCols.size();

	this->lastGetViewAtRow = rows / 2;        //reset to middle
	this->lastGetViewAtCol = columns / 2;

	//add space around the entire page area to accomodate older Wacom tablets with limited sense area.
	auto v_padding = addif(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
	auto h_padding = addif(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

	//Calculate border offset which will center pages in viewing area
	int minRequiredWidth = XOURNAL_PADDING_BETWEEN * (columns - 1);
	for (auto const& c: this->widthCols)
	{
		minRequiredWidth += c;
	}
	int centeringXBorder = (width - minRequiredWidth) / 2;    // this will center if all pages fit on screen.

	int minRequiredHeight = XOURNAL_PADDING_BETWEEN * (rows - 1);
	for (auto const& r: this->heightRows)
	{
		minRequiredHeight += r;
	}
	int centeringYBorder =
			(height - minRequiredHeight) / 2;    // this will center if all pages fit on screen vertically.

	int borderX = MAX (h_padding, centeringXBorder);
	int borderY = MAX (v_padding, centeringYBorder);

	// initialize here and x again in loop below.
	int x = borderX;
	int y = borderY;

	// Iterate over ALL possible rows and columns. 
	//We don't know which page, if any,  is to be displayed in each row, column -  ask the mapper object!
	//Then assign that page coordinates with center, left or right justify within row,column grid cell as required.
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < columns; c++)
		{
			int pageAtRowCol = this->mapper.map(c, r);

			if (pageAtRowCol >= 0)
			{

				XojPageView* v = this->view->viewPages[pageAtRowCol];
				v->setMappedRowCol(r, c);                    //store row and column for e.g. proper arrow key navigation
				int vDisplayWidth = v->getDisplayWidth();
				{
					int paddingLeft;
					int paddingRight;
					int columnPadding = this->widthCols[c] - vDisplayWidth;

					if (isPairedPages && len > 1)
					{
						// pair pages mode
						if (c % 2 == 0)
						{
							//align right
							paddingLeft = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
							paddingRight = XOURNAL_ROOM_FOR_SHADOW;
						}
						else
						{                                //align left
							paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
							paddingRight = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
						}
					}
					else
					{    // not paired page mode - center
						paddingLeft = XOURNAL_PADDING_BETWEEN / 2 + columnPadding / 2;      //center justify
						paddingRight = XOURNAL_PADDING_BETWEEN - paddingLeft + columnPadding / 2;
					}

					x += paddingLeft;

					v->setX(x);        //set the page position
					v->setY(y);

					x += vDisplayWidth + paddingRight;
				}
			}
			else
			{
				x += this->widthCols[c] + XOURNAL_PADDING_BETWEEN;
			}
		}
		x = borderX;
		y += this->heightRows[r] + XOURNAL_PADDING_BETWEEN;

	}

	int totalWidth = borderX;
	for (int c = 0; c < columns; c++)
	{
		totalWidth += this->widthCols[c] + XOURNAL_PADDING_BETWEEN;
		this->widthCols[c] = totalWidth;    //accumulated - absolute pixel location for use by getViewAt() and updateVisibility()
	}

	int totalHeight = borderY;
	for (int r = 0; r < rows; r++)
	{
		totalHeight += this->heightRows[r] + XOURNAL_PADDING_BETWEEN;
		this->heightRows[r] = totalHeight;
	}
}

void Layout::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);
	this->scrollHandling->setLayoutSize(width, height);
}

void Layout::scrollRelative(double x, double y)
{
	XOJ_CHECK_TYPE(Layout);

	if (this->view->getControl()->getSettings()->isPresentationMode())
	{
		return;
	}

	gtk_adjustment_set_value(scrollHandling->getHorizontal(),
	                         gtk_adjustment_get_value(scrollHandling->getHorizontal()) + x);
	gtk_adjustment_set_value(scrollHandling->getVertical(),
	                         gtk_adjustment_get_value(scrollHandling->getVertical()) + y);
}

void Layout::scrollAbs(double x, double y)
{
	XOJ_CHECK_TYPE(Layout);

	if (this->view->getControl()->getSettings()->isPresentationMode())
	{
		return;
	}

	gtk_adjustment_set_value(scrollHandling->getHorizontal(), x);
	gtk_adjustment_set_value(scrollHandling->getVertical(), y);
}


void Layout::ensureRectIsVisible(int x, int y, int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_clamp_page(scrollHandling->getHorizontal(), x - 5, x + width + 10);
	gtk_adjustment_clamp_page(scrollHandling->getVertical(), y - 5, y + height + 10);
}


XojPageView* Layout::getViewAt(int x, int y)
{

	XOJ_CHECK_TYPE(Layout);

//  No need to check page cache as the Linear search below starts at cached position.
//  Keep Binary search handy to check against.
//	
//  //Binary Search ... too much overhead makes this a slower option in our use case.
// 	auto rit = std::lower_bound( this->heightRows.begin(),  this->heightRows.end(), y);
// 	int rb = rit - this->heightRows.begin();	//get index
// 	auto cit = std::lower_bound( this->widthCols.begin(),  this->widthCols.end(), x);
// 	int cb = cit - this->widthCols.begin();

	auto numRows = this->heightRows.size();
	auto numCols = this->widthCols.size();

	/* Linear Up or Down Search from last position: */

	// Rows:
	int testRow = MAX(0, this->lastGetViewAtRow - 1);
	if (testRow > 0 && y <= this->heightRows[testRow]) //search lower
	{
		for (testRow--; testRow >= 0; testRow--)
		{
			if (y > this->heightRows[testRow])
			{
				break;    // past region
			}
		}
		testRow++;    // it's back up one
	}
	else    //search higher
	{
		for (; testRow < numRows; testRow++)
		{
			if (y <= this->heightRows[testRow])
			{
				break;    // found region
			}
		}

	}


	//Now for columns:
	int testCol = MAX(0, this->lastGetViewAtCol - 1);
	if (testCol > 0 && x <= this->widthCols[testCol]) //search lower
	{
		for (testCol--; testCol >= 0; testCol--)
		{
			if (x > this->widthCols[testCol])
			{
				break;
			}
		}
		testCol++;
	}
	else
	{
		for (; testCol < numCols; testCol++)
		{
			if (x <= this->widthCols[testCol])
			{
				break;
			}
		}

	}


	if (testCol < numCols && testRow < numRows)
	{
		int page = this->mapper.map(testCol, testRow);
		this->lastGetViewAtRow = testRow;
		this->lastGetViewAtCol = testCol;

		if (page >= 0 && this->view->viewPages[page]->containsPoint(x, y, false))
		{
			return this->view->viewPages[page];
		}

	}

	return NULL;
}

int Layout::getIndexAtGridMap(int row, int col)
{
	return this->mapper.map(col, row); //watch out.. x,y --> c,r

}

int Layout::getMinimalHeight()
{
	return this->minHeight;
}

int Layout::getMinimalWidth()
{
	return this->minWidth;
}
