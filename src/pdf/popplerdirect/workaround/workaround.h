#ifndef __WORKAROUND_H__
#define __WORKAROUND_H__

#include "conf.h"

#ifdef HAVE_POPPLER_CAIRO_OUTPUT_DEV
#include <poppler/CairoOutputDev.h>
#else
#include "CairoOutputDev.h"
#endif

#endif // __WORKAROUND_H__
