#pragma once

#include "config-debug.h"

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
#define IF_DEBUG_REPAINT(f) f
#else
#define IF_DEBUG_REPAINT(f)
#endif
