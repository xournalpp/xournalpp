#include "gui/LayoutMapper.h"

#include <algorithm>


LayoutMapper::LayoutMapper(){
	
	XOJ_INIT_TYPE(LayoutMapper);
}

LayoutMapper::~LayoutMapper()
{
	XOJ_RELEASE_TYPE(LayoutMapper);
}

void LayoutMapper::configureFromSettings(size_t numPages, Settings* settings)
{
	XOJ_CHECK_TYPE(LayoutMapper);
	// get from user settings:
	this->actualPages = numPages;
	this->showPairedPages = settings->isShowPairedPages();
	const int pairsOffset = this->showPairedPages ? settings->getPairsOffset() : 0;

	const bool fixRows = settings->isPresentationMode() ? false : settings->isViewFixedRows();
	const size_t numCols = settings->isPresentationMode() ? 1 : settings->getViewColumns();
	const size_t numRows = settings->isPresentationMode() ? 1 : settings->getViewRows();

	//assemble bitflags for LayoutType
	this->orientation = (settings->isPresentationMode() || settings->getViewLayoutVert()) ? Vertical : Horizontal;
	this->horizontalDir = settings->getViewLayoutR2L() ? RightToLeft : LeftToRight;
	this->verticalDir = settings->getViewLayoutB2T() ? BottomToTop : TopToBottom;

	this->configure(numRows, numCols, fixRows, pairsOffset);
}

void LayoutMapper::configure(size_t numRows, size_t numCols, bool useRows, int firstPageOffset)
{
	XOJ_CHECK_TYPE(LayoutMapper);

	if (useRows)
	{
		this->rows = std::max(1ul, numRows);

		// using  + ( rows-1) to round up (int)pages/rows
		this->cols = std::max(1ul, (this->actualPages + firstPageOffset + (this->rows - 1)) / this->rows);
		if (showPairedPages)
		{
			this->cols += this->cols % 2;	//make even
		}

	}
	else
	{
		this->cols = std::max(1ul, numCols);
		if (showPairedPages)
		{
			this->cols += this->cols % 2;	//make even
		}
		this->rows = std::max(1ul, (this->actualPages + firstPageOffset + (this->cols - 1)) / this->cols);
	}


	if (this->isVertical())
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
}

size_t LayoutMapper::getColumns()
{
	XOJ_CHECK_TYPE(LayoutMapper);

	return this->cols;
}

size_t LayoutMapper::getRows()
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


// Todo: replace with map<pair(x,y)> -> index and vector<index> -> pair(x,y)
//       precalculate it in configure
// Todo: replace with
//       boost::optional<size_t> LayoutMapper::map(size_t x, size_t y) or
//       std::optional<size_t> LayoutMapper::map(size_t x, size_t y)
LayoutMapper::optional_size_t LayoutMapper::map(size_t x, size_t y)
{
	XOJ_CHECK_TYPE(LayoutMapper);
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

	size_t res;
	if (this->isVertical())
	{
		if (this->showPairedPages)
		{
			res = ((y + this->rows * (x / 2)) * 2) + x % 2;
		}
		else
		{
			res = y + this->rows * x;
		}
	}
	else  //Horizontal
	{
		res = x + this->cols * y;
	}

	res -= this->offset;

	if (res >= this->actualPages)
	{
		return {};
	}

	return res;
}

bool LayoutMapper::isVertical() const
{
	return this->orientation == Vertical;
}

bool LayoutMapper::isBottomToTop() const
{
	return this->verticalDir == BottomToTop;
}

bool LayoutMapper::isRightToLeft() const
{
	return this->horizontalDir == RightToLeft;
}