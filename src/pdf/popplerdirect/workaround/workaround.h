#pragma once

#include "conf.h"

#ifdef HAVE_POPPLER_CAIRO_OUTPUT_DEV
#include <poppler/CairoOutputDev.h>
#else
//#include "CairoOutputDev.h"
#include "../poppler-0.24.1/poppler/CairoOutputDev.h"
#endif
