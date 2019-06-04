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

#include <XournalType.h>
#include "control/settings/Settings.h"

enum LayoutType : unsigned
{                             // V RTL BTT
	Horizontal = 0,           // 0 0   0
	Vertical = 1,             // 1 0   0
	RightToLeft = 2,          // 0 1   0
	// Vertical_RL = 3,       // 1 1   0
	BottomToTop = 4,          // 0 0   1
	// Vertical_BT = 5,       // 1 0   1
	// Horizontal_RL_BT = 6,  // 0 1   1
	// Vertical_BT_RL = 7,    // 1 1   1
	Size = 8                  // EnumSize
};

/** 
 * @brief Layout asks this mapper what page ( if any ) should be at a given column,row.
 */
class LayoutMapper
{
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

	[[deprecated ("use LayoutMapper::configureFromSettings")]]
	void configureForPresentation(int numPages, Settings* settings);

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
	void configure(int pages, int numRows, int numCols, bool useRows, LayoutType type, bool paired,
	               int firstPageOffset);

public:
	/**
	 * Map page location to document index
	 * 
	 * @param  x Row we are interested in
	 * @param  y Column we are interested in
	 * 
	 * @return Page index to put at coordinates 
	 */
	int map(int x, int y);

	/**
	 * Get number of columns
	 * 
	 * @return number of columns
	 */
	int getColumns();

	/**
	 * Get number of rows
	 * 
	 * @return number of rows
	 */
	int getRows();

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

	bool isRightToLeft();

private:
	XOJ_TYPE_ATTRIB;

	int cols = 0;
	int rows = 0;
	int actualPages = 0;
	int possiblePages = 0;
	int offset = 0;
	LayoutType layoutType = Horizontal;
	bool isPairedPages = false;

	bool isBottomToTop() const;

	bool isVertical() const;
};

