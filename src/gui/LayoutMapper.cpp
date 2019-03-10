#include "gui/LayoutMapper.h"


LayoutMapper::LayoutMapper(){
	
	XOJ_INIT_TYPE(LayoutMapper);
	
	this->actualPages = 0;
	this->possiblePages = 0;
	this->rows = 0;
	this->cols = 0;
	this->layoutType = Horizontal;
	
}


void LayoutMapper::configureFromSettings(int numPages, Settings* settings)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	int pages = numPages;

	// get from user settings:
	bool isPairedPages = settings->isShowPairedPages();
	int numCols = settings->getViewColumns();
	int numRows = settings->getViewRows();
	bool fixRows = settings->isViewFixedRows();
	int pairsOffset = settings->getPairsOffset();
	bool isVertical = settings->getViewLayoutVert();
	bool isRightToLeft = settings->getViewLayoutR2L();
	bool isBottomToTop = settings->getViewLayoutB2T();

	if (!isPairedPages)
	{
		pairsOffset = 0;
	}

	//assemble bitflags for LayoutType
	int type = isVertical ? LayoutBitFlags::Vertically : 0;
	type |= isRightToLeft ? LayoutBitFlags::RightToLeft : 0;
	type |= isBottomToTop ? LayoutBitFlags::BottomToTop : 0;

	
	layoutMapperInit(pages, numRows, numCols, fixRows, (LayoutType)type, isPairedPages, pairsOffset);
}

LayoutMapper::~LayoutMapper()
{
	XOJ_RELEASE_TYPE(LayoutMapper);
}

void LayoutMapper::layoutMapperInit(int pages, int numRows, int numCols, bool useRows, LayoutType type, bool isPaired,
		int firstPageOffset)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	this->isPairedPages = isPaired;
	if (useRows)
	{
		this->rows = MAX(1, numRows);

		// using  + ( rows-1) to round up (int)pages/rows
		this->cols = MAX(1, (pages + firstPageOffset + (this->rows - 1)) / this->rows);
	}
	else
	{
		this->cols = MAX(1, numCols);
		this->rows = MAX(1, (pages + firstPageOffset + (this->cols - 1)) / this->cols);
	}

	if (isPaired)
	{
		// add another column if pairing and odd number of columns.
		this->cols += this->cols % 2;
	}

	this->layoutType = type;

	if (type & LayoutBitFlags::Vertically)
	{
		// Vertical Layout
		if (isPaired)
		{
			this->offset = firstPageOffset % (2 * this->rows);
		}
		else
		{
			this->offset = firstPageOffset % this->rows;
		}
	}
	else
	{
		// Horizontal Layout
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

int LayoutMapper::getPairedPages()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->isPairedPages;
}

int LayoutMapper::map(int x, int y)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	if (this->layoutType < BitFlagsUsedToHere)
	{
		int res;
		if (this->layoutType & LayoutBitFlags::RightToLeft)
		{
			// reverse x
			x = this->cols - 1 - x;
		}

		if (this->layoutType & LayoutBitFlags::BottomToTop)
		{
			// reverse y
			y = this->rows - 1 - y;
		}

		if (this->layoutType & LayoutBitFlags::Vertically)
		{
			if (this->isPairedPages)
			{
				res = ((y + this->rows * ((int) (x / 2))) * 2) + x % 2;
			}
			else
			{
				res = y + this->rows * x;
			}
		}
		else //Horizontal
		{
			res = x + this->cols * y;
		}

		res -= this->offset;

		if (res >= this->actualPages)
		{
			res = -1;
		}

		return res;
	}

	g_warning("LayoutMapper::Unknown layout: %d\n", this->layoutType);
	return 0;
}
