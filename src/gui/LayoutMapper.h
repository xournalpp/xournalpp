/*
 * Xournal++
 *
 * A layout manager - map where( row,column) to which page( document index)
 *
 * @author Justin Jones
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

enum LayoutType {					//		1		2		4
	Horizontal = 0,							//									
	Vertical = 1,							//		V							
	Horizontal_RL = 2,				//				RL			
	Vertical_RL = 3,						//		V		RL		
	Horizontal_BT = 4,				//						BT		
	Vertical_BT = 5,						//		V				BT	
	Horizontal_RL_BT = 6,			//				RL		BT			
	Vertical_BT_RL = 7,				//		V		RL		BT		
	BitFlagsUsed,		//do not modify this or above

};

enum LayoutBitFlags{
		ColMajor = 1,		//aka Vertical
		RightToLeft = 2,
		BottomToTop = 4,
};

/** 
 * @brief Allow different orderings of document pages in layout.
 */
	
class LayoutMapper
{

public:
	int c=0;
	int r=0;
	int actualPages=0;
	int possiblePages=0;
	int offset = 0;
	LayoutType layoutType;
	bool paired = false;

	
	/**
	 * Initialize mapper of LayoutType with number of pages and of fixed rows or columns
	 * @param  pages  The number of pages in the document
	 * @param  rORc Number of rows OR columns depending on boolean flag isR
	 * @param  isR  How to interpret rORc: True is rows
	 * @param  off  Pages to offset - usually one or zero in order to pair up properly
	 * @param  type Specify LayoutType desired.  Options include: Horizontal, Vertical, VerticalPaired 
	 * 
	 * @return Page index to put at coordinates 
	 */	
	LayoutMapper( int pages, int rORc, bool isR , int off, LayoutType type, bool paired );
	
	
	
	
	/**
	 * Map page location to document index
	 * 
	 * @param  x Row we are interested in
	 * @param  y Column we are interested in
	 * 
	 * @return Page index to put at coordinates 
	 */
	int map(int x, int y);
		
};
