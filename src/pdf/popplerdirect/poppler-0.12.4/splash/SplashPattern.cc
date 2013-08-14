//========================================================================
//
// SplashPattern.cc
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "SplashMath.h"
#include "SplashScreen.h"
#include "SplashPattern.h"

//------------------------------------------------------------------------
// SplashPattern
//------------------------------------------------------------------------

SplashPattern::SplashPattern() {
}

SplashPattern::~SplashPattern() {
}

//------------------------------------------------------------------------
// SplashSolidColor
//------------------------------------------------------------------------

SplashSolidColor::SplashSolidColor(SplashColorPtr colorA) {
  splashColorCopy(color, colorA);
}

SplashSolidColor::~SplashSolidColor() {
}

void SplashSolidColor::getColor(int x, int y, SplashColorPtr c) {
  splashColorCopy(c, color);
}
