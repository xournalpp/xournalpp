#include "gui/LayoutMapper.h"


#define XYH( x,y,r,c,off)	(( c * y + x) - off%c)
#define XYV( x,y,r,c,off) (( r* x  + y) - off%r)
#define XYVP( x,y,r,c,off ) (((( y+ r* int(x/2))*2 + x%2 ) - off % 2)


#define  MAX(A, B)  ((A) > (B) ? (A) : (B))


LayoutMapper::LayoutMapper( int pages, int rORc, bool isR , int off, LayoutType type ){

	if(isR){
		r = MAX(1, rORc);
		c = MAX(1, (pages + off + (rORc -1) )/rORc);	// using  + ( rORc-1) to round up (int)pages/rows
	}else{
		c = MAX(1, rORc);
		r = MAX(1, (pages + off + (rORc -1) )/rORc);	
	}
	
	layoutType = type;
	switch ( type){
		case Horizontal:
			offset = off % c;
			break;
		case Vertical:	//vertical layout
			offset = off % r;
			break;
		case VerticalPaired:
			offset = off % (2*r);
			break;	
	}
	possiblePages = r * c;
	rORc = rORc;
	actualPages = pages;
}
	
	
int LayoutMapper::map(int x, int y){ 
	int res;
	switch( layoutType)
	{
		case Horizontal:
			res = (x + c * y);
			break;
			
		case Vertical:	//vertical layout
			res = (y + r* x);
			break;
			
		case VerticalPaired:
			res = ((( y + r*( (int)(x/2) )) * 2 ) + x %2);
			break;
	}
	res -= offset;
	if( res>=actualPages) res = -1;
							 
	return res;
}
