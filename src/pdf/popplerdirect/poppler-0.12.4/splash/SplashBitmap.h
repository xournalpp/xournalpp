//========================================================================
//
// SplashBitmap.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHBITMAP_H
#define SPLASHBITMAP_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"
#include <stdio.h>

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

class SplashBitmap {
public:

  // Create a new bitmap.  It will have <widthA> x <heightA> pixels in
  // color mode <modeA>.  Rows will be padded out to a multiple of
  // <rowPad> bytes.  If <topDown> is false, the bitmap will be stored
  // upside-down, i.e., with the last row first in memory.
  SplashBitmap(int widthA, int heightA, int rowPad,
	       SplashColorMode modeA, GBool alphaA,
	       GBool topDown = gTrue);

  ~SplashBitmap();

  int getWidth() { return width; }
  int getHeight() { return height; }
  int getRowSize() { return rowSize; }
  int getAlphaRowSize() { return width; }
  SplashColorMode getMode() { return mode; }
  SplashColorPtr getDataPtr() { return data; }
  Guchar *getAlphaPtr() { return alpha; }

  SplashError writePNMFile(char *fileName);
  SplashError writePNMFile(FILE *f);
  
  SplashError writePNGFile(char *fileName);
  SplashError writePNGFile(FILE *f);

  void getPixel(int x, int y, SplashColorPtr pixel);
  Guchar getAlpha(int x, int y);

private:

  int width, height;		// size of bitmap
  int rowSize;			// size of one row of data, in bytes
				//   - negative for bottom-up bitmaps
  SplashColorMode mode;		// color mode
  SplashColorPtr data;		// pointer to row zero of the color data
  Guchar *alpha;		// pointer to row zero of the alpha data
				//   (always top-down)

  friend class Splash;
};

#endif
