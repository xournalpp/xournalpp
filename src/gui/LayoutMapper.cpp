#include "gui/LayoutMapper.h"



LayoutMapper::LayoutMapper( int pages, int numRows, int numCols, bool useRows, LayoutType type, bool isPaired, int firstPageOffset)
{

	XOJ_INIT_TYPE(LayoutMapper);
	
	layoutMapperInit(  pages,  numRows, numCols,  useRows, type,  isPaired, firstPageOffset);
	
}


LayoutMapper::LayoutMapper( int pages,  int numRows, int numCols, bool useRows , bool isVertical, bool isRightToLeft, bool isBottomToTop, bool isPaired, int firstPageOffset)
{

	XOJ_INIT_TYPE(LayoutMapper);
	
	int type = isVertical?LayoutBitFlags::ColMajor:0;
	    type |=  isRightToLeft?LayoutBitFlags::RightToLeft:0;
		type |=  isBottomToTop?LayoutBitFlags::BottomToTop:0;
		
	
	layoutMapperInit(  pages,  numRows, numCols,  useRows,  (LayoutType)type,  isPaired,  firstPageOffset);
}



void LayoutMapper::layoutMapperInit( int pages, int numRows, int numCols, bool useRows , LayoutType type, bool isPaired, int firstPageOffset)
{

	XOJ_CHECK_TYPE(LayoutMapper);
	
	this->paired = isPaired;
	if(useRows)
	{
		this->rows = MAX(1, numRows);
		this->cols = MAX(1, (pages + firstPageOffset + (this->rows  -1) )/this->rows );	// using  + ( rows-1) to round up (int)pages/rows
	}
	else
	{
		this->cols = MAX(1, numCols);
		this->rows = MAX(1, (pages + firstPageOffset + (this->cols  -1) )/this->cols );	
	}

	if(isPaired)
	{
		this->cols += this->cols % 2;	 // add another column if pairing and odd number of columns.
	}

							 
	this->layoutType = type;
	
	if(type & LayoutBitFlags::ColMajor) 							// Vertical Layout
	{
		if( isPaired)
		{
			this->offset = firstPageOffset % (2*this->rows);
		}
		else
		{
			this->offset = firstPageOffset % this->rows;
		}
	}
	else																				// Horizontal Layout
	{
		this->offset = firstPageOffset % this->cols;
	}
	
	
	this->possiblePages = this->rows * this->cols;
	this->actualPages = pages;
}
	
	
int LayoutMapper::getColumns()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->cols;
}

int LayoutMapper::getRows()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->rows;
}

int LayoutMapper::getFirstPageOffset()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->offset;
}
	
	
	
int LayoutMapper::map(int x, int y)
{ 
	XOJ_CHECK_TYPE(LayoutMapper);

	int res;
	
	if( this->layoutType < BitFlagsUsedToHere)
	{
		if ( this->layoutType & LayoutBitFlags::RightToLeft)
		{
				x = this->cols - 1 - x;	 	// reverse x
		}
		if ( this->layoutType & LayoutBitFlags::BottomToTop)
		{
					y = this->rows - 1 - y; 	// reverse y 
		}		
				
		
		if(  this->layoutType & LayoutBitFlags::ColMajor) 	// aka Vertical
		{
			if( this->paired)
			{
				res = (( y + this->rows * (   (int)(x/2)   )  ) * 2 ) + x % 2;
			}
			else
			{
				res = y + this->rows * x;
			}
		}
		else //Horizontal
		{
				res = x + this->cols * y ;
		}

		
		res -= this->offset;
		
		if( res >= this->actualPages)
		{
			res = -1;
		}
								
		return res;
	}
	
	
}
