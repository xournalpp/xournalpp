#include "Layout.h"

#include "XournalView.h"

#include "control/Control.h"
#include "pageposition/PagePositionHandler.h"
#include "widgets/XournalWidget.h"
#include "gui/scroll/ScrollHandling.h"

// TODO: REMOVE AFTER TEST
	#include <iostream>
	#include <chrono>
	typedef std::chrono::high_resolution_clock Clock;



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


// TODO: REMOVE AFTER TEST
double binarySearchtime = 0;
double linearSearchTime = 0;
double fromLastLinearSearchTime = 0;
double fromCalcLinearSearchTime = 0;
double fromB4LastLinearSearchTime = 0;
double fromB4LastLinearBackSearchTime = 0;
double plotindex = 0;
						
						


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
			layout->updateCurrentPage();
			layout->scrollHandling->scrollChanged();
		}), this);

	g_signal_connect(scrollHandling->getVertical(), "value-changed", G_CALLBACK(
		+[](GtkAdjustment* adjustment, Layout* layout)
		{
			XOJ_CHECK_TYPE_OBJ(layout, Layout);
			layout->checkScroll(adjustment, layout->lastScrollVertical);
			layout->updateCurrentPage();
			layout->scrollHandling->scrollChanged();
		}), this);
	
	this->lastGetViewAtPage = 0;
	this->lastGetViewRow = 0;
	this->lastGetViewCol = 0;

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
 * Check which page should be selected
 */
void Layout::updateCurrentPage()
{
	XOJ_CHECK_TYPE(Layout);

	Rectangle visRect = getVisibleRect();

	Control* control = this->view->getControl();

	bool pairedPages = control->getSettings()->isShowPairedPages();

	if (visRect.y < 1)
	{
		if (pairedPages && this->view->viewPagesLen > 1 &&
		    this->view->viewPages[1]->isSelected())
		{
			// page 2 already selected
		}
		else
		{
			control->firePageSelected(0);
		}
		return;
	}

	size_t mostPageNr = 0;
	double mostPagePercent = 0;

	for (size_t page = 0; page < this->view->viewPagesLen; page++)
	{
		XojPageView* p = this->view->viewPages[page];

		Rectangle currentRect = p->getRect();

		// if we are already under the visible rectangle
		// then everything below will not be visible...
		if(currentRect.y > visRect.y + visRect.height)
		{
			p->setIsVisible(false);
			for (; page < this->view->viewPagesLen; page++)
			{
				p = this->view->viewPages[page];
				p->setIsVisible(false);
			}

			break;
		}

		// if the condition is satisfied we know that
		// the rectangles intersect vertically
		if (currentRect.y + currentRect.height >= visRect.y)
		{

			double percent =
				currentRect.intersect(visRect).area() / currentRect.area();

			if (percent > mostPagePercent)
			{
				mostPagePercent = percent;
				mostPageNr = page;
			}

			p->setIsVisible(true);
		}
		else
		{
			p->setIsVisible(false);
		}
	}

	if (pairedPages && mostPageNr < this->view->viewPagesLen - 1)
	{
		int y1 = this->view->viewPages[mostPageNr]->getY();
		int y2 = this->view->viewPages[mostPageNr + 1]->getY();

		if (y1 != y2 || !this->view->viewPages[mostPageNr + 1]->isSelected())
		{
			// if the second page is selected DON'T select the first page.
			// Only select the first page if none is selected
			control->firePageSelected(mostPageNr);
		}
	}
	else
	{
		control->firePageSelected(mostPageNr);
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

void Layout::layoutPages()
{
	XOJ_CHECK_TYPE(Layout);

	int len = this->view->viewPagesLen;
	
	this->lastGetViewAtPage = len;  // invalidate cache index.
	this->lastGetViewRow = 0;
	this->lastGetViewCol = 0;
	
	Settings* settings = this->view->getControl()->getSettings();

	// obtain rows, cols, paired and layout from view settings
	mapper.configureFromSettings(len, settings);

	// get from mapper (some may have changed to accomodate paired setting etc.)
	bool isPairedPages = mapper.getPairedPages();
	int pagesOffset = mapper.getFirstPageOffset();
	this->rows = mapper.getRows();
	this->columns = mapper.getColumns();


	this->sizeCol.assign(this->columns,0); //new size, clear to 0's

	this->sizeRow.assign(this->rows,0);

	for (int r = 0; r < this->rows; r++)
	{
		for (int c = 0; c < this->columns; c++)
		{
			int k = mapper.map(c, r);
			if (k >= 0)
			{

				XojPageView* v = this->view->viewPages[k];

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
	for( int c = 0 ; c< columns; c++ )
	{
		minRequiredWidth += this->sizeCol[c];
	}
	int centeringXBorder = ( visibleWidth - minRequiredWidth )/2;	// this will center if all pages fit on screen.

	
	int visibleHeight = gtk_adjustment_get_page_size(scrollHandling->getVertical());
	int minRequiredHeight = XOURNAL_PADDING_BETWEEN * (rows-1);
	for( int r = 0 ; r< rows; r++ )
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
			int pageAtRowCol = mapper.map(c, r);

			if (pageAtRowCol >= 0)
			{

				XojPageView* v = this->view->viewPages[pageAtRowCol];
				int vDisplayWidth = v->getDisplayWidth();
				{
					int paddingLeft;
					int paddingRight;
					int columnPadding = 0;
					columnPadding = (this->sizeCol[c] - vDisplayWidth);

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

					v->layout.setX(x);
					v->layout.setY(y);

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
		this->sizeCol[c] = totalWidth;	//accumulated for use by getViewAt()
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
	this->view->pagePosition->update(this->view->viewPages, len, totalHeight);
	
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

//	//try cached result first
	if  ( this->lastGetViewAtPage < this->view->viewPagesLen    &&     this->view->viewPages[this->lastGetViewAtPage]->containsPoint(x,y,false) )
	{
			return this->view->viewPages[this->lastGetViewAtPage];
	}
	
	int r;
	int c;
	int numRows = this->mapper.getRows();
	int numCols = this->mapper.getColumns();
	

	auto t1 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	BINARY SEARCH 	***************

		
			  
			  
	auto rit = std::lower_bound( this->sizeRow.begin(),  this->sizeRow.end(), y);	//binary search
	r = rit -  this->sizeRow.begin();	//get index
	
	
	auto cit = std::lower_bound( this->sizeCol.begin(),  this->sizeCol.end(), x);
	c = cit -  this->sizeCol.begin();


	auto t2 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	LINEAR SEARCH: From 0	***************


	/* test against linear search: */
	int r1;
	for( r1 = 0; r1 < numRows; r1++)
	{
		if ( y <=   this->sizeRow[r1] ) break;	// found region
	}

	int c1;
	for( c1 = 0; c1 < numCols; c1++)
	{
		if ( x <=  this->sizeCol[c1]) break;
	}
	//*/
	

	auto t3 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	LINEAR SEARCH: From Previous 	***************

						
	/* Linear Search from last position: */
	
	// Rows:
	int r2 = this->lastGetViewRow;
	if( r2 > 0 && y <= this->sizeRow[r2] ) //search lower
	{
		for( r2--; r2>=0; r2--)
		{
			if ( y >   this->sizeRow[r2] ) 
			{	
				break;	// past region
			}
		}
		r2++;
	}
	else
	{
		for( ;  r2 < numRows; r2++)
		{
			if ( y <=   this->sizeRow[r2] ) break;	// found region
		}		
		
	}
	
	
	//Columns:
	int c2 = this->lastGetViewCol;						
	if( c2 >0 && x <= this->sizeCol[c2] ) //search lower
	{
		for( c2--; c2>=0; c2--)
		{
			if ( x >   this->sizeCol[c2] ) 
			{	
				break;	// past region
			}
		}
		c2++;
	}
	else
	{
		for( ;  c2 < numCols; c2++)
		{
			if ( x <=   this->sizeCol[c2] ) break;	// found region
		}		
		
	}
	

							auto t4 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	LINEAR SEARCH: From Approximation - would be bad with irregular sized pages - bi-directional	***************

						
	/* Linear Search from calculated position: */
	
	// Rows:
	int r3 = y / ( this->sizeRow[numRows-1]/numRows);
	int calcR = r3;
	if( r3 > 0 && y <= this->sizeRow[r3] ) //search lower
	{
		for( r3--; r3>=0; r3--)
		{
			if ( y >   this->sizeRow[r3] ) 
			{	
				break;	// past region
			}
		}
		r3++;
	}
	else
	{
		for( ;  r3 < numRows; r3++)
		{
			if ( y <=   this->sizeRow[r3] ) break;	// found region
		}		
		
	}
	
	
	//Columns:					
	int c3 = x / ( this->sizeCol[numCols-1]/numCols);
	int calcC = c3;
	if( c3 >0 && x <= this->sizeCol[c3] ) //search lower
	{
		for( c3--; c3>=0; c3--)
		{
			if ( x >   this->sizeCol[c3] ) 
			{	
				break;	// past region
			}
		}
		c3++;
	}
	else
	{
		for( ;  c3 < numCols; c3++)
		{
			if ( x <=   this->sizeCol[c3] ) break;	// found region
		}		
		
	}			
		
		
		
	auto t5 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	LINEAR SEARCH: From previous - 2	forward only ***************


	/* test against linear search: */
	int r4 = MAX(0, this->lastGetViewRow - 3);
	if ( y >   this->sizeRow[r4] )	//search forward
	{
		r4++;
		for( ; r4 < numRows; r4++)
		{
			if ( y <=   this->sizeRow[r4] ) break;	// found region
		}
	}
	else
	{
		for( r4 = 0; r4 < numRows; r4++)
		{
			if ( y <=   this->sizeRow[r4] ) break;	// found region
		}
	}
	

	int c4 = MAX(0, this->lastGetViewCol - 3);
	if ( x >  this->sizeCol[c4])	//search forward
	{
		c4++;	
		for( ; c4 < numCols; c4++)
		{
			if ( x <=  this->sizeCol[c4]) break;
		}
	}
	else
	{
		for( c4 = 0; c4 < numCols; c4++)
		{
			if ( x <=  this->sizeCol[c4]) break;
		}
	}
	//*/
	
	
		auto t6 = Clock::now();	// TODO: REMOVE AFTER TEST					************* 	LINEAR SEARCH: From Previous - Bidriectional	***************

						
	/* Linear Search from last position: */
	
	// Rows:
	int r5 = MAX(0, this->lastGetViewRow - 1);
	if( r5 > 0 && y <= this->sizeRow[r5] ) //search lower
	{
		for( r5--; r5>=0; r5--)
		{
			if ( y >   this->sizeRow[r5] ) 
			{	
				break;	// past region
			}
		}
		r5++;
	}
	else
	{
		for( ;  r5 < numRows; r5++)
		{
			if ( y <=   this->sizeRow[r5] ) break;	// found region
		}		
		
	}
	
	
	//Columns:
	int c5 = MAX(0, this->lastGetViewCol - 1);			
	if( c5 >0 && x <= this->sizeCol[c5] ) //search lower
	{
		for( c5--; c5>=0; c5--)
		{
			if ( x >   this->sizeCol[c5] ) 
			{	
				break;	// past region
			}
		}
		c5++;
	}
	else
	{
		for( ;  c5 < numCols; c5++)
		{
			if ( x <=   this->sizeCol[c5] ) break;	// found region
		}		
		
	}
	

	
	

		
							// TODO: REMOVE BELOW AFTER TEST
							auto t7 = Clock::now();
							
							binarySearchtime += std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
							linearSearchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
							fromLastLinearSearchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
							fromCalcLinearSearchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(t5 - t4).count();
							fromB4LastLinearSearchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count();
							fromB4LastLinearBackSearchTime += std::chrono::duration_cast<std::chrono::nanoseconds>(t7 - t6).count();
								
							string cOK =  ( c == c1 &&  c == c2 && c == c3 &&  c == c4 && c == c5)? "cOK" : "cBAD";  
							string rOK =  ( r == r1 &&  r == r2 && r == r3 &&  r == r4 && r == r5)? "rOK" : "rBAD";  
								if( plotindex == 0 )
								{
									std::clog << " index \t binary \tlinear \t fromlast \t fromCalc \t fromB4Last \t fromB4LastBiDir" << std::endl;
								}
								std::clog << plotindex++ << '\t' 
									<< binarySearchtime << '\t'
									<< linearSearchTime << '\t'
									<< fromLastLinearSearchTime << "\t"
									<< fromCalcLinearSearchTime << "\t"
									<< fromB4LastLinearSearchTime << "\t"									
									<< fromB4LastLinearBackSearchTime 
									<< "\t\t"									
									<< c << '=' << c1 << '=' << c2 << '=' << c3 << '=' << c4 << '=' << c5 <<" \t" << cOK << "\t"
									<< r << '=' << r1 << '=' << r2 << '=' << r3 << '=' << r4 << '=' << r5 << " \t" << rOK << "\t"
									<< " \t Last " << this->lastGetViewCol << ":" << this->lastGetViewRow << "\t\t"
									<< " \t Calc " << calcC << ":" << calcR
									<< std::endl;
									
							c = c3;
							r = r3;


		
	
	if ( c <= numCols  && r <= numRows ) 
	{
		int page = this->mapper.map(c,r);
		this->lastGetViewAtPage = page;
		this->lastGetViewRow = r;
		this->lastGetViewCol = c;

		if ( page>=0 && this->view->viewPages[page]->containsPoint(x,y,false) )
		{
			return this->view->viewPages[page];
		}
		
	}
	
	return NULL;
	
	
}
