//========================================================================
//
// ImageOutputDev.cc
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
// Copyright (C) 2005, 2007, 2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Rainer Keller <class321@gmx.de>
// Copyright (C) 2008 Timothy Lee <timothy.lee@siriushk.com>
// Copyright (C) 2008 Vasile Gaburici <gaburici@cs.umd.edu>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Jakob Voss <jakob.voss@gbv.de>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Fischer <fischer@unix-ag.uni-kl.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "Error.h"
#include "GfxState.h"
#include "Object.h"
#include "Stream.h"
#include "ImageOutputDev.h"

ImageOutputDev::ImageOutputDev(char *fileRootA, GBool pageNamesA, GBool dumpJPEGA, GBool listImagesA) {
  listImages = listImagesA;
  if (!listImages) {
    fileRoot = copyString(fileRootA);
    fileName = (char *)gmalloc(strlen(fileRoot) + 45);
  }
  dumpJPEG = dumpJPEGA;
  pageNames = pageNamesA;
  imgNum = 0;
  pageNum = 0;
  ok = gTrue;
  if (listImages) {
    printf("page   num  type   width height color comp bpc  enc interp  object ID\n");
    printf("---------------------------------------------------------------------\n");
  }
}


ImageOutputDev::~ImageOutputDev() {
  if (!listImages) {
    gfree(fileName);
    gfree(fileRoot);
  }
}

void ImageOutputDev::setFilename(const char *fileExt) {
  if (pageNames) {
    sprintf(fileName, "%s-%03d-%03d.%s", fileRoot, pageNum, imgNum, fileExt);
  } else {
    sprintf(fileName, "%s-%03d.%s", fileRoot, imgNum, fileExt);
  }
}

void ImageOutputDev::listImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate, GBool inlineImg,
			       ImageType imageType) {
  const char *type;
  const char *colorspace;
  const char *enc;
  int components, bpc;

  printf("%4d %5d ", pageNum, imgNum);
  type = "";
  switch (imageType) {
  case imgImage:
    type = "image";
    break;
  case imgStencil:
    type = "stencil";
    break;
  case imgMask:
    type = "mask";
    break;
  case imgSmask:
    type = "smask";
    break;
  }
  printf("%-7s %5d %5d  ", type, width, height);

  colorspace = "-";
  /* masks and stencils default to ncomps = 1 and bpc = 1 */
  components = 1;
  bpc = 1;
  if (colorMap && colorMap->isOk()) {
    switch (colorMap->getColorSpace()->getMode()) {
      case csDeviceGray:
      case csCalGray:
        colorspace = "gray";
        break;
      case csDeviceRGB:
      case csCalRGB:
        colorspace = "rgb";
        break;
      case csDeviceCMYK:
        colorspace = "cmyk";
        break;
      case csLab:
        colorspace = "lab";
        break;
      case csICCBased:
        colorspace = "icc";
        break;
      case csIndexed:
        colorspace = "index";
        break;
      case csSeparation:
        colorspace = "sep";
        break;
      case csDeviceN:
        colorspace = "devn";
        break;
      case csPattern:
      default:
        colorspace = "-";
        break;
    }
    components = colorMap->getNumPixelComps();
    bpc = colorMap->getBits();
  }
  printf("%-5s  %2d  %2d  ", colorspace, components, bpc);

  switch (str->getKind()) {
  case strCCITTFax:
    enc = "ccitt";
    break;
  case strDCT:
    enc = "jpeg";
    break;
  case strJPX:
    enc = "jpx";
    break;
  case strJBIG2:
    enc = "jbig2";
    break;
  case strFile:
  case strFlate:
  case strCachedFile:
  case strASCIIHex:
  case strASCII85:
  case strLZW:
  case strRunLength:
  case strWeird:
  default:
    enc = "image";
    break;
  }
  printf("%-5s  ", enc);

  printf("%-3s  ", interpolate ? "yes" : "no");

  if (inlineImg) {
    printf("[inline]\n");
  } else if (ref->isRef()) {
    const Ref imageRef = ref->getRef();
    if (imageRef.gen >= 100000) {
      printf("[none]\n");
    } else {
      printf(" %6d %2d\n", imageRef.num, imageRef.gen);
    }
  } else {
    printf("[none]\n");
  }

  ++imgNum;
}

void ImageOutputDev::writeMask(GfxState *state, Object *ref, Stream *str,
			       int width, int height, GBool invert,
			       GBool interpolate, GBool inlineImg) {
  FILE *f;
  int c;
  int size, i;

  // dump JPEG file
  if (dumpJPEG && str->getKind() == strDCT && !inlineImg) {

    // open the image file
    setFilename("jpg");
    ++imgNum;
    if (!(f = fopen(fileName, "wb"))) {
      error(errIO, -1, "Couldn't open image file '{0:s}'", fileName);
      return;
    }

    // initialize stream
    str = str->getNextStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f);

    str->close();
    fclose(f);

  // dump PBM file
  } else {

    // open the image file and write the PBM header
    setFilename("pbm");
    ++imgNum;
    if (!(f = fopen(fileName, "wb"))) {
      error(errIO, -1, "Couldn't open image file '{0:s}'", fileName);
      return;
    }
    fprintf(f, "P4\n");
    fprintf(f, "%d %d\n", width, height);

    // initialize stream
    str->reset();

    // copy the stream
    size = height * ((width + 7) / 8);
    for (i = 0; i < size; ++i) {
      fputc(str->getChar(), f);
    }

    str->close();
    fclose(f);
  }
}

void ImageOutputDev::writeImage(GfxState *state, Object *ref, Stream *str,
				int width, int height,
				GfxImageColorMap *colorMap,
				GBool interpolate, int *maskColors, GBool inlineImg) {
  FILE *f;
  ImageStream *imgStr;
  Guchar *p;
  Guchar zero = 0;
  GfxGray gray;
  GfxRGB rgb;
  int x, y;
  int c;
  int size, i;
  int pbm_mask = 0xff;

  // dump JPEG file
  if (dumpJPEG && str->getKind() == strDCT &&
      (colorMap->getNumPixelComps() == 1 ||
       colorMap->getNumPixelComps() == 3) &&
      !inlineImg) {

    // open the image file
    setFilename("jpg");
    ++imgNum;
    if (!(f = fopen(fileName, "wb"))) {
      error(errIO, -1, "Couldn't open image file '{0:s}'", fileName);
      return;
    }

    // initialize stream
    str = str->getNextStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF)
      fputc(c, f);

    str->close();
    fclose(f);

  // dump PBM file
  } else if (colorMap->getNumPixelComps() == 1 &&
	     colorMap->getBits() == 1) {

    // open the image file and write the PBM header
    setFilename("pbm");
    ++imgNum;
    if (!(f = fopen(fileName, "wb"))) {
      error(errIO, -1, "Couldn't open image file '{0:s}'", fileName);
      return;
    }
    fprintf(f, "P4\n");
    fprintf(f, "%d %d\n", width, height);

    // initialize stream
    str->reset();

    // if 0 comes out as 0 in the color map, the we _flip_ stream bits
    // otherwise we pass through stream bits unmolested
    colorMap->getGray(&zero, &gray);
    if(colToByte(gray))
      pbm_mask = 0;

    // copy the stream
    size = height * ((width + 7) / 8);
    for (i = 0; i < size; ++i) {
      fputc(str->getChar() ^ pbm_mask, f);
    }

    str->close();
    fclose(f);

  // dump PPM file
  } else {

    // open the image file and write the PPM header
    setFilename("ppm");
    ++imgNum;
    if (!(f = fopen(fileName, "wb"))) {
      error(errIO, -1, "Couldn't open image file '{0:s}'", fileName);
      return;
    }
    fprintf(f, "P6\n");
    fprintf(f, "%d %d\n", width, height);
    fprintf(f, "255\n");

    // initialize stream
    imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			     colorMap->getBits());
    imgStr->reset();

    // for each line...
    for (y = 0; y < height; ++y) {

      // write the line
      if ((p = imgStr->getLine())) {
	for (x = 0; x < width; ++x) {
	  colorMap->getRGB(p, &rgb);
	  fputc(colToByte(rgb.r), f);
	  fputc(colToByte(rgb.g), f);
	  fputc(colToByte(rgb.b), f);
	  p += colorMap->getNumPixelComps();
	}
      } else {
	for (x = 0; x < width; ++x) {
	  fputc(0, f);
	  fputc(0, f);
	  fputc(0, f);
	}
      }
    }
    imgStr->close();
    delete imgStr;

    fclose(f);
  }
}

GBool ImageOutputDev::tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *cat, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep) {
  return gTrue;
  // do nothing -- this avoids the potentially slow loop in Gfx.cc
}

void ImageOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				   int width, int height, GBool invert,
				   GBool interpolate, GBool inlineImg) {
  if (listImages)
    listImage(state, ref, str, width, height, NULL, interpolate, inlineImg, imgMask);
  else
    writeMask(state, ref, str, width, height, invert, interpolate, inlineImg);
}

void ImageOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate, int *maskColors, GBool inlineImg) {
  if (listImages)
    listImage(state, ref, str, width, height, colorMap, interpolate, inlineImg, imgImage);
  else
    writeImage(state, ref, str, width, height, colorMap, interpolate, maskColors, inlineImg);
}

void ImageOutputDev::drawMaskedImage(
  GfxState *state, Object *ref, Stream *str,
  int width, int height, GfxImageColorMap *colorMap, GBool interpolate,
  Stream *maskStr, int maskWidth, int maskHeight, GBool maskInvert, GBool maskInterpolate) {
  if (listImages) {
    listImage(state, ref, str, width, height, colorMap, interpolate, gFalse, imgImage);
    listImage(state, ref, str, maskWidth, maskHeight, NULL, maskInterpolate, gFalse, imgMask);
  } else {
    drawImage(state, ref, str, width, height, colorMap, interpolate, NULL, gFalse);
    drawImageMask(state, ref, maskStr, maskWidth, maskHeight, maskInvert,
		  maskInterpolate, gFalse);
  }
}

void ImageOutputDev::drawSoftMaskedImage(
  GfxState *state, Object *ref, Stream *str,
  int width, int height, GfxImageColorMap *colorMap, GBool interpolate,
  Stream *maskStr, int maskWidth, int maskHeight,
  GfxImageColorMap *maskColorMap, GBool maskInterpolate) {
  if (listImages) {
    listImage(state, ref, str, width, height, colorMap, interpolate, gFalse, imgImage);
    listImage(state, ref, maskStr, maskWidth, maskHeight, maskColorMap, maskInterpolate, gFalse, imgSmask);
  } else {
    drawImage(state, ref, str, width, height, colorMap, interpolate, NULL, gFalse);
    drawImage(state, ref, maskStr, maskWidth, maskHeight,
	      maskColorMap, maskInterpolate, NULL, gFalse);
  }
}
