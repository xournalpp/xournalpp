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

enum LayoutType
{								//		1		2		4
	Horizontal 			= 0,	//
	Vertical 			= 1,	//		V
	Horizontal_RL 		= 2,	//				RL
	Vertical_RL 		= 3,	//		V		RL
	Horizontal_BT 		= 4,	//						BT
	Vertical_BT 		= 5,	//		V				BT
	Horizontal_RL_BT 	= 6,	//				RL		BT
	Vertical_BT_RL 		= 7,	//		V		RL		BT
	BitFlagsUsedToHere 	= 8,	//	do not modify this or above
};

enum LayoutBitFlags
{
	Vertically  = 1,
	RightToLeft = 2,
	BottomToTop = 4,
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

	
	void configureFromSettings(int numPages, Settings* settings);
	
	
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
	int getPairedPages();

private:
	XOJ_TYPE_ATTRIB;

	int cols = 0;
	int rows = 0;
	int actualPages = 0;
	int possiblePages = 0;
	int offset = 0;
	LayoutType layoutType = Horizontal;
	bool isPairedPages = false;
};

