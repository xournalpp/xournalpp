#include "PositionInputData.h"

auto PositionInputData::isShiftDown() const -> bool { return state & GDK_SHIFT_MASK; }

auto PositionInputData::isControlDown() const -> bool { return state & GDK_CONTROL_MASK; }

auto PositionInputData::isAltDown() const -> bool { return state & GDK_ALT_MASK; }
