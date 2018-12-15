#include "PositionInputData.h"

bool PositionInputData::isShiftDown() const
{
	return state & GDK_SHIFT_MASK;
}
 
