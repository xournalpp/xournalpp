#include "gui/LayoutMapper.h"


#define XYH( x,y,r,c,off)	(( c * y + x) - off%c)
#define XYV( x,y,r,c,off) (( r* x  + y) - off%r)
#define XYVP( x,y,r,c,off ) (((( y+ r* int(x/2))*2 + x%2 ) - off % 2)


#define  MAX(A, B)  ((A) > (B) ? (A) : (B))


LayoutMapper::LayoutMapper( int pages, int rORc, bool isR , int off, LayoutType type, bool isPaired){

	paired = isPaired;
	if(isR){
		r = MAX(1, rORc);
		c = MAX(1, (pages + off + (rORc -1) )/rORc);	// using  + ( rORc-1) to round up (int)pages/rows
	}else{
		c = MAX(1, rORc);
		r = MAX(1, (pages + off + (rORc -1) )/rORc);	
	}

	if(paired) c += c%2;	 // add another column if pairing and odd number of columns.

							 
	layoutType = type;
	
	if(type & LayoutBitFlags::ColMajor)
	{
		if( paired){
			offset = off % (2*r);
		}
		else{
			offset = off % r;
		}
	}
	else //Horizontal Layout
	{
		offset = off % c;
	}
	
	
	possiblePages = r * c;
	actualPages = pages;
}
	
	
int LayoutMapper::map(int x, int y){ 
	int res;
	
	if( layoutType < BitFlagsUsed)
	{
		if ( layoutType & LayoutBitFlags::RightToLeft)
		{
				x = c-1-x;	//mirror x
		}
		if ( layoutType & LayoutBitFlags::BottomToTop)
		{
					y = r-1-y;
		}		
				
		
		if(  layoutType & LayoutBitFlags::ColMajor) 	// aka Vertical
		{
			if( paired)
			{
				res = ((( y + r*( (int)(x/2) )) * 2 ) + x %2);
			}
			else{
				res = (y + r* x);
			}
		}
		else //Horizontal
		{
				res = (x + c * y);
		}

		
		res -= offset;
		if( res>=actualPages) res = -1;
								
		return res;
	}
	
	
}
