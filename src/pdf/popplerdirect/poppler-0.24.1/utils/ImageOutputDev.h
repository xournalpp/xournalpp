//========================================================================
//
// ImageOutputDev.h
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Rainer Keller <class321@gmx.de>
// Copyright (C) 2008 Timothy Lee <timothy.lee@siriushk.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Jakob Voss <jakob.voss@gbv.de>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef IMAGEOUTPUTDEV_H
#define IMAGEOUTPUTDEV_H

#include "poppler/poppler-config.h"

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "goo/gtypes.h"
#include "OutputDev.h"

class GfxState;

//------------------------------------------------------------------------
// ImageOutputDev
//------------------------------------------------------------------------

class ImageOutputDev: public OutputDev {
public:
  enum ImageType {
    imgImage,
    imgStencil,
    imgMask,
    imgSmask
  };

  // Create an OutputDev which will write images to files named
  // <fileRoot>-NNN.<type> or <fileRoot>-PPP-NNN.<type>, if 
  // <pageNames> is set. Normally, all images are written as PBM
  // (.pbm) or PPM (.ppm) files.  If <dumpJPEG> is set, JPEG images 
  // are written as JPEG (.jpg) files.
  ImageOutputDev(char *fileRootA, GBool pageNamesA, GBool dumpJPEGA, GBool listImagesA);

  // Destructor.
  virtual ~ImageOutputDev();

  // Check if file was successfully created.
  virtual GBool isOk() { return ok; }

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need non-text content?
  virtual GBool needNonText() { return gTrue; }

  // Start a page
  virtual void startPage(int pageNumA, GfxState *state, XRef *xref) 
			{ pageNum = pageNumA; }
 
  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  //----- path painting
  virtual GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   GBool interpolate,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap,
				   GBool maskInterpolate);

private:
  // Sets the output filename with a given file extension
  void setFilename(const char *fileExt);
  void listImage(GfxState *state, Object *ref, Stream *str,
		 int width, int height,
		 GfxImageColorMap *colorMap,
		 GBool interpolate, GBool inlineImg,
		 ImageType imageType);
  void writeMask(GfxState *state, Object *ref, Stream *str,
		 int width, int height, GBool invert,
		 GBool interpolate, GBool inlineImg);
  void writeImage(GfxState *state, Object *ref, Stream *str,
                  int width, int height, GfxImageColorMap *colorMap,
                  GBool interpolate, int *maskColors, GBool inlineImg);


  char *fileRoot;		// root of output file names
  char *fileName;		// buffer for output file names
  GBool listImages;		// list images instead of dumping
  GBool dumpJPEG;		// set to dump native JPEG files
  GBool pageNames;		// set to include page number in file names
  int pageNum;			// current page number
  int imgNum;			// current image number
  GBool ok;			// set up ok?
};

#endif
