#include "Layout.h"

#include "XournalView.h"

#include "control/Control.h"
#include "widgets/XournalWidget.h"
#include "gui/scroll/ScrollHandling.h"

#include <algorithm>
#include <numeric>
#include <cstdlib>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
#include <iostream>


/**
 * Padding outside the pages, including shadow
 */
constexpr size_t const XOURNAL_PADDING = 10;

/**
 * Allowance for shadow between page pairs in paired page mode
 */
constexpr size_t const XOURNAL_ROOM_FOR_SHADOW = 3;

/**
 * Padding between the pages
 */
constexpr size_t const XOURNAL_PADDING_BETWEEN = 15;


double searchtime1 = 0;
double searchtime2 = 0;
double searchtime3 = 0;
double plotindex = 0;


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

void Layout::updateVisibility()
{
	XOJ_CHECK_TYPE(Layout);

	Rectangle visRect = getVisibleRect();
	
	// step through every possible page position and update using p->setIsVisible()
	// Using initial grid aprox speeds things up by a factor of 5.  See previous git check-in for specifics.
	int x1 = 0;
	int y1 = 0;

	for (size_t row = 0; row < this->heightRows.size(); ++row)
	{
		int y2 = this->heightRows[row];
		for (size_t col = 0; col < this->widthCols.size(); ++col)
		{
			int x2 = this->widthCols[col];
			auto optionalPage = this->mapper.at({col, row});
			if (optionalPage)  // a page exists at this grid location
			{
				XojPageView* pageView = this->view->viewPages[*optionalPage];


				//check if grid location is visible as an aprox for page visiblity:
				if (!(visRect.x > x2 || visRect.x + visRect.width < x1)  // visrect not outside current row/col
				    && !(visRect.y > y2 || visRect.y + visRect.height < y1))
				{
					// now use exact check of page itself:
					// visrect not outside current page dimensions:
					Rectangle pageRect = pageView->getRect();
					pageView->setIsVisible(
					        !(visRect.x > pageRect.x + pageRect.width || visRect.x + visRect.width < pageRect.x) &&
					        !(visRect.y > pageRect.y + pageRect.height || visRect.y + visRect.height < pageRect.y));
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


/**
 * adds the addend to base if the predicate is true
 */
inline size_t sumIf(size_t base, size_t addend, bool predicate)
{
	if (predicate)
	{
		return base + addend;
	}
	return base;
}


void Layout::recalculate()
{
	XOJ_CHECK_TYPE(Layout);

	auto* settings = view->getControl()->getSettings();
	size_t len = view->viewPagesLen;
	mapper.configureFromSettings(len, settings);
	size_t colCount = mapper.getColumns();
	size_t rowCount = mapper.getRows();

	widthCols.assign(colCount, 0);
	heightRows.assign(rowCount, 0);

	for (size_t pageIdx{}; pageIdx < len; ++pageIdx)
	{
		auto const& raster_p = mapper.at(pageIdx);  //auto [c, r] raster = mapper.at();
		auto const& c = raster_p.first;
		auto const& r = raster_p.second;
		XojPageView* v = view->viewPages[pageIdx];
		widthCols[c] = std::max<unsigned>(widthCols[c], v->getDisplayWidth());
		heightRows[r] = std::max<unsigned>(heightRows[r], v->getDisplayHeight());
	}

	//add space around the entire page area to accomodate older Wacom tablets with limited sense area.
	size_t const vPadding =
	        sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
	size_t const hPadding =
	        sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

	minWidth = 2 * hPadding + (widthCols.size() - 1) * XOURNAL_PADDING_BETWEEN;
	minHeight = 2 * vPadding + (heightRows.size() - 1) * XOURNAL_PADDING_BETWEEN;

	minWidth = std::accumulate(begin(widthCols), end(widthCols), minWidth);
	minHeight = std::accumulate(begin(heightRows), end(heightRows), minHeight);

	setLayoutSize(minWidth, minHeight);
	valid = true;
}

void Layout::layoutPages(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	if (!valid)
	{
		recalculate();
	}
	valid = false;

	size_t const len = this->view->viewPagesLen;
	Settings* settings = this->view->getControl()->getSettings();

	// get from mapper (some may have changed to accomodate paired setting etc.)
	bool const isPairedPages = this->mapper.isPairedPages();

	auto const rows = this->heightRows.size();
	auto const columns = this->widthCols.size();

	this->lastGetViewAtRow = rows / 2;  //reset to middle
	this->lastGetViewAtCol = columns / 2;

	//add space around the entire page area to accomodate older Wacom tablets with limited sense area.
	int64_t const v_padding =
	        sumIf(XOURNAL_PADDING, settings->getAddVerticalSpaceAmount(), settings->getAddVerticalSpace());
	int64_t const h_padding =
	        sumIf(XOURNAL_PADDING, settings->getAddHorizontalSpaceAmount(), settings->getAddHorizontalSpace());

	int64_t const centeringXBorder = static_cast<int64_t>(width - minWidth) / 2;
	int64_t const centeringYBorder = static_cast<int64_t>(height - minHeight) / 2;

	int64_t const borderX = std::max<int64_t>(h_padding, centeringXBorder);
	int64_t const borderY = std::max<int64_t>(v_padding, centeringYBorder);

	// initialize here and x again in loop below.
	int64_t x = borderX;
	int64_t y = borderY;


	// Iterate over ALL possible rows and columns. 
	//We don't know which page, if any,  is to be displayed in each row, column -  ask the mapper object!
	//Then assign that page coordinates with center, left or right justify within row,column grid cell as required.
	for (size_t r = 0; r < rows; r++)
	{
		for (size_t c = 0; c < columns; c++)
		{
			auto optionalPage = this->mapper.at({c, r});

			if (optionalPage)
			{

				XojPageView* v = this->view->viewPages[*optionalPage];
				v->setMappedRowCol(r, c);  //store row and column for e.g. proper arrow key navigation
				int64_t vDisplayWidth = v->getDisplayWidth();
				{
					int64_t paddingLeft;
					int64_t paddingRight;
					auto columnPadding = static_cast<int64_t>(this->widthCols[c] - vDisplayWidth);

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
						{								//align left
							paddingLeft = XOURNAL_ROOM_FOR_SHADOW;
							paddingRight = XOURNAL_PADDING_BETWEEN - XOURNAL_ROOM_FOR_SHADOW + columnPadding;
						}
					}
					else
					{	// not paired page mode - center
						paddingLeft = XOURNAL_PADDING_BETWEEN / 2 + columnPadding / 2;      //center justify
						paddingRight = XOURNAL_PADDING_BETWEEN - paddingLeft + columnPadding / 2;
					}

					x += paddingLeft;

					v->setX(x);		//set the page position
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

	int64_t totalWidth = borderX;
	for (auto&& widthCol: this->widthCols)
	{
		//accumulated - absolute pixel location for use by getViewAt() and updateVisibility()
		totalWidth += widthCol + XOURNAL_PADDING_BETWEEN;
		widthCol = totalWidth;
	}

	int64_t totalHeight = borderY;
	for (auto&& heightRow: this->heightRows)
	{
		totalHeight += heightRow + XOURNAL_PADDING_BETWEEN;
		heightRow = totalHeight;
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

	if(this->view->getControl()->getSettings()->isPresentationMode())
	{
		return;
	}

	gtk_adjustment_set_value(scrollHandling->getHorizontal(), gtk_adjustment_get_value(scrollHandling->getHorizontal()) + x);
	gtk_adjustment_set_value(scrollHandling->getVertical(), gtk_adjustment_get_value(scrollHandling->getVertical()) + y);
}

void Layout::scrollAbs(double x, double y)
{
	XOJ_CHECK_TYPE(Layout);

	if(this->view->getControl()->getSettings()->isPresentationMode())
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
// 	auto rit = std::lower_bound( this->sizeRow.begin(),  this->sizeRow.end(), y);
// 	int rb = rit - this->sizeRow.begin();	//get index
// 	auto cit = std::lower_bound( this->sizeCol.begin(),  this->sizeCol.end(), x);
// 	int cb = cit - this->sizeCol.begin();

	auto t0 = Clock::now();

	auto fast_linear_search = [](std::vector<unsigned> const& container, size_t start, size_t value) {
		auto row_i = std::next(begin(container), start);
		if (row_i != begin(container) && value < *row_i)
		{
			//Todo: replace with c++14
			//Todo: rend(this->heightRows)
			//Todo: std::make_reverse_iterator(c_i)
			auto rbeg_i = std::reverse_iterator<decltype(row_i)>(row_i);
			return std::find_if(rbeg_i, container.rend(), [value](int item) { return value > item; }).base();
		}
		return std::find_if(row_i, end(container), [value](int item) { return value <= item; });
	};

	auto const& row_i = fast_linear_search(this->heightRows, this->lastGetViewAtRow, y);
	auto const& col_i = fast_linear_search(this->widthCols, this->lastGetViewAtCol, x);


	auto t1 = Clock::now();

	/* Linear Up or Down Search from last position: */
	/* This is over 7x faster than the above code  */

	// Rows:
	int r = std::max(0, static_cast<int>(this->lastGetViewAtRow) - 1);
	if (r > 0 && y <= this->heightRows[r])  //search lower
	{
		for (r--; r >= 0; r--)
		{
			if (y > this->heightRows[r])
			{
				break;  // past region - it's back up one
			}
		}
		r++;
	}
	else  //search higher
	{
		for (; r < this->heightRows.size(); r++)
		{
			if (y <= this->heightRows[r]) break;  // found region
		}
	}


	//Now for columns:
	int c = std::max(0, static_cast<int>(this->lastGetViewAtCol) - 1);
	if (c > 0 && x <= this->widthCols[c])  //search lower
	{
		for (c--; c >= 0; c--)
		{
			if (x > this->widthCols[c])
			{
				break;  // past region
			}
		}
		c++;
	}
	else
	{
		for (; c < this->widthCols.size(); c++)
		{
			if (x <= this->widthCols[c]) break;  // found region
		}
	}


	auto t2 = Clock::now();

	//Binary Search ... too much overhead makes this a slower option in our use case.
	/* This is over 2x faster than the first code block but 3x slower than the second code block*/
	auto rit = std::lower_bound(this->heightRows.begin(), this->heightRows.end(), y);
	int rb = rit - this->heightRows.begin();  //get index
	auto cit = std::lower_bound(this->widthCols.begin(), this->widthCols.end(), x);
	int cb = cit - this->widthCols.begin();

	auto t3 = Clock::now();

	searchtime1 += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
	searchtime2 += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	searchtime3 += std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();


	if (plotindex == 0)
	{
		std::clog << " index \tagreeR \t agreeC \t new \told \t linear" << std::endl;
	}
	//remove these along with timing etc when done testing:
	int r0 = std::distance(this->heightRows.cbegin(), row_i);
	int c0 = std::distance(this->widthCols.cbegin(), col_i);

	std::clog << plotindex++ << '\t' << r0 << '=' << r << '=' << rb << '\t' << c0 << '=' << c << '=' << cb << '\t'
	          << searchtime1 << '\t' << searchtime2 << '\t' << searchtime3 << '\t' << std::endl;


	if (col_i != end(this->widthCols) && row_i != end(this->heightRows))
	{
		//Todo c++14+ cbegin(...);
		this->lastGetViewAtRow = std::distance(this->heightRows.cbegin(), row_i);
		this->lastGetViewAtCol = std::distance(this->widthCols.cbegin(), col_i);
		auto optionalPage = this->mapper.at({this->lastGetViewAtCol, this->lastGetViewAtRow});


		if (optionalPage && this->view->viewPages[*optionalPage]->containsPoint(x, y, false))
		{
			return this->view->viewPages[*optionalPage];
		}
	}

	return nullptr;
}
// Todo replace with boost::optional<size_t> Layout::getIndexAtGridMap(size_t row, size_t col)
//                  or std::optional<size_t> Layout::getIndexAtGridMap(size_t row, size_t col)
LayoutMapper::optional_size_t Layout::getIndexAtGridMap(size_t row, size_t col)
{
	return this->mapper.at({col, row});  //watch out.. x,y --> c,r
}

int Layout::getMinimalHeight()
{
	return this->minHeight;
}

int Layout::getMinimalWidth()
{
	return this->minWidth;
}
