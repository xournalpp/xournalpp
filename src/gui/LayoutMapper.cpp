#include "gui/LayoutMapper.h"


LayoutMapper::LayoutMapper()
{
	XOJ_INIT_TYPE(LayoutMapper);
}

LayoutMapper::~LayoutMapper()
{
	XOJ_RELEASE_TYPE(LayoutMapper);
}


void LayoutMapper::configureFromSettings(size_t numPages, Settings* settings)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	auto showPairedPages = settings->isShowPairedPages();
	auto pairsOffset = showPairedPages ? settings->getPairsOffset() : 0;

	auto numCols = settings->isPresentationMode() ? 1 : settings->getViewColumns();
	auto numRows = settings->isPresentationMode() ? 1 : settings->getViewRows();
	auto fixRows = settings->isPresentationMode() ? false : settings->isViewFixedRows(); //i.e. use columns

	// get from user settings:
	auto isVertical = settings->getViewLayoutVert();
	auto isRightToLeft = settings->getViewLayoutR2L();
	auto isBottomToTop = settings->getViewLayoutB2T();

	if (settings->isPresentationMode())
	{

	};
	//assemble bitflags for LayoutType
	auto type = settings->isPresentationMode() ? Vertical :
	            ((isVertical ? Vertical : Horizontal) |
	             (isRightToLeft ? RightToLeft : LeftToRight) |
	             (isBottomToTop ? BottomToTop : TopToBottom));

	this->configure(numPages, numRows, numCols, fixRows, static_cast<LayoutType>(type), showPairedPages, pairsOffset);
}

void LayoutMapper::configure(int pages, int numRows, int numCols, bool useRows, LayoutType type, bool showPairedPages,
                             int firstPageOffset)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	this->showPairedPages = showPairedPages;
	if (useRows)
	{
		this->rows = MAX(1, numRows);

		// using  + ( rows-1) to round up (int)pages/rows
		this->cols = MAX(1, (pages + firstPageOffset + (this->rows - 1)) / this->rows);
		if (showPairedPages)
		{
			this->cols += this->cols % 2;    //make even
		}

	}
	else
	{
		this->cols = MAX(1, numCols);
		if (showPairedPages)
		{
			this->cols += this->cols % 2;    //make even
		}
		this->rows = MAX(1, (pages + firstPageOffset + (this->cols - 1)) / this->cols);
	}

	this->layoutType = type;

	if (type & Vertical)
	{
		// Vertical Layout
		if (showPairedPages)
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

bool LayoutMapper::getPairedPages()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->showPairedPages;
}

int LayoutMapper::map(int x, int y)
{
	XOJ_CHECK_TYPE(LayoutMapper);
	if (this->layoutType >= Size)
	{
		g_warning("LayoutMapper::Unknown layout: %d\n", this->layoutType);
		return 0;
	}

	int res;
	if (this->isRightToLeft())
	{
		// reverse x
		x = this->cols - 1 - x;
	}

	if (this->isBottomToTop())
	{
		// reverse y
		y = this->rows - 1 - y;
	}

	if (this->isVertical())
	{
		if (this->showPairedPages)
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

bool LayoutMapper::isVertical() const
{
	return (this->layoutType & Vertical) == Vertical;
}

bool LayoutMapper::isBottomToTop() const
{
	return (this->layoutType & BottomToTop) == BottomToTop;
}

bool LayoutMapper::isRightToLeft() const
{
	return (this->layoutType & RightToLeft) == RightToLeft;
}