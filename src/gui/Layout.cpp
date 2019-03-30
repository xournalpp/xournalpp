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

	g_signal_connect(scrollHandling->getHorizontal(), "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollHorizontal);
			layout->updateVisibility();
			layout->scrollHandling->scrollChanged();
		}), this);

	g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollVertical);
			layout->updateVisibility();
			layout->scrollHandling->scrollChanged();
		}), this);
	

	lastScrollHorizontal = gtk_adjustment_get_value(scrollHandling->getHorizontal());
	lastScrollVertical = gtk_adjustment_get_value(scrollHandling->getVertical());
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
 	
	for (int onRow = 0; onRow < this->rows; onRow++)
	{
		int y2 = this->sizeRow[onRow];
		for (int onCol = 0; onCol < this->columns; onCol++)
		{
			int x2 = this->sizeCol[onCol];		
			int pageIndex = this->mapper.map(onCol, onRow);
			if (pageIndex >= 0)	// a page exists at this grid location
			{

				XojPageView* pageView = this->view->viewPages[pageIndex];
				
				
				//check if grid location is visible as an aprox for page visiblity:
				if(		!(visRect.x >x2  || visRect.x+visRect.width < x1) // visrect not outside current row/col 
					&& 	!(visRect.y >y2  || visRect.y+visRect.height < y1))
				{
					// now use exact check of page itself:
					Rectangle pageRect = pageView->getRect();
					if(		!(visRect.x >pageRect.x+pageRect.width  || visRect.x+visRect.width < pageRect.x) // visrect not outside current page dimensions 
						&& 	!(visRect.y >pageRect.y+pageRect.height  || visRect.y+visRect.height < pageRect.y))
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

/**
 * Returns the height of the entire Layout
 */
double Layout::getLayoutHeight()
{
	XOJ_CHECK_TYPE(Layout);

	return layoutHeight;
}

/**
 * Returns the width of the entire Layout
 */
double Layout::getLayoutWidth()
{
	XOJ_CHECK_TYPE(Layout);

	return layoutWidth;
}


/**
 * layoutPages
 *  Sets out pages in a grid.
 *  Document pages are assigned to grid positions by the mapper object and may be ordered in a myriad of ways.
 */
void Layout::layoutPages()
{
	XOJ_CHECK_TYPE(Layout);

	int len = this->view->viewPagesLen;
	
	
	Settings* settings = this->view->getControl()->getSettings();

	// obtain rows, cols, paired and layout from view settings
	this->mapper.configureFromSettings(len, settings);

	// get from mapper (some may have changed to accomodate paired setting etc.)
	bool isPairedPages = this->mapper.getPairedPages();
	this->rows = this->mapper.getRows();
	this->columns = this->mapper.getColumns();
	
	this->lastGetViewAtRow = this->rows/2;		//reset to middle
	this->lastGetViewAtCol = this->columns/2;
	

	this->sizeCol.assign(this->columns,0); //new size, clear to 0's

	this->sizeRow.assign(this->rows,0);
	
	// look through every grid position, get assigned page from mapper and find out minimal row and column sizes to fit largest pages.

	for (int r = 0; r < this->rows; r++)
	{
		for (int c = 0; c < this->columns; c++)
		{
			int pageIndex = this->mapper.map(c, r);
			if (pageIndex >= 0)
			{

				XojPageView* v = this->view->viewPages[pageIndex];

				if (this->sizeCol[c] < v->getDisplayWidth())
				{
					this->sizeCol[c] = v->getDisplayWidth();
				}
				if (this->sizeRow[r] < v->getDisplayHeight())
				{
					this->sizeRow[r] = v->getDisplayHeight();
				}
			}

		}
	}

	
	
	
	//add space around the entire page area to accomodate older Wacom tablets with limited sense area.
	int borderPrefX =  XOURNAL_PADDING;
	if (settings->getAddHorizontalSpace() )
	{
		borderPrefX += XOURNAL_PADDING_FREE_SPACE;	// this adds extra space to the left and right 
	}
	
	int borderPrefY = XOURNAL_PADDING;
	if (settings->getAddVerticalSpace() )
	{
		borderPrefY += XOURNAL_PADDING_FREE_SPACE;	// this adds space to the top and bottom
	}	

	
	
	//Calculate border offset which will center pages in viewing area
	int visibleWidth = gtk_adjustment_get_page_size(scrollHandling->getHorizontal());
	int minRequiredWidth = XOURNAL_PADDING_BETWEEN * (columns-1);
	for (int c = 0 ; c< columns; c++ )
	{
		minRequiredWidth += this->sizeCol[c];
	}
	int centeringXBorder = ( visibleWidth - minRequiredWidth )/2;	// this will center if all pages fit on screen.

	
	int visibleHeight = gtk_adjustment_get_page_size(scrollHandling->getVertical());
	int minRequiredHeight = XOURNAL_PADDING_BETWEEN * (rows-1);
	for (int r = 0 ; r< rows; r++ )
	{
		minRequiredHeight += this->sizeRow[r];
	}
	int centeringYBorder = ( visibleHeight - minRequiredHeight )/2;	// this will center if all pages fit on screen vertically.

	
		
	int borderX = MAX ( borderPrefX , centeringXBorder);
	int borderY = MAX ( borderPrefY , centeringYBorder);
	
	
	// initialize here and x again in loop below.
	int x = borderX;
	int y = borderY;


	// Iterate over ALL possible rows and columns. 
	//We don't know which page, if any,  is to be displayed in each row, column -  ask the mapper object!
	//Then assign that page coordinates with center, left or right justify within row,column grid cell as required.
	for (int r = 0; r < this->rows; r++)
	{
		for (int c = 0; c < this->columns; c++)
		{
			int pageAtRowCol = this->mapper.map(c, r);

			if (pageAtRowCol >= 0)
			{

				XojPageView* v = this->view->viewPages[pageAtRowCol];
				v->setMappedRowCol( r,c );  					//store row and column for e.g. proper arrow key navigation
				int vDisplayWidth = v->getDisplayWidth();
				{
					int paddingLeft;
					int paddingRight;
					int columnPadding = this->sizeCol[c] - vDisplayWidth;

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
				x += this->sizeCol[c] + XOURNAL_PADDING_BETWEEN;
			}
		}
		x = borderX;
		y += this->sizeRow[r] + XOURNAL_PADDING_BETWEEN;

	}


	int totalWidth = borderX;
	for (int c = 0; c < this->columns; c++)
	{
		totalWidth += this->sizeCol[c] + XOURNAL_PADDING_BETWEEN;
		this->sizeCol[c] = totalWidth;	//accumulated - absolute pixel location for use by getViewAt() and updateVisibility()
	}
	totalWidth += borderX - XOURNAL_PADDING_BETWEEN;

	
	int totalHeight = borderY;
	for (int r = 0; r < this->rows; r++)
	{
		totalHeight += this->sizeRow[r]+ XOURNAL_PADDING_BETWEEN;
		this->sizeRow[r] = totalHeight; 
	}
	totalHeight += borderY - XOURNAL_PADDING_BETWEEN;


	this->setLayoutSize(totalWidth, totalHeight);
	
}


void Layout::setLayoutSize(int width, int height)
{
	XOJ_CHECK_TYPE(Layout);

	this->layoutHeight = height;
	this->layoutWidth = width;

	this->scrollHandling->setLayoutSize(width, height);
}

void Layout::setSize(int widgetWidth, int widgetHeight)
{
	XOJ_CHECK_TYPE(Layout);

	if (this->lastWidgetWidth != widgetWidth)
	{
		this->layoutPages();
		this->lastWidgetWidth = widgetWidth;
	}
	else
	{
		this->setLayoutSize(this->layoutWidth, this->layoutHeight);
	}

	this->view->getControl()->calcZoomFitSize();
}

void Layout::scrollRelativ(int x, int y)
{
	XOJ_CHECK_TYPE(Layout);

	gtk_adjustment_set_value(scrollHandling->getHorizontal(), gtk_adjustment_get_value(scrollHandling->getHorizontal()) + x);
	gtk_adjustment_set_value(scrollHandling->getVertical(), gtk_adjustment_get_value(scrollHandling->getVertical()) + y);
}

void Layout::scrollAbs(int x, int y)
{
	XOJ_CHECK_TYPE(Layout);

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



	int numRows = this->mapper.getRows();
	int numCols = this->mapper.getColumns();

						
	/* Linear Up or Down Search from last position: */
	
	// Rows:
	int testRow = MAX(0, this->lastGetViewAtRow - 1);
	if (testRow > 0 && y <= this->sizeRow[testRow]) //search lower
	{
		for (testRow--; testRow>=0; testRow--)
		{
			if ( y >   this->sizeRow[testRow] ) 
			{
				break;	// past region
			}
		}
		testRow++;	// it's back up one
	}
	else	//search higher
	{
		for (; testRow < numRows; testRow++)
		{
			if (y <=   this->sizeRow[testRow]) 
			{
				break;	// found region
			}
		}
		
	}
	
	
	//Now for columns:
	int testCol = MAX(0, this->lastGetViewAtCol - 1);			
	if (testCol >0 && x <= this->sizeCol[testCol]) //search lower
	{
		for (testCol--; testCol>=0; testCol--)
		{
			if (x >   this->sizeCol[testCol]) 
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
			if (x <=   this->sizeCol[testCol])
			{
				break;
			}
		}
		
	}
	
	
	if (testCol < numCols && testRow < numRows) 
	{
		int page = this->mapper.map(testCol,testRow);
		this->lastGetViewAtRow = testRow;
		this->lastGetViewAtCol = testCol;

		if (page>=0 && this->view->viewPages[page]->containsPoint(x,y,false))
		{
			return this->view->viewPages[page];
		}
		
	}
	
	return NULL;
	
	
}


int Layout::getIndexAtGridMap(int row, int col)
{
	return this->mapper.map( col, row); //watch out.. x,y --> c,r
	
}
