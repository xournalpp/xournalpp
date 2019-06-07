/*
 * Xournal++
 *
 * A layout manager - map where( row,column) to which page( document index)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XournalType.h"
#include "control/settings/Settings.h"

#include <boost/optional.hpp>

/** 
 * @brief Layout asks this mapper what page ( if any ) should be at a given column,row.
 */
class LayoutMapper
{
	/**
	 * @brief The Layout of the pages
	 */
	enum Orientation : bool
	{
		Horizontal = false,
		Vertical = true,
	};

	/**
	 * Horizontal read direction
	 */
	enum HorizontalDirection : bool
	{
		LeftToRight = false,
		RightToLeft = true,
	};

	/**
	 * Vertical read direction
	 */
	enum VerticalDirection : bool
	{
		TopToBottom = false,
		BottomToTop = true,
	};

public:
	/**
	 * LayoutMapper
	 * 
	 * Create a bare mapper to be configured before use.
	 */
	LayoutMapper();


	/**
	 * configureFromSettings
	 * Obtain user settings to determine arguments to configure().
	 * 
	 * @param  pages  The number of pages in the document
	 * @param  settings  The Settings from which users settings are obtained
	 */


	void configureFromSettings(size_t numPages, Settings* settings);

	virtual ~LayoutMapper();

private:
	/**
	 * configure
	 * 
	 * Set mapper to LayoutType with number of pages and of fixed rows or columns
	 * @param  pages  The number of pages in the document
	 * @param  numRows Number of rows ( used if useRows )
	 * @param  numCols  Number of columns ( used if !useRows )
	 * @param  useRows  use pages/rows to recalculate cols else recalculate rows
	 * @param  type Specify LayoutType desired.  Options include: Horizontal, Vertical, 
	 * @param  isPaired Display pages in pairs including offset 
	 * @param  firstPageOffset  Pages to offset - usually one or zero in order to pair up properly
	 */
	void configure(size_t numRows, size_t numCols, bool useRows, int firstPageOffset);

public:
	/**
	 * Map page location to document index
	 * 
	 * @param  x Row we are interested in
	 * @param  y Column we are interested in
	 * 
	 * @return Page index to put at coordinates 
	 */
	boost::optional<size_t> map(size_t x, size_t y);

	/**
	 * Get number of columns
	 * 
	 * @return number of columns
	 */
	size_t getColumns();

	/**
	 * Get number of rows
	 * 
	 * @return number of rows
	 */
	size_t getRows();

	/**
	 * Get offset
	 * 
	 * @return first page offset
	 */
	int getFirstPageOffset();

	/**
	 * Get PairedPages
	 * 
	 * @return isPairedPages
	 */
	bool getPairedPages();

	bool isRightToLeft() const;
	bool isBottomToTop() const;
	bool isVertical() const;

private:
	XOJ_TYPE_ATTRIB;

	size_t cols = 0;
	size_t rows = 0;
	size_t actualPages = 0;

	int offset = 0;

	bool showPairedPages = false;
	Orientation orientation = Vertical;
	HorizontalDirection horizontalDir = LeftToRight;
	VerticalDirection verticalDir = TopToBottom;
};

