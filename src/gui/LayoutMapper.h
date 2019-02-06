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

enum LayoutType {
  Horizontal,
  Vertical,
  VerticalPaired,
  //TODO: add in Right to Left mappings for other cultures (i.e. Japanese )
};

/** 
 * @brief Allow different orderings of document pages in layout.
 */
	
class LayoutMapper
{

public:
	int c=0;
	int r=0;
	int rORc;
	int actualPages=0;
	int possiblePages=0;
	int offset = 0;
	LayoutType layoutType;

	
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
	LayoutMapper( int pages, int rORc, bool isR , int off, LayoutType type );
	
	
	
	
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
