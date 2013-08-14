//========================================================================
//
// SplashScreen.h
//
//========================================================================

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

//------------------------------------------------------------------------
// SplashScreen
//------------------------------------------------------------------------

class SplashScreen {
public:

  SplashScreen(SplashScreenParams *params);
  SplashScreen(SplashScreen *screen);
  ~SplashScreen();

  SplashScreen *copy() { return new SplashScreen(this); }

  // Return the computed pixel value (0=black, 1=white) for the gray
  // level <value> at (<x>, <y>).
  int test(int x, int y, Guchar value);

  // Returns true if value is above the white threshold or below the
  // black threshold, i.e., if the corresponding halftone will be
  // solid white or black.
  GBool isStatic(Guchar value);

private:
  void buildDispersedMatrix(int i, int j, int val,
			    int delta, int offset);
  void buildClusteredMatrix();
  int distance(int x0, int y0, int x1, int y1);
  void buildSCDMatrix(int r);

  Guchar *mat;			// threshold matrix
  int size;			// size of the threshold matrix
  Guchar minVal;		// any pixel value below minVal generates
				//   solid black
  Guchar maxVal;		// any pixel value above maxVal generates
				//   solid white
};

#endif
