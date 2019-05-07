#include "PositionInputData.h"

bool PositionInputData::isShiftDown() const
{
	return state & GDK_SHIFT_MASK;
}
 
bool PositionInputData::isControlDown() const
{
	return state & GDK_CONTROL_MASK;
}

bool PositionInputData::isAltDown() const
{
	return state & GDK_MOD1_MASK;
}
