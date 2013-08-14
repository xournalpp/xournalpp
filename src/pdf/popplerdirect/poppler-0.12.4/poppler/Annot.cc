//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2007, 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007-2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "goo/gmem.h"
#include "goo/gstrtod.h"
#include "GooList.h"
#include "Error.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "Lexer.h"
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "PDFDocEncoding.h"
#include "Form.h"
#include "Error.h"
#include "Page.h"
#include "XRef.h"
#include "Movie.h"
#include "OptionalContent.h"
#include "Sound.h"
#include "FileSpec.h"
#include "DateInfo.h"
#include <string.h>

#define annotFlagHidden    0x0002
#define annotFlagPrint     0x0004
#define annotFlagNoView    0x0020

#define fieldFlagReadOnly           0x00000001
#define fieldFlagRequired           0x00000002
#define fieldFlagNoExport           0x00000004
#define fieldFlagMultiline          0x00001000
#define fieldFlagPassword           0x00002000
#define fieldFlagNoToggleToOff      0x00004000
#define fieldFlagRadio              0x00008000
#define fieldFlagPushbutton         0x00010000
#define fieldFlagCombo              0x00020000
#define fieldFlagEdit               0x00040000
#define fieldFlagSort               0x00080000
#define fieldFlagFileSelect         0x00100000
#define fieldFlagMultiSelect        0x00200000
#define fieldFlagDoNotSpellCheck    0x00400000
#define fieldFlagDoNotScroll        0x00800000
#define fieldFlagComb               0x01000000
#define fieldFlagRichText           0x02000000
#define fieldFlagRadiosInUnison     0x02000000
#define fieldFlagCommitOnSelChange  0x04000000

#define fieldQuadLeft   0
#define fieldQuadCenter 1
#define fieldQuadRight  2

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle 0.55228475

AnnotLineEndingStyle parseAnnotLineEndingStyle(GooString *string) {
  if (string != NULL) {
    if (!string->cmp("Square")) {
      return annotLineEndingSquare;
    } else if (!string->cmp("Circle")) {
      return annotLineEndingCircle;
    } else if (!string->cmp("Diamond")) {
      return annotLineEndingDiamond;
    } else if (!string->cmp("OpenArrow")) {
      return annotLineEndingOpenArrow;
    } else if (!string->cmp("ClosedArrow")) {
      return annotLineEndingClosedArrow;
    } else if (!string->cmp("Butt")) {
      return annotLineEndingButt;
    } else if (!string->cmp("ROpenArrow")) {
      return annotLineEndingROpenArrow;
    } else if (!string->cmp("RClosedArrow")) {
      return annotLineEndingRClosedArrow;
    } else if (!string->cmp("Slash")) {
      return annotLineEndingSlash;
    } else {
      return annotLineEndingNone;
    }
  } else {
    return annotLineEndingNone;
  }  
}

static AnnotExternalDataType parseAnnotExternalData(Dict* dict) {
  Object obj1;
  AnnotExternalDataType type;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Markup3D")) {
      type = annotExternalDataMarkup3D;
    } else {
      type = annotExternalDataMarkupUnknown;
    }
    delete typeName;
  } else {
    type = annotExternalDataMarkupUnknown;
  }
  obj1.free();

  return type;
}

PDFRectangle *parseDiffRectangle(Array *array, PDFRectangle *rect) {
  PDFRectangle *newRect = NULL;
  if (array->getLength() == 4) {
    // deltas
    Object obj1;
    double dx1 = (array->get(0, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dy1 = (array->get(1, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dx2 = (array->get(2, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dy2 = (array->get(3, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();

    // checking that the numbers are valid (i.e. >= 0),
    // and that applying the differences still give us a valid rect
    if (dx1 >= 0 && dy1 >= 0 && dx2 >= 0 && dy2
        && (rect->x2 - rect->x1 - dx1 - dx2) >= 0
        && (rect->y2 - rect->y1 - dy1 - dy2) >= 0) {
      newRect = new PDFRectangle();
      newRect->x1 = rect->x1 + dx1;
      newRect->y1 = rect->y1 + dy1;
      newRect->x2 = rect->x2 - dx2;
      newRect->y2 = rect->y2 - dy2;
    }
  }
  return newRect;
}

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

AnnotBorderEffect::AnnotBorderEffect(Dict *dict) {
  Object obj1;

  if (dict->lookup("S", &obj1)->isName()) {
    GooString *effectName = new GooString(obj1.getName());

    if (!effectName->cmp("C"))
      effectType = borderEffectCloudy;
    else
      effectType = borderEffectNoEffect;
    delete effectName;
  } else {
    effectType = borderEffectNoEffect;
  }
  obj1.free();

  if ((dict->lookup("I", &obj1)->isNum()) && effectType == borderEffectCloudy) {
    intensity = obj1.getNum();
  } else {
    intensity = 0;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotPath
//------------------------------------------------------------------------

AnnotPath::AnnotPath() {
  coords = NULL;
  coordsLength = 0;
}

AnnotPath::AnnotPath(Array *array) {
  coords = NULL;
  coordsLength = 0;
  parsePathArray(array);
}

AnnotPath::AnnotPath(AnnotCoord **coords, int coordsLength) {
  this->coords = coords;
  this->coordsLength = coordsLength;
}

AnnotPath::~AnnotPath() {
  if (coords) {
    for (int i = 0; i < coordsLength; ++i)
      delete coords[i];
    gfree(coords);
  }
}

double AnnotPath::getX(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord]->getX();
  return 0;
}

double AnnotPath::getY(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord]->getY();
  return 0;
}

AnnotCoord *AnnotPath::getCoord(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord];
  return NULL;
}

void AnnotPath::parsePathArray(Array *array) {
  int tempLength;
  AnnotCoord **tempCoords;
  GBool correct = gTrue;

  if (array->getLength() % 2) {
    error(-1, "Bad Annot Path");
    return;
  }

  tempLength = array->getLength() / 2;
  tempCoords = (AnnotCoord **) gmallocn (tempLength, sizeof(AnnotCoord *));
  memset(tempCoords, 0, tempLength * sizeof(AnnotCoord *));
  for (int i = 0; i < tempLength && correct; i++) {
    Object obj1;
    double x = 0, y = 0;

    if (array->get(i * 2, &obj1)->isNum()) {
      x = obj1.getNum();
    } else {
      correct = gFalse;
    }
    obj1.free();

    if (array->get((i * 2) + 1, &obj1)->isNum()) {
      y = obj1.getNum();
    } else {
      correct = gFalse;
    }
    obj1.free();

    if (!correct) {
      for (int j = i - 1; j >= 0; j--)
        delete tempCoords[j];
      gfree (tempCoords);
      return;
    }

    tempCoords[i] = new AnnotCoord(x, y);
  }

  coords = tempCoords;
  coordsLength = tempLength;
}

//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

AnnotCalloutLine::AnnotCalloutLine(double x1, double y1, double x2, double y2)
    :  coord1(x1, y1), coord2(x2, y2) {
}

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

AnnotCalloutMultiLine::AnnotCalloutMultiLine(double x1, double y1, double x2,
                                             double y2, double x3, double y3)
    : AnnotCalloutLine(x1, y1, x2, y2), coord3(x3, y3) {
}

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

AnnotQuadrilaterals::AnnotQuadrilaterals(Array *array, PDFRectangle *rect) {
  int arrayLength = array->getLength();
  GBool correct = gTrue;
  int quadsLength = 0;
  AnnotQuadrilateral **quads;
  double quadArray[8];

  // default values
  quadrilaterals = NULL;
  quadrilateralsLength = 0;

  if ((arrayLength % 8) == 0) {
    int i = 0;

    quadsLength = arrayLength / 8;
    quads = (AnnotQuadrilateral **) gmallocn
        ((quadsLength), sizeof(AnnotQuadrilateral *));
    memset(quads, 0, quadsLength * sizeof(AnnotQuadrilateral *));

    while (i < (quadsLength) && correct) {
      for (int j = 0; j < 8 && correct; j++) {
        Object obj;
        if (array->get(i * 8 + j, &obj)->isNum()) {
          quadArray[j] = obj.getNum();
          if (j % 2 == 1) {
              if (quadArray[j] < rect->y1 || quadArray[j] > rect->y2)
                  correct = gFalse;
          } else {
              if (quadArray[j] < rect->x1 || quadArray[j] > rect->x2)
                  correct = gFalse;
          }
        } else {
            correct = gFalse;
        }
        obj.free();
      }

      if (correct)
        quads[i] = new AnnotQuadrilateral(quadArray[0], quadArray[1],
                                          quadArray[2], quadArray[3],
                                          quadArray[4], quadArray[5],
                                          quadArray[6], quadArray[7]);
      i++;
    }

    if (correct) {
      quadrilateralsLength = quadsLength;
      quadrilaterals = quads;
    } else {
      for (int j = 0; j < i; j++)
        delete quads[j];
      gfree (quads);
    }
  }
}

AnnotQuadrilaterals::~AnnotQuadrilaterals() {
  if (quadrilaterals) {
    for(int i = 0; i < quadrilateralsLength; i++)
      delete quadrilaterals[i];

    gfree (quadrilaterals);
  }
}

double AnnotQuadrilaterals::getX1(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord1.getX();
  return 0;
}

double AnnotQuadrilaterals::getY1(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord1.getY();
  return 0;
}

double AnnotQuadrilaterals::getX2(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord2.getX();
  return 0;
}

double AnnotQuadrilaterals::getY2(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord2.getY();
  return 0;
}

double AnnotQuadrilaterals::getX3(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord3.getX();
  return 0;
}

double AnnotQuadrilaterals::getY3(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord3.getY();
  return 0;
}

double AnnotQuadrilaterals::getX4(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord4.getX();
  return 0;
}

double AnnotQuadrilaterals::getY4(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord4.getY();
  return 0;
}

AnnotQuadrilaterals::AnnotQuadrilateral::AnnotQuadrilateral(double x1, double y1,
    double x2, double y2, double x3, double y3, double x4, double y4)
    : coord1(x1, y1), coord2(x2, y2), coord3(x3, y3), coord4(x4, y4) {
}

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------
AnnotBorder::AnnotBorder() {
  type = typeUnknown;
  width = 1;
  dashLength = 0;
  dash = NULL;
  style = borderSolid;
}

AnnotBorder::~AnnotBorder() {
  if (dash)
    gfree (dash); 
}
  
//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

AnnotBorderArray::AnnotBorderArray() {
  type = typeArray;
  horizontalCorner = 0;
  verticalCorner = 0;
}

AnnotBorderArray::AnnotBorderArray(Array *array) {
  Object obj1;
  int arrayLength = array->getLength();

  GBool correct = gTrue;
  if (arrayLength == 3 || arrayLength == 4) {
    // implementation note 81 in Appendix H.

    if (array->get(0, &obj1)->isNum())
      horizontalCorner = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    if (array->get(1, &obj1)->isNum())
      verticalCorner = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    if (array->get(2, &obj1)->isNum())
      width = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    // TODO: check not all zero ? (Line Dash Pattern Page 217 PDF 8.1)
    if (arrayLength == 4) {
      if (array->get(3, &obj1)->isArray()) {
        Array *dashPattern = obj1.getArray();
        int tempLength = dashPattern->getLength();
        double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

        for(int i = 0; i < tempLength && i < DASH_LIMIT && correct; i++) {

          if (dashPattern->get(i, &obj1)->isNum()) {
            tempDash[i] = obj1.getNum();

            if (tempDash[i] < 0)
              correct = gFalse;

          } else {
            correct = gFalse;
          }
          obj1.free();
        }

        if (correct) {
          dashLength = tempLength;
          dash = tempDash;
          style = borderDashed;
        } else {
          gfree (tempDash);
        }
      } else {
        correct = gFalse;
      }
      obj1.free();
    }
  } else {
    correct = gFalse;
  }
  
  if (!correct) {
    width = 0;
  }
}

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

AnnotBorderBS::AnnotBorderBS() {
  type = typeBS;
}

AnnotBorderBS::AnnotBorderBS(Dict *dict) {
  Object obj1, obj2;

  // acroread 8 seems to need both W and S entries for
  // any border to be drawn, even though the spec
  // doesn't claim anything of that sort. We follow
  // that behaviour by veryifying both entries exist
  // otherwise we set the borderWidth to 0
  // --jrmuizel
  dict->lookup("W", &obj1);
  dict->lookup("S", &obj2);
  if (obj1.isNum() && obj2.isName()) {
    GooString *styleName = new GooString(obj2.getName());

    width = obj1.getNum();

    if (!styleName->cmp("S")) {
      style = borderSolid;
    } else if (!styleName->cmp("D")) {
      style = borderDashed;
    } else if (!styleName->cmp("B")) {
      style = borderBeveled;
    } else if (!styleName->cmp("I")) {
      style = borderInset;
    } else if (!styleName->cmp("U")) {
      style = borderUnderlined;
    } else {
      style = borderSolid;
    }
    delete styleName;
  } else {
    width = 0;
  }
  obj2.free();
  obj1.free();

  // TODO: check not all zero (Line Dash Pattern Page 217 PDF 8.1)
  if (dict->lookup("D", &obj1)->isArray()) {
    GBool correct = gTrue;
    int tempLength = obj1.arrayGetLength();
    double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

    for(int i = 0; i < tempLength && correct; i++) {
      Object obj2;

      if (obj1.arrayGet(i, &obj2)->isNum()) {
        tempDash[i] = obj2.getNum();

        if (tempDash[i] < 0)
          correct = gFalse;
      } else {
        correct = gFalse;
      }
      obj2.free();
    }

    if (correct) {
      dashLength = tempLength;
      dash = tempDash;
      style = borderDashed;
    } else {
      gfree (tempDash);
    }

  }

  if (!dash) {
    dashLength = 1;
    dash = (double *) gmallocn (dashLength, sizeof (double));
    dash[0] = 3;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

AnnotColor::AnnotColor() {
  length = 0;
  values = NULL;
}

AnnotColor::AnnotColor(double gray) {
  length = 1;
  values = (double *) gmallocn (length, sizeof(double));

  values[0] = gray;
}

AnnotColor::AnnotColor(double r, double g, double b) {
  length = 3;
  values = (double *) gmallocn (length, sizeof(double));

  values[0] = r;
  values[1] = g;
  values[2] = b;
}

AnnotColor::AnnotColor(double c, double m, double y, double k) {
  length = 4;
  values = (double *) gmallocn (length, sizeof(double));

  values[0] = c;
  values[1] = m;
  values[2] = y;
  values[3] = k;
}

AnnotColor::AnnotColor(Array *array) {
  // TODO: check what Acrobat does in the case of having more than 5 numbers.
  if (array->getLength() < 5) {
    length = array->getLength();
    values = (double *) gmallocn (length, sizeof(double));

    for(int i = 0; i < length; i++) {  
      Object obj1;

      if (array->get(i, &obj1)->isNum()) {
        values[i] = obj1.getNum();

        if (values[i] < 0 || values[i] > 1)
          values[i] = 0;
      } else {
        values[i] = 0;
      }
      obj1.free();
    }
  } else {
    values = NULL;
    length = 0;
  }
}

AnnotColor::~AnnotColor() {
  if (values)
    gfree (values);
}

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

AnnotBorderStyle::AnnotBorderStyle(AnnotBorderType typeA, double widthA,
				   double *dashA, int dashLengthA,
				   double rA, double gA, double bA) {
  type = typeA;
  width = widthA;
  dash = dashA;
  dashLength = dashLengthA;
  r = rA;
  g = gA;
  b = bA;
}

AnnotBorderStyle::~AnnotBorderStyle() {
  if (dash) {
    gfree(dash);
  }
}

//------------------------------------------------------------------------
// AnnotIconFit
//------------------------------------------------------------------------

AnnotIconFit::AnnotIconFit(Dict* dict) {
  Object obj1;

  if (dict->lookup("SW", &obj1)->isName()) {
    GooString *scaleName = new GooString(obj1.getName());

    if(!scaleName->cmp("B")) {
      scaleWhen = scaleBigger;
    } else if(!scaleName->cmp("S")) {
      scaleWhen = scaleSmaller;
    } else if(!scaleName->cmp("N")) {
      scaleWhen = scaleNever;
    } else {
      scaleWhen = scaleAlways;
    }
    delete scaleName;
  } else {
    scaleWhen = scaleAlways;
  }
  obj1.free();

  if (dict->lookup("S", &obj1)->isName()) {
    GooString *scaleName = new GooString(obj1.getName());

    if(!scaleName->cmp("A")) {
      scale = scaleAnamorphic;
    } else {
      scale = scaleProportional;
    }
    delete scaleName;
  } else {
    scale = scaleProportional;
  }
  obj1.free();

  if (dict->lookup("A", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;
    (obj1.arrayGet(0, &obj2)->isNum() ? left = obj2.getNum() : left = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? bottom = obj2.getNum() : bottom = 0);
    obj2.free();

    if (left < 0 || left > 1)
      left = 0.5;

    if (bottom < 0 || bottom > 1)
      bottom = 0.5;

  } else {
    left = bottom = 0.5;
  }
  obj1.free();

  if (dict->lookup("FB", &obj1)->isBool()) {
    fullyBounds = obj1.getBool();
  } else {
    fullyBounds = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotAppearanceCharacs
//------------------------------------------------------------------------

AnnotAppearanceCharacs::AnnotAppearanceCharacs(Dict *dict) {
  Object obj1;

  if (dict->lookup("R", &obj1)->isInt()) {
    rotation = obj1.getInt();
  } else {
    rotation = 0;
  }
  obj1.free();

  if (dict->lookup("BC", &obj1)->isArray()) {
    borderColor = new AnnotColor(obj1.getArray());
  } else {
    borderColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BG", &obj1)->isArray()) {
    backColor = new AnnotColor(obj1.getArray());
  } else {
    backColor = NULL;
  }
  obj1.free();

  if (dict->lookup("CA", &obj1)->isName()) {
    normalCaption = new GooString(obj1.getName());
  } else {
    normalCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("RC", &obj1)->isName()) {
    rolloverCaption = new GooString(obj1.getName());
  } else {
    rolloverCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("AC", &obj1)->isName()) {
    alternateCaption = new GooString(obj1.getName());
  } else {
    alternateCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("IF", &obj1)->isDict()) {
    iconFit = new AnnotIconFit(obj1.getDict());
  } else {
    iconFit = NULL;
  }
  obj1.free();

  if (dict->lookup("TP", &obj1)->isInt()) {
    position = (AnnotAppearanceCharacsTextPos) obj1.getInt();
  } else {
    position = captionNoIcon;
  }
  obj1.free();
}

AnnotAppearanceCharacs::~AnnotAppearanceCharacs() {
  if (borderColor)
    delete borderColor;

  if (backColor)
    delete backColor;

  if (normalCaption)
    delete normalCaption;

  if (rolloverCaption)
    delete rolloverCaption;

  if (alternateCaption)
    delete alternateCaption;

  if (iconFit)
    delete iconFit;
}

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

Annot::Annot(XRef *xrefA, PDFRectangle *rectA, Catalog *catalog) {
  Object obj1;

  flags = flagUnknown;
  type = typeUnknown;

  obj1.initArray (xrefA);
  Object obj2;
  obj1.arrayAdd (obj2.initReal (rectA->x1));
  obj1.arrayAdd (obj2.initReal (rectA->y1));
  obj1.arrayAdd (obj2.initReal (rectA->x2));
  obj1.arrayAdd (obj2.initReal (rectA->y2));
  obj2.free ();

  annotObj.initDict (xrefA);
  annotObj.dictSet ("Type", obj2.initName ("Annot"));
  annotObj.dictSet ("Rect", &obj1);
  // obj1 is owned by the dict

  ref = xrefA->addIndirectObject (&annotObj);

  initialize (xrefA, annotObj.getDict(), catalog);
}

Annot::Annot(XRef *xrefA, Dict *dict, Catalog* catalog) {
  hasRef = false;
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (xrefA, dict, catalog);
}

Annot::Annot(XRef *xrefA, Dict *dict, Catalog* catalog, Object *obj) {
  if (obj->isRef()) {
    hasRef = gTrue;
    ref = obj->getRef();
  } else {
    hasRef = gFalse;
  }
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (xrefA, dict, catalog);
}

void Annot::initialize(XRef *xrefA, Dict *dict, Catalog *catalog) {
  Object asObj, obj1, obj2, obj3;

  appRef.num = 0;
  appRef.gen = 65535;
  ok = gTrue;
  xref = xrefA;
  appearBuf = NULL;
  fontSize = 0;

  //----- parse the rectangle
  rect = new PDFRectangle();
  if (dict->lookup("Rect", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    (obj1.arrayGet(0, &obj2)->isNum() ? rect->x1 = obj2.getNum() : rect->x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? rect->y1 = obj2.getNum() : rect->y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? rect->x2 = obj2.getNum() : rect->x2 = 1);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? rect->y2 = obj2.getNum() : rect->y2 = 1);
    obj2.free();

    if (rect->x1 > rect->x2) {
      double t = rect->x1;
      rect->x1 = rect->x2;
      rect->x2 = t;
    }

    if (rect->y1 > rect->y2) {
      double t = rect->y1;
      rect->y1 = rect->y2;
      rect->y2 = t;
    }
  } else {
    rect->x1 = rect->y1 = 0;
    rect->x2 = rect->y2 = 1;
    error(-1, "Bad bounding box for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Contents", &obj1)->isString()) {
    contents = obj1.getString()->copy();
  } else {
    contents = NULL;
  }
  obj1.free();

  /* TODO: Page Object indirect reference (should be parsed ?) */
  pageDict = NULL;
  /*if (dict->lookup("P", &obj1)->isDict()) {
    pageDict = NULL;
  } else {
    pageDict = NULL;
  }
  obj1.free();
  */

  if (dict->lookup("NM", &obj1)->isString()) {
    name = obj1.getString()->copy();
  } else {
    name = NULL;
  }
  obj1.free();

  if (dict->lookup("M", &obj1)->isString()) {
    modified = obj1.getString()->copy();
  } else {
    modified = NULL;
  }
  obj1.free();

  //----- get the flags
  if (dict->lookup("F", &obj1)->isInt()) {
    flags |= obj1.getInt();
  } else {
    flags = flagUnknown;
  }
  obj1.free();

  if (dict->lookup("AP", &obj1)->isDict()) {
    Object obj2;

    if (dict->lookup("AS", &obj2)->isName()) {
      Object obj3;

      appearState = new GooString(obj2.getName());
      if (obj1.dictLookup("N", &obj3)->isDict()) {
        Object obj4;

        if (obj3.dictLookupNF(appearState->getCString(), &obj4)->isRef()) {
          obj4.copy(&appearance);
        } else {
          obj4.free();
          if (obj3.dictLookupNF("Off", &obj4)->isRef()) {
            obj4.copy(&appearance);
          }
        } 
        obj4.free();
      }
      obj3.free();
    } else {
      obj2.free();

      appearState = NULL;
      if (obj1.dictLookupNF("N", &obj2)->isRef()) {
        obj2.copy(&appearance);
      }
    }
    obj2.free();
  } else {
    appearState = NULL;
  }
  obj1.free();

  //----- parse the border style
  if (dict->lookup("BS", &obj1)->isDict()) {
    border = new AnnotBorderBS(obj1.getDict());
  } else {
    obj1.free();

    if (dict->lookup("Border", &obj1)->isArray())
      border = new AnnotBorderArray(obj1.getArray());
    else
      // Adobe draws no border at all if the last element is of
      // the wrong type.
      border = NULL;
  }
  obj1.free();

  if (dict->lookup("C", &obj1)->isArray()) {
    color = new AnnotColor(obj1.getArray());
  } else {
    color = NULL;
  }
  obj1.free();

  if (dict->lookup("StructParent", &obj1)->isInt()) {
    treeKey = obj1.getInt();
  } else {
    treeKey = 0;
  }
  obj1.free();

  optContentConfig = catalog ? catalog->getOptContentConfig() : NULL;
  dict->lookupNF("OC", &oc);
  if (!oc.isRef() && !oc.isNull()) {
    error (-1, "Annotation OC value not null or dict: %i", oc.getType());
  }
}

void Annot::update(const char *key, Object *value) {
  /* Set M to current time */
  delete modified;
  modified = timeToDateString(NULL);

  Object obj1;
  obj1.initString (modified->copy());
  annotObj.dictSet("M", &obj1);

  annotObj.dictSet(const_cast<char*>(key), value);
  
  xref->setModifiedObject(&annotObj, ref);
}

void Annot::setContents(GooString *new_content) {
  delete contents;

  if (new_content) {
    contents = new GooString(new_content);
    //append the unicode marker <FE FF> if needed	
    if (!contents->hasUnicodeMarker()) {
      contents->insert(0, 0xff);
      contents->insert(0, 0xfe);
    }
  } else {
    contents = new GooString();
  }
  
  Object obj1;
  obj1.initString(contents->copy());
  update ("Contents", &obj1);
}

void Annot::setColor(AnnotColor *new_color) {
  delete color;

  if (new_color) {
    Object obj1, obj2;
    double *values = new_color->getValues();

    obj1.initArray(xref);
    for (int i = 0; i < (int)new_color->getSpace(); i++)
      obj1.arrayAdd(obj2.initReal (values[i]));
    update ("C", &obj1);
    color = new_color;
  } else {
    color = NULL;
  }
}

double Annot::getXMin() {
  return rect->x1;
}

double Annot::getYMin() {
  return rect->y1;
}

void Annot::readArrayNum(Object *pdfArray, int key, double *value) {
  Object valueObject;

  pdfArray->arrayGet(key, &valueObject);
  if (valueObject.isNum()) {
    *value = valueObject.getNum();
  } else {
    *value = 0;
    ok = gFalse;
  }
  valueObject.free();
}

Annot::~Annot() {
  annotObj.free();
  
  delete rect;
  
  if (contents)
    delete contents;

  if (pageDict)
    delete pageDict;

  if (name)
    delete name;

  if (modified)
    delete modified;

  appearance.free();

  if (appearState)
    delete appearState;

  if (border)
    delete border;

  if (color)
    delete color;

  oc.free();
}

// Set the current fill or stroke color, based on <a> (which should
// have 1, 3, or 4 elements).  If <adjust> is +1, color is brightened;
// if <adjust> is -1, color is darkened; otherwise color is not
// modified.
void Annot::setColor(Array *a, GBool fill, int adjust) {
  Object obj1;
  double color[4];
  int nComps, i;

  nComps = a->getLength();
  if (nComps > 4) {
    nComps = 4;
  }
  for (i = 0; i < nComps && i < 4; ++i) {
    if (a->get(i, &obj1)->isNum()) {
      color[i] = obj1.getNum();
    } else {
      color[i] = 0;
    }
    obj1.free();
  }
  if (nComps == 4) {
    adjust = -adjust;
  }
  if (adjust > 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i] + 0.5;
    }
  } else if (adjust < 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i];
    }
  }
  if (nComps == 4) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:c}\n",
        color[0], color[1], color[2], color[3],
        fill ? 'k' : 'K');
  } else if (nComps == 3) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:s}\n",
        color[0], color[1], color[2],
        fill ? "rg" : "RG");
  } else {
    appearBuf->appendf("{0:.2f} {1:c}\n",
        color[0],
        fill ? 'g' : 'G');
  }
}

// Draw an (approximate) circle of radius <r> centered at (<cx>, <cy>).
// If <fill> is true, the circle is filled; otherwise it is stroked.
void Annot::drawCircle(double cx, double cy, double r, GBool fill) {
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + r, cy + bezierCircle * r,
      cx + bezierCircle * r, cy + r,
      cx, cy + r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - bezierCircle * r, cy + r,
      cx - r, cy + bezierCircle * r,
      cx - r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - r, cy - bezierCircle * r,
      cx - bezierCircle * r, cy - r,
      cx, cy - r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + bezierCircle * r, cy - r,
      cx + r, cy - bezierCircle * r,
      cx + r, cy);
  appearBuf->append(fill ? "f\n" : "s\n");
}

// Draw the top-left half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleTopLeft(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r2, cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - r2,
      cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx - (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx - r2,
      cy - r2);
  appearBuf->append("S\n");
}

// Draw the bottom-right half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleBottomRight(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx - r2, cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + r2,
      cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx + (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx + r2,
      cy + r2);
  appearBuf->append("S\n");
}

void Annot::draw(Gfx *gfx, GBool printing) {
  Object obj;

  // check the flags
  if ((flags & annotFlagHidden) ||
      (printing && !(flags & annotFlagPrint)) ||
      (!printing && (flags & annotFlagNoView))) {
    return;
  }

  // check the OC
  if (optContentConfig && oc.isRef()) {
    if (! optContentConfig->optContentIsVisible(&oc))
      return;
  }

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (type == typeLink) ? border : (AnnotBorder *)NULL, color,
      rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

AnnotPopup::AnnotPopup(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typePopup;

  annotObj.dictSet ("Subtype", obj1.initName ("Popup"));
  initialize (xrefA, annotObj.getDict(), catalog);
}

AnnotPopup::AnnotPopup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  type = typePopup;
  initialize(xrefA, dict, catalog);
}

AnnotPopup::~AnnotPopup() {
  parent.free();
}

void AnnotPopup::initialize(XRef *xrefA, Dict *dict, Catalog *catalog) {
  Object obj1;

  if (!dict->lookupNF("Parent", &parent)->isRef()) {
    parent.initNull();
  }

  if (dict->lookup("Open", &obj1)->isBool()) {
    open = obj1.getBool();
  } else {
    open = gFalse;
  }
  obj1.free();
}

void AnnotPopup::setParent(Object *parentA) {
  parentA->copy(&parent);
  update ("Parent", &parent);
}

void AnnotPopup::setParent(Annot *parentA) {
  Ref parentRef = parentA->getRef();
  parent.initRef(parentRef.num, parentRef.gen);
  update ("Parent", &parent);
}

void AnnotPopup::setOpen(GBool openA) {
  Object obj1;

  open = openA;
  obj1.initBool(open);
  update ("Open", &obj1);
}

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------
AnnotMarkup::AnnotMarkup(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  initialize(xrefA, annotObj.getDict(), catalog, &annotObj);
}

AnnotMarkup::AnnotMarkup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  initialize(xrefA, dict, catalog, obj);
}

AnnotMarkup::~AnnotMarkup() {
  if (label)
    delete label;

  if (popup)
    delete popup;

  if (date)
    delete date;

  if (subject)
    delete subject;
}

void AnnotMarkup::initialize(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) {
  Object obj1;

  if (dict->lookup("T", &obj1)->isString()) {
    label = obj1.getString()->copy();
  } else {
    label = NULL;
  }
  obj1.free();

  if (dict->lookup("Popup", &obj1)->isDict()) {
    popup = new AnnotPopup(xrefA, obj1.getDict(), catalog, obj);
  } else {
    popup = NULL;
  }
  obj1.free();

  if (dict->lookup("CA", &obj1)->isNum()) {
    opacity = obj1.getNum();
  } else {
    opacity = 1.0;
  }
  obj1.free();

  if (dict->lookup("CreationDate", &obj1)->isString()) {
    date = obj1.getString()->copy();
  } else {
    date = NULL;
  }
  obj1.free();

  if (dict->lookupNF("IRT", &obj1)->isRef()) {
    inReplyTo = obj1.getRef();
  } else {
    inReplyTo.num = 0;
    inReplyTo.gen = 0;
  }
  obj1.free();

  if (dict->lookup("Subj", &obj1)->isString()) {
    subject = obj1.getString()->copy();
  } else {
    subject = NULL;
  }
  obj1.free();

  if (dict->lookup("RT", &obj1)->isName()) {
    GooString *replyName = new GooString(obj1.getName());

    if (!replyName->cmp("R")) {
      replyTo = replyTypeR;
    } else if (!replyName->cmp("Group")) {
      replyTo = replyTypeGroup;
    } else {
      replyTo = replyTypeR;
    }
    delete replyName;
  } else {
    replyTo = replyTypeR;
  }
  obj1.free();

  if (dict->lookup("ExData", &obj1)->isDict()) {
    exData = parseAnnotExternalData(obj1.getDict());
  } else {
    exData = annotExternalDataMarkupUnknown;
  }
  obj1.free();
}

void AnnotMarkup::setLabel(GooString *new_label) {
  delete label;

  if (new_label) {
    label = new GooString(new_label);
    //append the unicode marker <FE FF> if needed
    if (!label->hasUnicodeMarker()) {
      label->insert(0, 0xff);
      label->insert(0, 0xfe);
    }
  } else {
    label = new GooString();
  }

  Object obj1;
  obj1.initString(label->copy());
  update ("T", &obj1);
}

void AnnotMarkup::setPopup(AnnotPopup *new_popup) {
  delete popup;

  if (new_popup) {
    Object obj1;
    Ref popupRef = new_popup->getRef();

    obj1.initRef (popupRef.num, popupRef.gen);
    update ("Popup", &obj1);

    new_popup->setParent(this);
    popup = new_popup;
  } else {
    popup = NULL;
  }
}

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

AnnotText::AnnotText(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;

  annotObj.dictSet ("Subtype", obj1.initName ("Text"));
  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotText::AnnotText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;
  initialize (xrefA, catalog, dict);
}

AnnotText::~AnnotText() {
  delete icon;
}

void AnnotText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("Open", &obj1)->isBool())
    open = obj1.getBool();
  else
    open = gFalse;
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    icon = new GooString(obj1.getName());
  } else {
    icon = new GooString("Note");
  }
  obj1.free();

  if (dict->lookup("StateModel", &obj1)->isString()) {
    Object obj2;
    GooString *modelName = obj1.getString();

    if (dict->lookup("State", &obj2)->isString()) {
      GooString *stateName = obj2.getString();

      if (!stateName->cmp("Marked")) {
        state = stateMarked;
      } else if (!stateName->cmp("Unmarked")) {
        state = stateUnmarked;
      } else if (!stateName->cmp("Accepted")) {
        state = stateAccepted;
      } else if (!stateName->cmp("Rejected")) {
        state = stateRejected;
      } else if (!stateName->cmp("Cancelled")) {
        state = stateCancelled;
      } else if (!stateName->cmp("Completed")) {
        state = stateCompleted;
      } else if (!stateName->cmp("None")) {
        state = stateNone;
      } else {
        state = stateUnknown;
      }
    } else {
      state = stateUnknown;
    }
    obj2.free();

    if (!modelName->cmp("Marked")) {
      switch (state) {
        case stateUnknown:
          state = stateMarked;
          break;
        case stateAccepted:
        case stateRejected:
        case stateCancelled:
        case stateCompleted:
        case stateNone:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else if (!modelName->cmp("Review")) {
      switch (state) {
        case stateUnknown:
          state = stateNone;
          break;
        case stateMarked:
        case stateUnmarked:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else {
      state = stateUnknown;
    }
  } else {
    state = stateUnknown;
  }
  obj1.free();
}

void AnnotText::setOpen(GBool openA) {
  Object obj1;

  open = openA;
  obj1.initBool(open);
  update ("Open", &obj1);
}

void AnnotText::setIcon(GooString *new_icon) {
  if (new_icon && icon->cmp(new_icon) == 0)
    return;

  delete icon;

  if (new_icon) {
    icon = new GooString (new_icon);
  } else {
    icon = new GooString("Note");
  }

  Object obj1;
  obj1.initName (icon->getCString());
  update("Name", &obj1);
}

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------
AnnotLink::AnnotLink(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeLink;
  annotObj.dictSet ("Subtype", obj1.initName ("Link"));
  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotLink::AnnotLink(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {

  type = typeLink;
  initialize (xrefA, catalog, dict);
}

AnnotLink::~AnnotLink() {
  /*
  if (actionDict)
    delete actionDict;
  */
  dest.free();
  /*
  if (uriAction)
    delete uriAction;
  */
  if (quadrilaterals)
    delete quadrilaterals;
}

void AnnotLink::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;
  /*
  if (dict->lookup("A", &obj1)->isDict()) {
    actionDict = NULL;
  } else {
    actionDict = NULL;
  }
  obj1.free();
  */
  dict->lookup("Dest", &dest);
  if (dict->lookup("H", &obj1)->isName()) {
    GooString *effect = new GooString(obj1.getName());

    if (!effect->cmp("N")) {
      linkEffect = effectNone;
    } else if (!effect->cmp("I")) {
      linkEffect = effectInvert;
    } else if (!effect->cmp("O")) {
      linkEffect = effectOutline;
    } else if (!effect->cmp("P")) {
      linkEffect = effectPush;
    } else {
      linkEffect = effectInvert;
    }
    delete effect;
  } else {
    linkEffect = effectInvert;
  }
  obj1.free();
  /*
  if (dict->lookup("PA", &obj1)->isDict()) {
    uriAction = NULL;
  } else {
    uriAction = NULL;
  }
  obj1.free();
  */
  if (dict->lookup("QuadPoints", &obj1)->isArray()) {
    quadrilaterals = new AnnotQuadrilaterals(obj1.getArray(), rect);
  } else {
    quadrilaterals = NULL;
  }
  obj1.free();
}

void AnnotLink::draw(Gfx *gfx, GBool printing) {
  Object obj;

  // check the flags
  if ((flags & annotFlagHidden) ||
      (printing && !(flags & annotFlagPrint)) ||
      (!printing && (flags & annotFlagNoView))) {
    return;
  }

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------
AnnotFreeText::AnnotFreeText(XRef *xrefA, PDFRectangle *rect, GooString *da, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeFreeText;

  annotObj.dictSet ("Subtype", obj1.initName ("FreeText"));

  Object obj2;
  obj2.initString (da->copy());
  annotObj.dictSet("DA", &obj2);

  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotFreeText::AnnotFreeText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeFreeText;
  initialize(xrefA, catalog, dict);
}

AnnotFreeText::~AnnotFreeText() {
  delete appearanceString;

  if (styleString)
    delete styleString;

  if (calloutLine)
    delete calloutLine;

  if (borderEffect)
    delete borderEffect;

  if (rectangle)
    delete rectangle;
}

void AnnotFreeText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("DA", &obj1)->isString()) {
    appearanceString = obj1.getString()->copy();
  } else {
    appearanceString = new GooString();
    error(-1, "Bad appearance for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Q", &obj1)->isInt()) {
    quadding = (AnnotFreeTextQuadding) obj1.getInt();
  } else {
    quadding = quaddingLeftJustified;
  }
  obj1.free();

  if (dict->lookup("DS", &obj1)->isString()) {
    styleString = obj1.getString()->copy();
  } else {
    styleString = NULL;
  }
  obj1.free();

  if (dict->lookup("CL", &obj1)->isArray() && obj1.arrayGetLength() >= 4) {
    double x1, y1, x2, y2;
    Object obj2;

    (obj1.arrayGet(0, &obj2)->isNum() ? x1 = obj2.getNum() : x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? y1 = obj2.getNum() : y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? x2 = obj2.getNum() : x2 = 0);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? y2 = obj2.getNum() : y2 = 0);
    obj2.free();

    if (obj1.arrayGetLength() == 6) {
      double x3, y3;
      (obj1.arrayGet(4, &obj2)->isNum() ? x3 = obj2.getNum() : x3 = 0);
      obj2.free();
      (obj1.arrayGet(5, &obj2)->isNum() ? y3 = obj2.getNum() : y3 = 0);
      obj2.free();
      calloutLine = new AnnotCalloutMultiLine(x1, y1, x2, y2, x3, y3);
    } else {
      calloutLine = new AnnotCalloutLine(x1, y1, x2, y2);
    }
  } else {
    calloutLine = NULL;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    GooString *intentName = new GooString(obj1.getName());

    if (!intentName->cmp("FreeText")) {
      intent = intentFreeText;
    } else if (!intentName->cmp("FreeTextCallout")) {
      intent = intentFreeTextCallout;
    } else if (!intentName->cmp("FreeTextTypeWriter")) {
      intent = intentFreeTextTypeWriter;
    } else {
      intent = intentFreeText;
    }
    delete intentName;
  } else {
    intent = intentFreeText;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  if (dict->lookup("RD", &obj1)->isArray()) {
    rectangle = parseDiffRectangle(obj1.getArray(), rect);
  } else {
    rectangle = NULL;
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isName()) {
    GooString *styleName = new GooString(obj1.getName());
    endStyle = parseAnnotLineEndingStyle(styleName);
    delete styleName;
  } else {
    endStyle = annotLineEndingNone;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

AnnotLine::AnnotLine(XRef *xrefA, PDFRectangle *rect, PDFRectangle *lRect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeLine;
  annotObj.dictSet ("Subtype", obj1.initName ("Line"));

  Object obj2, obj3;
  obj2.initArray (xrefA);
  obj2.arrayAdd (obj3.initReal (lRect->x1));
  obj2.arrayAdd (obj3.initReal (lRect->y1));
  obj2.arrayAdd (obj3.initReal (lRect->x2));
  obj2.arrayAdd (obj3.initReal (lRect->y2));
  annotObj.dictSet ("L", &obj2);

  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotLine::AnnotLine(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeLine;
  initialize(xrefA, catalog, dict);
}

AnnotLine::~AnnotLine() {
  delete coord1;
  delete coord2;

  if (interiorColor)
    delete interiorColor;

  if (measure)
    delete measure;
}

void AnnotLine::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("L", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    double x1, y1, x2, y2;

    (obj1.arrayGet(0, &obj2)->isNum() ? x1 = obj2.getNum() : x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? y1 = obj2.getNum() : y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? x2 = obj2.getNum() : x2 = 0);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? y2 = obj2.getNum() : y2 = 0);
    obj2.free();

    coord1 = new AnnotCoord(x1, y1);
    coord2 = new AnnotCoord(x2, y2);
  } else {
    coord1 = new AnnotCoord();
    coord2 = new AnnotCoord();
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;

    if(obj1.arrayGet(0, &obj2)->isString())
      startStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      startStyle = annotLineEndingNone;
    obj2.free();

    if(obj1.arrayGet(1, &obj2)->isString())
      endStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      endStyle = annotLineEndingNone;
    obj2.free();

  } else {
    startStyle = endStyle = annotLineEndingNone;
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("LL", &obj1)->isNum()) {
    leaderLineLength = obj1.getNum();
  } else {
    leaderLineLength = 0;
  }
  obj1.free();

  if (dict->lookup("LLE", &obj1)->isNum()) {
    leaderLineExtension = obj1.getNum();

    if (leaderLineExtension < 0)
      leaderLineExtension = 0;
  } else {
    leaderLineExtension = 0;
  }
  obj1.free();

  if (dict->lookup("Cap", &obj1)->isBool()) {
    caption = obj1.getBool();
  } else {
    caption = gFalse;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    GooString *intentName = new GooString(obj1.getName());

    if(!intentName->cmp("LineArrow")) {
      intent = intentLineArrow;
    } else if(!intentName->cmp("LineDimension")) {
      intent = intentLineDimension;
    } else {
      intent = intentLineArrow;
    }
    delete intentName;
  } else {
    intent = intentLineArrow;
  }
  obj1.free();

  if (dict->lookup("LLO", &obj1)->isNum()) {
    leaderLineOffset = obj1.getNum();

    if (leaderLineOffset < 0)
      leaderLineOffset = 0;
  } else {
    leaderLineOffset = 0;
  }
  obj1.free();

  if (dict->lookup("CP", &obj1)->isName()) {
    GooString *captionName = new GooString(obj1.getName());

    if(!captionName->cmp("Inline")) {
      captionPos = captionPosInline;
    } else if(!captionName->cmp("Top")) {
      captionPos = captionPosTop;
    } else {
      captionPos = captionPosInline;
    }
    delete captionName;
  } else {
    captionPos = captionPosInline;
  }
  obj1.free();

  if (dict->lookup("Measure", &obj1)->isDict()) {
    measure = NULL;
  } else {
    measure = NULL;
  }
  obj1.free();

  if ((dict->lookup("CO", &obj1)->isArray()) && (obj1.arrayGetLength() == 2)) {
    Object obj2;

    (obj1.arrayGet(0, &obj2)->isNum() ? captionTextHorizontal = obj2.getNum() :
      captionTextHorizontal = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? captionTextVertical = obj2.getNum() :
      captionTextVertical = 0);
    obj2.free();
  } else {
    captionTextHorizontal = captionTextVertical = 0;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------
AnnotTextMarkup::AnnotTextMarkup(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType,
				 AnnotQuadrilaterals *quadPoints, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  switch (subType) {
    case typeHighlight:
      annotObj.dictSet ("Subtype", obj1.initName ("Highlight"));
      break;
    case typeUnderline:
      annotObj.dictSet ("Subtype", obj1.initName ("Underline"));
      break;
    case typeSquiggly:
      annotObj.dictSet ("Subtype", obj1.initName ("Squiggly"));
      break;
    case typeStrikeOut:
      annotObj.dictSet ("Subtype", obj1.initName ("StrikeOut"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotTextMarkup\n");
  }

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < quadPoints->getQuadrilateralsLength(); ++i) {
    Object obj3;

    obj2.arrayAdd (obj3.initReal (quadPoints->getX1(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY1(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX2(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY2(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX3(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY3(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX4(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY4(i)));
  }

  annotObj.dictSet ("QuadPoints", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotTextMarkup::AnnotTextMarkup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typeHighlight;
  initialize(xrefA, catalog, dict);
}

void AnnotTextMarkup::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Highlight")) {
      type = typeHighlight;
    } else if (!typeName.cmp("Underline")) {
      type = typeUnderline;
    } else if (!typeName.cmp("Squiggly")) {
      type = typeSquiggly;
    } else if (!typeName.cmp("StrikeOut")) {
      type = typeStrikeOut;
    }
  }
  obj1.free();

  if(dict->lookup("QuadPoints", &obj1)->isArray()) {
    quadrilaterals = new AnnotQuadrilaterals(obj1.getArray(), rect);
  } else {
    quadrilaterals = NULL;
  }
  obj1.free();
}

AnnotTextMarkup::~AnnotTextMarkup() {
  if(quadrilaterals) {
    delete quadrilaterals;
  }
}

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

AnnotWidget::AnnotWidget(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  type = typeWidget;
  widget = NULL;
  initialize(xrefA, catalog, dict);
}

AnnotWidget::~AnnotWidget() {
  if (appearCharacs)
    delete appearCharacs;
  
  if (action)
    delete action;
    
  if (additionActions)
    delete additionActions;
    
  if (parent)
    delete parent;
}

void AnnotWidget::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if ((form = catalog->getForm ())) {
    widget = form->findWidgetByRef (ref);

    // check if field apperances need to be regenerated
    // Only text or choice fields needs to have appearance regenerated
    // see section 8.6.2 "Variable Text" of PDFReference
    regen = gFalse;
    if (widget != NULL && (widget->getType () == formText || widget->getType () == formChoice)) {
      regen = form->getNeedAppearances ();
    }
  }

  // If field doesn't have an AP we'll have to generate it
  if (appearance.isNone () || appearance.isNull ())
    regen = gTrue;

  if(dict->lookup("H", &obj1)->isName()) {
    GooString *modeName = new GooString(obj1.getName());

    if(!modeName->cmp("N")) {
      mode = highlightModeNone;
    } else if(!modeName->cmp("O")) {
      mode = highlightModeOutline;
    } else if(!modeName->cmp("P") || !modeName->cmp("T")) {
      mode = highlightModePush;
    } else {
      mode = highlightModeInvert;
    }
    delete modeName;
  } else {
    mode = highlightModeInvert;
  }
  obj1.free();

  if(dict->lookup("MK", &obj1)->isDict()) {
    appearCharacs = new AnnotAppearanceCharacs(obj1.getDict());
  } else {
    appearCharacs = NULL;
  }
  obj1.free();

  if(dict->lookup("A", &obj1)->isDict()) {
    action = NULL;
  } else {
    action = NULL;
  }
  obj1.free();

  if(dict->lookup("AA", &obj1)->isDict()) {
    additionActions = NULL;
  } else {
    additionActions = NULL;
  }
  obj1.free();

  if(dict->lookup("Parent", &obj1)->isDict()) {
    parent = NULL;
  } else {
    parent = NULL;
  }
  obj1.free();
}

// Grand unified handler for preparing text strings to be drawn into form
// fields.  Takes as input a text string (in PDFDocEncoding or UTF-16).
// Converts some or all of this string to the appropriate encoding for the
// specified font, and computes the width of the text.  Can optionally stop
// converting when a specified width has been reached, to perform line-breaking
// for multi-line fields.
//
// Parameters:
//   text: input text string to convert
//   outBuf: buffer for writing re-encoded string
//   i: index at which to start converting; will be updated to point just after
//      last character processed
//   font: the font which will be used for display
//   width: computed width (unscaled by font size) will be stored here
//   widthLimit: if non-zero, stop converting to keep width under this value
//      (should be scaled down by font size)
//   charCount: count of number of characters will be stored here
//   noReencode: if set, do not try to translate the character encoding
//      (useful for Zapf Dingbats or other unusual encodings)
//      can only be used with simple fonts, not CID-keyed fonts
//
// TODO: Handle surrogate pairs in UTF-16.
//       Should be able to generate output for any CID-keyed font.
//       Doesn't handle vertical fonts--should it?
void AnnotWidget::layoutText(GooString *text, GooString *outBuf, int *i,
                             GfxFont *font, double *width, double widthLimit,
                             int *charCount, GBool noReencode)
{
  CharCode c;
  Unicode uChar, *uAux;
  double w = 0.0;
  int uLen, n;
  double dx, dy, ox, oy;
  GBool unicode = text->hasUnicodeMarker();
  GBool spacePrev;              // previous character was a space

  // State for backtracking when more text has been processed than fits within
  // widthLimit.  We track for both input (text) and output (outBuf) the offset
  // to the first character to discard.
  //
  // We keep track of several points:
  //   1 - end of previous completed word which fits
  //   2 - previous character which fit
  int last_i1, last_i2, last_o1, last_o2;

  if (unicode && text->getLength() % 2 != 0) {
    error(-1, "AnnotWidget::layoutText, bad unicode string");
    return;
  }

  // skip Unicode marker on string if needed
  if (unicode && *i == 0)
    *i = 2;

  // Start decoding and copying characters, until either:
  //   we reach the end of the string
  //   we reach the maximum width
  //   we reach a newline character
  // As we copy characters, keep track of the last full word to fit, so that we
  // can backtrack if we exceed the maximum width.
  last_i1 = last_i2 = *i;
  last_o1 = last_o2 = 0;
  spacePrev = gFalse;
  outBuf->clear();

  while (*i < text->getLength()) {
    last_i2 = *i;
    last_o2 = outBuf->getLength();

    if (unicode) {
      uChar = (unsigned char)(text->getChar(*i)) << 8;
      uChar += (unsigned char)(text->getChar(*i + 1));
      *i += 2;
    } else {
      if (noReencode)
        uChar = text->getChar(*i) & 0xff;
      else
        uChar = pdfDocEncoding[text->getChar(*i) & 0xff];
      *i += 1;
    }

    // Explicit line break?
    if (uChar == '\r' || uChar == '\n') {
      // Treat a <CR><LF> sequence as a single line break
      if (uChar == '\r' && *i < text->getLength()) {
        if (unicode && text->getChar(*i) == '\0'
            && text->getChar(*i + 1) == '\n')
          *i += 2;
        else if (!unicode && text->getChar(*i) == '\n')
          *i += 1;
      }

      break;
    }

    if (noReencode) {
      outBuf->append(uChar);
    } else {
      CharCodeToUnicode *ccToUnicode = font->getToUnicode();
      if (!ccToUnicode) {
        // This assumes an identity CMap.
        outBuf->append((uChar >> 8) & 0xff);
        outBuf->append(uChar & 0xff);
      } else if (ccToUnicode->mapToCharCode(&uChar, &c, 1)) {
        ccToUnicode->decRefCnt();
        if (font->isCIDFont()) {
          // TODO: This assumes an identity CMap.  It should be extended to
          // handle the general case.
          outBuf->append((c >> 8) & 0xff);
          outBuf->append(c & 0xff);
        } else {
          // 8-bit font
          outBuf->append(c);
        }
      } else {
        ccToUnicode->decRefCnt();
        fprintf(stderr,
                "warning: layoutText: cannot convert U+%04X\n", uChar);
      }
    }

    // If we see a space, then we have a linebreak opportunity.
    if (uChar == ' ') {
      last_i1 = *i;
      if (!spacePrev)
        last_o1 = last_o2;
      spacePrev = gTrue;
    } else {
      spacePrev = gFalse;
    }

    // Compute width of character just output
    if (outBuf->getLength() > last_o2) {
      dx = 0.0;
      font->getNextChar(outBuf->getCString() + last_o2,
                        outBuf->getLength() - last_o2,
                        &c, &uAux, &uLen, &dx, &dy, &ox, &oy);
      w += dx;
    }

    // Current line over-full now?
    if (widthLimit > 0.0 && w > widthLimit) {
      if (last_o1 > 0) {
        // Back up to the previous word which fit, if there was a previous
        // word.
        *i = last_i1;
        outBuf->del(last_o1, outBuf->getLength() - last_o1);
      } else if (last_o2 > 0) {
        // Otherwise, back up to the previous character (in the only word on
        // this line)
        *i = last_i2;
        outBuf->del(last_o2, outBuf->getLength() - last_o2);
      } else {
        // Otherwise, we were trying to fit the first character; include it
        // anyway even if it overflows the space--no updates to make.
      }
      break;
    }
  }

  // If splitting input into lines because we reached the width limit, then
  // consume any remaining trailing spaces that would go on this line from the
  // input.  If in doing so we reach a newline, consume that also.  This code
  // won't run if we stopped at a newline above, since in that case w <=
  // widthLimit still.
  if (widthLimit > 0.0 && w > widthLimit) {
    if (unicode) {
      while (*i < text->getLength()
             && text->getChar(*i) == '\0' && text->getChar(*i + 1) == ' ')
        *i += 2;
      if (*i < text->getLength()
          && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\r')
        *i += 2;
      if (*i < text->getLength()
          && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\n')
        *i += 2;
    } else {
      while (*i < text->getLength() && text->getChar(*i) == ' ')
        *i += 1;
      if (*i < text->getLength() && text->getChar(*i) == '\r')
        *i += 1;
      if (*i < text->getLength() && text->getChar(*i) == '\n')
        *i += 1;
    }
  }

  // Compute the actual width and character count of the final string, based on
  // breakpoint, if this information is requested by the caller.
  if (width != NULL || charCount != NULL) {
    char *s = outBuf->getCString();
    int len = outBuf->getLength();

    if (width != NULL)
      *width = 0.0;
    if (charCount != NULL)
      *charCount = 0;

    while (len > 0) {
      dx = 0.0;
      n = font->getNextChar(s, len, &c, &uAux, &uLen, &dx, &dy, &ox, &oy);

      if (n == 0) {
        break;
      }

      if (width != NULL)
        *width += dx;
      if (charCount != NULL)
        *charCount += 1;

      s += n;
      len -= n;
    }
  }
}

// Copy the given string to appearBuf, adding parentheses around it and
// escaping characters as appropriate.
void AnnotWidget::writeString(GooString *str, GooString *appearBuf)
{
  char c;
  int i;

  appearBuf->append('(');

  for (i = 0; i < str->getLength(); ++i) {
    c = str->getChar(i);
    if (c == '(' || c == ')' || c == '\\') {
      appearBuf->append('\\');
      appearBuf->append(c);
    } else if (c < 0x20) {
      appearBuf->appendf("\\{0:03o}", (unsigned char)c);
    } else {
      appearBuf->append(c);
    }
  }

  appearBuf->append(')');
}

// Draw the variable text or caption for a field.
void AnnotWidget::drawText(GooString *text, GooString *da, GfxFontDict *fontDict,
    GBool multiline, int comb, int quadding,
    GBool txField, GBool forceZapfDingbats,
    GBool password) {
  GooList *daToks;
  GooString *tok, *convertedText;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, xPrev, y, w, wMax;
  int tfPos, tmPos, i, j;
  GBool freeText = gFalse;      // true if text should be freed before return
  GBool freeFont = gFalse;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)
  
  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
        ++i;
      }
      if (i < da->getLength()) {
        for (j = i + 1;
            j < da->getLength() && !Lexer::isSpace(da->getChar(j));
            ++j) ;
        daToks->append(new GooString(da, i, j - i));
        i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
        tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
        tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // force ZapfDingbats
  if (forceZapfDingbats) {
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos);
      if (tok->cmp("/ZaDb")) {
        tok->clear();
        tok->append("/ZaDb");
      }
    }
  }
  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        if (forceZapfDingbats) {
          // We are forcing ZaDb but the font does not exist
          // so create a fake one
          Ref r; // dummy Ref, it's not used at all in this codepath
          r.num = 0;
          r.gen = 0;
          Dict *d = new Dict(xref);
          font = new Gfx8BitFont(xref, "ZaDb", r, new GooString("ZapfDingbats"), fontType1, d);
          delete d;
          freeFont = gTrue;
          addDingbatsResource = gTrue;
        } else {
          error(-1, "Unknown font in field's DA string");
        }
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    if (daToks) {
      deleteGooList(daToks, GooString);
    }
    return;
  }

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // for a password field, replace all characters with asterisks
  if (password) {
    int len;
    if (text->hasUnicodeMarker())
      len = (text->getLength() - 2) / 2;
    else
      len = text->getLength();

    text = new GooString;
    for (i = 0; i < len; ++i)
      text->append('*');
    freeText = gTrue;
  }

  convertedText = new GooString;

  // setup
  if (txField) {
    appearBuf->append("/Tx BMC\n");
  }
  appearBuf->append("q\n");
  appearBuf->append("BT\n");
  // multi-line text
  if (multiline) {
    // note: the comb flag is ignored in multiline mode

    wMax = rect->x2 - rect->x1 - 2 * borderWidth - 4;

    // compute font autosize
    if (fontSize == 0) {
      for (fontSize = 20; fontSize > 1; --fontSize) {
        y = rect->y2 - rect->y1;
        i = 0;
        while (i < text->getLength()) {
          layoutText(text, convertedText, &i, font, &w, wMax / fontSize, NULL,
                     forceZapfDingbats);
          y -= fontSize;
        }
        // approximate the descender for the last line
        if (y >= 0.33 * fontSize) {
          break;
        }
      }
      if (tfPos >= 0) {
        tok = (GooString *)daToks->get(tfPos + 1);
        tok->clear();
        tok->appendf("{0:.2f}", fontSize);
      }
    }

    // starting y coordinate
    // (note: each line of text starts with a Td operator that moves
    // down a line)
    y = rect->y2 - rect->y1;

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->append('0');
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (i = 0; i < daToks->getLength(); ++i) {
        appearBuf->append((GooString *)daToks->get(i))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 0 {0:.2f} Tm\n", y);
    }

    // write a series of lines of text
    i = 0;
    xPrev = 0;
    while (i < text->getLength()) {
      layoutText(text, convertedText, &i, font, &w, wMax / fontSize, NULL,
                 forceZapfDingbats);
      w *= fontSize;

      // compute text start position
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
          break;
      }

      // draw the line
      appearBuf->appendf("{0:.2f} {1:.2f} Td\n", x - xPrev, -fontSize);
      writeString(convertedText, appearBuf);
      appearBuf->append(" Tj\n");

      // next line
      xPrev = x;
    }

    // single-line text
  } else {
    //~ replace newlines with spaces? - what does Acrobat do?

    // comb formatting
    if (comb > 0) {
      int charCount;

      // compute comb spacing
      w = (rect->x2 - rect->x1 - 2 * borderWidth) / comb;

      // compute font autosize
      if (fontSize == 0) {
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        if (w < fontSize) {
          fontSize = w;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      i = 0;
      layoutText(text, convertedText, &i, font, NULL, 0.0, &charCount,
                 forceZapfDingbats);
      if (charCount > comb)
        charCount = comb;

      // compute starting text cell
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth;
          break;
        case fieldQuadCenter:
          x = borderWidth + (comb - charCount) / 2 * w;
          break;
        case fieldQuadRight:
          x = borderWidth + (comb - charCount) * w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      char *s = convertedText->getCString();
      int len = convertedText->getLength();
      i = 0;
      xPrev = w;                // so that first character is placed properly
      while (i < comb && len > 0) {
        CharCode code;
        Unicode *uAux;
        int uLen, n;
        double dx, dy, ox, oy;

        dx = 0.0;
        n = font->getNextChar(s, len, &code, &uAux, &uLen, &dx, &dy, &ox, &oy);
        dx *= fontSize;

        // center each character within its cell, by advancing the text
        // position the appropriate amount relative to the start of the
        // previous character
        x = 0.5 * (w - dx);
        appearBuf->appendf("{0:.2f} 0 Td\n", x - xPrev + w);

        GooString *charBuf = new GooString(s, n);
        writeString(charBuf, appearBuf);
        appearBuf->append(" Tj\n");
        delete charBuf;

        i++;
        s += n;
        len -= n;
        xPrev = x;
      }

      // regular (non-comb) formatting
    } else {
      i = 0;
      layoutText(text, convertedText, &i, font, &w, 0.0, NULL,
                 forceZapfDingbats);

      // compute font autosize
      if (fontSize == 0) {
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / w;
        if (fontSize2 < fontSize) {
          fontSize = fontSize2;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      // compute text start position
      w *= fontSize;
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      writeString(convertedText, appearBuf);
      appearBuf->append(" Tj\n");
    }
  }
  // cleanup
  appearBuf->append("ET\n");
  appearBuf->append("Q\n");
  if (txField) {
    appearBuf->append("EMC\n");
  }
  if (daToks) {
    deleteGooList(daToks, GooString);
  }
  if (freeText) {
    delete text;
  }
  delete convertedText;
  if (freeFont) {
    font->decRefCnt();
  }
}

// Draw the variable text or caption for a field.
void AnnotWidget::drawListBox(GooString **text, GBool *selection,
			      int nOptions, int topIdx,
			      GooString *da, GfxFontDict *fontDict, GBool quadding) {
  GooList *daToks;
  GooString *tok, *convertedText;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, y, w, wMax;
  int tfPos, tmPos, i, j;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)

  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
	++i;
       }
      if (i < da->getLength()) {
	for (j = i + 1;
	     j < da->getLength() && !Lexer::isSpace(da->getChar(j));
	     ++j) ;
	daToks->append(new GooString(da, i, j - i));
	i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
	tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
	tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        error(-1, "Unknown font in field's DA string");
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    if (daToks) {
      deleteGooList(daToks, GooString);
    }
    return;
  }

  convertedText = new GooString;

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // compute font autosize
  if (fontSize == 0) {
    wMax = 0;
    for (i = 0; i < nOptions; ++i) {
      j = 0;
      layoutText(text[i], convertedText, &j, font, &w, 0.0, NULL, gFalse);
      if (w > wMax) {
        wMax = w;
      }
    }
    fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
    fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / wMax;
    if (fontSize2 < fontSize) {
      fontSize = fontSize2;
    }
    fontSize = floor(fontSize);
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos + 1);
      tok->clear();
      tok->appendf("{0:.2f}", fontSize);
    }
  }
  // draw the text
  y = rect->y2 - rect->y1 - 1.1 * fontSize;
  for (i = topIdx; i < nOptions; ++i) {
    // setup
    appearBuf->append("q\n");

    // draw the background if selected
    if (selection[i]) {
      appearBuf->append("0 g f\n");
      appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re f\n",
          borderWidth,
          y - 0.2 * fontSize,
          rect->x2 - rect->x1 - 2 * borderWidth,
          1.1 * fontSize);
    }

    // setup
    appearBuf->append("BT\n");

    // compute text width and start position
    j = 0;
    layoutText(text[i], convertedText, &j, font, &w, 0.0, NULL, gFalse);
    w *= fontSize;
    switch (quadding) {
      case fieldQuadLeft:
      default:
        x = borderWidth + 2;
        break;
      case fieldQuadCenter:
        x = (rect->x2 - rect->x1 - w) / 2;
        break;
      case fieldQuadRight:
        x = rect->x2 - rect->x1 - borderWidth - 2 - w;
        break;
    }

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->appendf("{0:.2f}", x);
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (j = 0; j < daToks->getLength(); ++j) {
        appearBuf->append((GooString *)daToks->get(j))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
    }

    // change the text color if selected
    if (selection[i]) {
      appearBuf->append("1 g\n");
    }

    // write the text string
    writeString(convertedText, appearBuf);
    appearBuf->append(" Tj\n");

    // cleanup
    appearBuf->append("ET\n");
    appearBuf->append("Q\n");

    // next line
    y -= 1.1 * fontSize;
  }

  if (daToks) {
    deleteGooList(daToks, GooString);
  }

  delete convertedText;
}

void AnnotWidget::generateFieldAppearance() {
  Object mkObj, ftObj, appearDict, drObj, obj1, obj2, obj3;
  Dict *field;
  Dict *annot;
  Dict *acroForm;
  Dict *mkDict;
  MemStream *appearStream;
  GfxFontDict *fontDict;
  GBool hasCaption;
  double w, dx, dy, r;
  double *dash;
  GooString *caption, *da;
  GooString **text;
  GBool *selection;
  int dashLength, ff, quadding, comb, nOptions, topIdx, i, j;
  GBool modified;

  if (widget == NULL || !widget->getField () || !widget->getField ()->getObj ()->isDict ())
    return;

  field = widget->getField ()->getObj ()->getDict ();
  annot = widget->getObj ()->getDict ();
  acroForm = form->getObj ()->getDict ();
  
  // do not regenerate appearence if widget has not changed
  modified = widget->isModified ();

  // only regenerate when it doesn't have an AP or
  // it already has an AP but widget has been modified
  if (!regen && !modified) {
    return;
  }

  appearBuf = new GooString ();
  // get the appearance characteristics (MK) dictionary
  if (annot->lookup("MK", &mkObj)->isDict()) {
    mkDict = mkObj.getDict();
  } else {
    mkDict = NULL;
  }
  // draw the background
  if (mkDict) {
    if (mkDict->lookup("BG", &obj1)->isArray() &&
        obj1.arrayGetLength() > 0) {
      setColor(obj1.getArray(), gTrue, 0);
      appearBuf->appendf("0 0 {0:.2f} {1:.2f} re f\n",
          rect->x2 - rect->x1, rect->y2 - rect->y1);
    }
    obj1.free();
  }

  // get the field type
  Form::fieldLookup(field, "FT", &ftObj);

  // get the field flags (Ff) value
  if (Form::fieldLookup(field, "Ff", &obj1)->isInt()) {
    ff = obj1.getInt();
  } else {
    ff = 0;
  }
  obj1.free();

  // draw the border
  if (mkDict && border) {
    w = border->getWidth();
    if (w > 0) {
      mkDict->lookup("BC", &obj1);
      if (!(obj1.isArray() && obj1.arrayGetLength() > 0)) {
        mkDict->lookup("BG", &obj1);
      }
      if (obj1.isArray() && obj1.arrayGetLength() > 0) {
        dx = rect->x2 - rect->x1;
        dy = rect->y2 - rect->y1;

        // radio buttons with no caption have a round border
        hasCaption = mkDict->lookup("CA", &obj2)->isString();
        obj2.free();
        if (ftObj.isName("Btn") && (ff & fieldFlagRadio) && !hasCaption) {
          r = 0.5 * (dx < dy ? dx : dy);
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
            case AnnotBorder::borderUnderlined:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, gFalse);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
              appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
              setColor(obj1.getArray(), gFalse, 0);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, gFalse);
              setColor(obj1.getArray(), gFalse,
                  border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
              drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
              setColor(obj1.getArray(), gFalse,
                  border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
              break;
          }

        } else {
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n",
                  0.5 * w, dx - w, dy - w);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
              setColor(obj1.getArray(), gTrue,
                  border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
              appearBuf->append("0 0 m\n");
              appearBuf->appendf("0 {0:.2f} l\n", dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
              appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
              appearBuf->append("f\n");
              setColor(obj1.getArray(), gTrue,
                  border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              appearBuf->append("0 0 m\n");
              appearBuf->appendf("{0:.2f} 0 l\n", dx);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, w);
              appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
              appearBuf->append("f\n");
              break;
            case AnnotBorder::borderUnderlined:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
              break;
          }

          // clip to the inside of the border
          appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n",
              w, dx - 2 * w, dy - 2 * w);
        }
      }
      obj1.free();
    }
  }

  // get the resource dictionary
  acroForm->lookup("DR", &drObj);

  // build the font dictionary
  if (drObj.isDict() && drObj.dictLookup("Font", &obj1)->isDict()) {
    fontDict = new GfxFontDict(xref, NULL, obj1.getDict());
  } else {
    fontDict = NULL;
  }
  obj1.free();

  // get the default appearance string
  if (Form::fieldLookup(field, "DA", &obj1)->isNull()) {
    obj1.free();
    acroForm->lookup("DA", &obj1);
  }
  if (obj1.isString()) {
    da = obj1.getString()->copy();
    //TODO: look for a font size / name HERE
    // => create a function
  } else {
    da = NULL;
  }
  obj1.free();

  // draw the field contents
  if (ftObj.isName("Btn")) {
    caption = NULL;
    if (mkDict) {
      if (mkDict->lookup("CA", &obj1)->isString()) {
        caption = obj1.getString()->copy();
      }
      obj1.free();
    }
    // radio button
    if (ff & fieldFlagRadio) {
      //~ Acrobat doesn't draw a caption if there is no AP dict (?)
      if (Form::fieldLookup(field, "V", &obj1)->isName()) {
        if (annot->lookup("AS", &obj2)->isName(obj1.getName()) &&
	    strcmp (obj1.getName(), "Off") != 0) {
          if (caption) {
            drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
                gFalse, gTrue);
          } else {
            if (mkDict) {
              if (mkDict->lookup("BC", &obj3)->isArray() &&
                  obj3.arrayGetLength() > 0) {
                dx = rect->x2 - rect->x1;
                dy = rect->y2 - rect->y1;
                setColor(obj3.getArray(), gTrue, 0);
                drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy),
                    gTrue);
              }
              obj3.free();
            }
          }
        }
        obj2.free();
      }
      obj1.free();
      // pushbutton
    } else if (ff & fieldFlagPushbutton) {
      if (caption) {
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gFalse);
      }
      // checkbox
    } else {
      if (annot->lookup("AS", &obj1)->isName() &&
          strcmp(obj1.getName(), "Off") != 0) {
        if (!caption) {
          caption = new GooString("3"); // ZapfDingbats checkmark
        }
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gTrue);
      }
      obj1.free();
    }
    if (caption) {
      delete caption;
    }
  } else if (ftObj.isName("Tx")) {
    if (Form::fieldLookup(field, "V", &obj1)->isString()) {
      if (Form::fieldLookup(field, "Q", &obj2)->isInt()) {
        quadding = obj2.getInt();
      } else {
        quadding = fieldQuadLeft;
      }
      obj2.free();
      comb = 0;
      if (ff & fieldFlagComb) {
        if (Form::fieldLookup(field, "MaxLen", &obj2)->isInt()) {
          comb = obj2.getInt();
        }
        obj2.free();
      }
      drawText(obj1.getString(), da, fontDict,
          ff & fieldFlagMultiline, comb, quadding, gTrue, gFalse, ff & fieldFlagPassword);
    }
    obj1.free();
  } else if (ftObj.isName("Ch")) {
    if (Form::fieldLookup(field, "Q", &obj1)->isInt()) {
      quadding = obj1.getInt();
    } else {
      quadding = fieldQuadLeft;
    }
    obj1.free();
    // combo box
    if (ff & fieldFlagCombo) {
      if (Form::fieldLookup(field, "V", &obj1)->isString()) {
        drawText(obj1.getString(), da, fontDict,
            gFalse, 0, quadding, gTrue, gFalse);
        //~ Acrobat draws a popup icon on the right side
      }
      obj1.free();
      // list box
    } else {
      if (field->lookup("Opt", &obj1)->isArray()) {
        nOptions = obj1.arrayGetLength();
        // get the option text
        text = (GooString **)gmallocn(nOptions, sizeof(GooString *));
        for (i = 0; i < nOptions; ++i) {
          text[i] = NULL;
          obj1.arrayGet(i, &obj2);
          if (obj2.isString()) {
            text[i] = obj2.getString()->copy();
          } else if (obj2.isArray() && obj2.arrayGetLength() == 2) {
            if (obj2.arrayGet(1, &obj3)->isString()) {
              text[i] = obj3.getString()->copy();
            }
            obj3.free();
          }
          obj2.free();
          if (!text[i]) {
            text[i] = new GooString();
          }
        }
        // get the selected option(s)
        selection = (GBool *)gmallocn(nOptions, sizeof(GBool));
        //~ need to use the I field in addition to the V field
	Form::fieldLookup(field, "V", &obj2);
        for (i = 0; i < nOptions; ++i) {
          selection[i] = gFalse;
          if (obj2.isString()) {
            if (!obj2.getString()->cmp(text[i])) {
              selection[i] = gTrue;
            }
          } else if (obj2.isArray()) {
            for (j = 0; j < obj2.arrayGetLength(); ++j) {
              if (obj2.arrayGet(j, &obj3)->isString() &&
                  !obj3.getString()->cmp(text[i])) {
                selection[i] = gTrue;
              }
              obj3.free();
            }
          }
        }
        obj2.free();
        // get the top index
        if (field->lookup("TI", &obj2)->isInt()) {
          topIdx = obj2.getInt();
        } else {
          topIdx = 0;
        }
        obj2.free();
        // draw the text
        drawListBox(text, selection, nOptions, topIdx, da, fontDict, quadding);
        for (i = 0; i < nOptions; ++i) {
          delete text[i];
        }
        gfree(text);
        gfree(selection);
      }
      obj1.free();
    }
  } else if (ftObj.isName("Sig")) {
    //~unimp
  } else {
    error(-1, "Unknown field type");
  }

  if (da) {
    delete da;
  }

  // build the appearance stream dictionary
  appearDict.initDict(xref);
  appearDict.dictAdd(copyString("Length"),
      obj1.initInt(appearBuf->getLength()));
  appearDict.dictAdd(copyString("Subtype"), obj1.initName("Form"));
  obj1.initArray(xref);
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(rect->x2 - rect->x1));
  obj1.arrayAdd(obj2.initReal(rect->y2 - rect->y1));
  appearDict.dictAdd(copyString("BBox"), &obj1);

  // set the resource dictionary
  if (drObj.isDict()) {
    appearDict.dictAdd(copyString("Resources"), drObj.copy(&obj1));
  }
  drObj.free();

  // build the appearance stream
  appearStream = new MemStream(strdup(appearBuf->getCString()), 0,
      appearBuf->getLength(), &appearDict);
  appearance.free();
  appearance.initStream(appearStream);
  delete appearBuf;

  appearStream->setNeedFree(gTrue);

  if (widget->isModified()) {
    //create a new object that will contains the new appearance
    
    //if we already have a N entry in our AP dict, reuse it
    if (annot->lookup("AP", &obj1)->isDict() &&
        obj1.dictLookupNF("N", &obj2)->isRef()) {
      appRef = obj2.getRef();
    }

    obj2.free();
    obj1.free();

    // this annot doesn't have an AP yet, create one
    if (appRef.num == 0)
      appRef = xref->addIndirectObject(&appearance);
    else // since we reuse the already existing AP, we have to notify the xref about this update
      xref->setModifiedObject(&appearance, appRef);

    // update object's AP and AS
    Object apObj;
    apObj.initDict(xref);

    Object oaRef;
    oaRef.initRef(appRef.num, appRef.gen);

    apObj.dictSet("N", &oaRef);
    annot->set("AP", &apObj);
    Dict* d = new Dict(annot);
    d->decRef();
    Object dictObj;
    dictObj.initDict(d);

    xref->setModifiedObject(&dictObj, ref);
    dictObj.free();
  }

  if (fontDict) {
    delete fontDict;
  }
  ftObj.free();
  mkObj.free();
}


void AnnotWidget::draw(Gfx *gfx, GBool printing) {
  Object obj;

  // check the flags
  if ((flags & annotFlagHidden) ||
      (printing && !(flags & annotFlagPrint)) ||
      (!printing && (flags & annotFlagNoView))) {
    return;
  }

  addDingbatsResource = gFalse;
  generateFieldAppearance ();

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  if (addDingbatsResource) {
    // We are forcing ZaDb but the font does not exist
    // so create a fake one
    Object baseFontObj, subtypeObj;
    baseFontObj.initName("ZapfDingbats");
    subtypeObj.initName("Type1");

    Object fontDictObj;
    Dict *fontDict = new Dict(xref);
    fontDict->decRef();
    fontDict->add(copyString("BaseFont"), &baseFontObj);
    fontDict->add(copyString("Subtype"), &subtypeObj);
    fontDictObj.initDict(fontDict);

    Object fontsDictObj;
    Dict *fontsDict = new Dict(xref);
    fontsDict->decRef();
    fontsDict->add(copyString("ZaDb"), &fontDictObj);
    fontsDictObj.initDict(fontsDict);

    Dict *dict = new Dict(xref);
    dict->add(copyString("Font"), &fontsDictObj);
    gfx->pushResources(dict);
    delete dict;
  }
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  if (addDingbatsResource) {
    gfx->popResources();
  }
  obj.free();
}


//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------
AnnotMovie::AnnotMovie(XRef *xrefA, PDFRectangle *rect, Movie *movieA, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeMovie;
  annotObj.dictSet ("Subtype", obj1.initName ("Movie"));

  movie = movieA->copy();
  // TODO: create movie dict from movieA

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotMovie::AnnotMovie(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = typeMovie;
  initialize(xrefA, catalog, dict);

  movie = new Movie();
  movie->parseAnnotMovie(this);
}

AnnotMovie::~AnnotMovie() {
  if (title)
    delete title;
  if (fileName)
    delete fileName;
  delete movie;

  if (posterStream && (!posterStream->decRef())) {
    delete posterStream;
  }
}

void AnnotMovie::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("T", &obj1)->isString()) {
    title = obj1.getString()->copy();
  } else {
    title = NULL;
  }
  obj1.free();

  Object movieDict;
  Object aDict;

  // default values
  fileName = NULL;
  width = 0;
  height = 0;
  rotationAngle = 0;
  rate = 1.0;
  volume = 1.0;
  showControls = false;
  repeatMode = repeatModeOnce;
  synchronousPlay = false;
  
  hasFloatingWindow = false;
  isFullscreen = false;
  FWScaleNum = 1;
  FWScaleDenum = 1;
  FWPosX = 0.5;
  FWPosY = 0.5;

  if (dict->lookup("Movie", &movieDict)->isDict()) {
    Object obj2;
    if (getFileSpecNameForPlatform(movieDict.dictLookup("F", &obj1), &obj2)) {
      fileName = obj2.getString()->copy();
      obj2.free();
    }
    obj1.free();

    if (movieDict.dictLookup("Aspect", &obj1)->isArray()) {
      Array* aspect = obj1.getArray();
      if (aspect->getLength() >= 2) {
	Object tmp;
	if( aspect->get(0, &tmp)->isNum() ) {
	  width = (int)floor( aspect->get(0, &tmp)->getNum() + 0.5 );
	}
	tmp.free();
	if( aspect->get(1, &tmp)->isNum() ) {
	  height = (int)floor( aspect->get(1, &tmp)->getNum() + 0.5 );
	}
	tmp.free();
      }
    }
    obj1.free();

    if (movieDict.dictLookup("Rotate", &obj1)->isInt()) {
      // round up to 90°
      rotationAngle = (((obj1.getInt() + 360) % 360) % 90) * 90;
    }
    obj1.free();

    //
    // movie poster
    //
    posterType = posterTypeNone;
    posterStream = NULL;
    if (!movieDict.dictLookup("Poster", &obj1)->isNone()) {
      if (obj1.isBool()) {
	GBool v = obj1.getBool();
	if (v)
	  posterType = posterTypeFromMovie;
      }
      
      if (obj1.isStream()) {
	posterType = posterTypeStream;
	
	// "copy" stream
	posterStream = obj1.getStream();
	posterStream->incRef();
      }

      obj1.free();
    }

  }
  movieDict.free();


  // activation dictionary parsing ...

  if (dict->lookup("A", &aDict)->isDict()) {
    if (!aDict.dictLookup("Start", &obj1)->isNone()) {
      if (obj1.isInt()) {
	// If it is representable as an integer (subject to the implementation limit for
	// integers, as described in Appendix C), it should be specified as such.

	start.units = obj1.getInt();
      }
      if (obj1.isString()) {
	// If it is not representable as an integer, it should be specified as an 8-byte
	// string representing a 64-bit twos-complement integer, most significant
	// byte first.

	// UNSUPPORTED
      }

      if (obj1.isArray()) {
	Array* a = obj1.getArray();

	Object tmp;
	a->get(0, &tmp);
	if (tmp.isInt()) {
	  start.units = tmp.getInt();
	}
	if (tmp.isString()) {
	  // UNSUPPORTED
	}
	tmp.free();

	a->get(1, &tmp);
	if (tmp.isInt()) {
	  start.units_per_second = tmp.getInt();
	}
	tmp.free();
      }
    }
    obj1.free();

    if (!aDict.dictLookup("Duration", &obj1)->isNone()) {
      if (obj1.isInt()) {
	duration.units = obj1.getInt();
      }
      if (obj1.isString()) {
	// UNSUPPORTED
      }

      if (obj1.isArray()) {
	Array* a = obj1.getArray();

	Object tmp;
	a->get(0, &tmp);
	if (tmp.isInt()) {
	  duration.units = tmp.getInt();
	}
	if (tmp.isString()) {
	  // UNSUPPORTED
	}
	tmp.free();

	a->get(1, &tmp);
	if (tmp.isInt()) {
	  duration.units_per_second = tmp.getInt();
	}
	tmp.free();
      }
    }
    obj1.free();

    if (aDict.dictLookup("Rate", &obj1)->isNum()) {
      rate = obj1.getNum();
    }
    obj1.free();

    if (aDict.dictLookup("Volume", &obj1)->isNum()) {
      volume = obj1.getNum();
    }
    obj1.free();

    if (aDict.dictLookup("ShowControls", &obj1)->isBool()) {
      showControls = obj1.getBool();
    }
    obj1.free();

    if (aDict.dictLookup("Synchronous", &obj1)->isBool()) {
      synchronousPlay = obj1.getBool();
    }
    obj1.free();

    if (aDict.dictLookup("Mode", &obj1)->isName()) {
      char* name = obj1.getName();
      if (!strcmp(name, "Once"))
	repeatMode = repeatModeOnce;
      if (!strcmp(name, "Open"))
	repeatMode = repeatModeOpen;
      if (!strcmp(name, "Repeat"))
	repeatMode = repeatModeRepeat;
      if (!strcmp(name,"Palindrome"))
	repeatMode = repeatModePalindrome;
    }
    obj1.free();

    if (aDict.dictLookup("FWScale", &obj1)->isArray()) {
      // the presence of that entry implies that the movie is to be played
      // in a floating window
      hasFloatingWindow = true;

      Array* scale = obj1.getArray();
      if (scale->getLength() >= 2) {
	Object tmp;
	if (scale->get(0, &tmp)->isInt()) {
	  FWScaleNum = tmp.getInt();
	}
	tmp.free();
	if (scale->get(1, &tmp)->isInt()) {
	  FWScaleDenum = tmp.getInt();
	}
	tmp.free();
      }

      // detect fullscreen mode
      if ((FWScaleNum == 999) && (FWScaleDenum == 1)) {
	isFullscreen = true;
      }
    }
    obj1.free();

    if (aDict.dictLookup("FWPosition", &obj1)->isArray()) {
      Array* pos = obj1.getArray();
      if (pos->getLength() >= 2) {
	Object tmp;
	if (pos->get(0, &tmp)->isNum()) {
	  FWPosX = tmp.getNum();
	}
	tmp.free();
	if (pos->get(1, &tmp)->isNum()) {
	  FWPosY = tmp.getNum();
	}
	tmp.free();
      }
    }
  }
  aDict.free();
}

void AnnotMovie::getMovieSize(int& width, int& height) {
  width = this->width;
  height = this->height;
}

void AnnotMovie::getZoomFactor(int& num, int& denum) {
  num = FWScaleNum;
  denum = FWScaleDenum;
}


//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------
AnnotScreen::AnnotScreen(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeScreen;

  annotObj.dictSet ("Subtype", obj1.initName ("Screen"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotScreen::AnnotScreen(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = typeScreen;
  initialize(xrefA, catalog, dict);
}

AnnotScreen::~AnnotScreen() {
  if (title)
    delete title;
  if (appearCharacs)
    delete appearCharacs;

  action.free();
  additionAction.free();
}

void AnnotScreen::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  title = NULL;
  if (dict->lookup("T", &obj1)->isString()) {
    title = obj1.getString()->copy();
  }
  obj1.free();

  dict->lookup("A", &action);

  dict->lookup("AA", &additionAction);

  appearCharacs = NULL;
  if(dict->lookup("MK", &obj1)->isDict()) {
    appearCharacs = new AnnotAppearanceCharacs(obj1.getDict());
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotStamp
//------------------------------------------------------------------------
AnnotStamp::AnnotStamp(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
  AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeStamp;
  annotObj.dictSet ("Subtype", obj1.initName ("Stamp"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotStamp::AnnotStamp(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeStamp;
  initialize(xrefA, catalog, dict);
}

AnnotStamp::~AnnotStamp() {
  delete icon;
}

void AnnotStamp::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("Name", &obj1)->isName()) {
    icon = new GooString(obj1.getName());
  } else {
    icon = new GooString("Draft");
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotGeometry
//------------------------------------------------------------------------
AnnotGeometry::AnnotGeometry(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  switch (subType) {
    case typeSquare:
      annotObj.dictSet ("Subtype", obj1.initName ("Square"));
      break;
    case typeCircle:
      annotObj.dictSet ("Subtype", obj1.initName ("Circle"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotGeometry\n");
  }

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotGeometry::AnnotGeometry(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typeSquare;
  initialize(xrefA, catalog, dict);
}

AnnotGeometry::~AnnotGeometry() {
  delete interiorColor;
  delete borderEffect;
  delete geometryRect;
}

void AnnotGeometry::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Square")) {
      type = typeSquare;
    } else if (!typeName.cmp("Circle")) {
      type = typeCircle;
    }
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  geometryRect = NULL;
  if (dict->lookup("RD", &obj1)->isArray()) {
    geometryRect = parseDiffRectangle(obj1.getArray(), rect);
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotPolygon
//------------------------------------------------------------------------
AnnotPolygon::AnnotPolygon(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType,
			   AnnotPath *path, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  switch (subType) {
    case typePolygon:
      annotObj.dictSet ("Subtype", obj1.initName ("Polygon"));
      break;
    case typePolyLine:
      annotObj.dictSet ("Subtype", obj1.initName ("PolyLine"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotGeometry\n");
  }

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < path->getCoordsLength(); ++i) {
    Object obj3;

    obj2.arrayAdd (obj3.initReal (path->getX(i)));
    obj2.arrayAdd (obj3.initReal (path->getY(i)));
  }

  annotObj.dictSet ("Vertices", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotPolygon::AnnotPolygon(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typePolygon;
  initialize(xrefA, catalog, dict);
}

AnnotPolygon::~AnnotPolygon() {
  delete vertices;

  if (interiorColor)
    delete interiorColor;

  if (borderEffect)
    delete borderEffect;
}

void AnnotPolygon::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Polygon")) {
      type = typePolygon;
    } else if (!typeName.cmp("PolyLine")) {
      type = typePolyLine;
    }
  }
  obj1.free();

  if (dict->lookup("Vertices", &obj1)->isArray()) {
    vertices = new AnnotPath(obj1.getArray());
  } else {
    vertices = new AnnotPath();
    error(-1, "Bad Annot Polygon Vertices");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;

    if(obj1.arrayGet(0, &obj2)->isString())
      startStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      startStyle = annotLineEndingNone;
    obj2.free();

    if(obj1.arrayGet(1, &obj2)->isString())
      endStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      endStyle = annotLineEndingNone;
    obj2.free();

  } else {
    startStyle = endStyle = annotLineEndingNone;
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    GooString *intentName = new GooString(obj1.getName());

    if(!intentName->cmp("PolygonCloud")) {
      intent = polygonCloud;
    } else if(!intentName->cmp("PolyLineDimension")) {
      intent = polylineDimension;
    } else {
      intent = polygonDimension;
    }
    delete intentName;
  } else {
    intent = polygonCloud;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotCaret
//------------------------------------------------------------------------
AnnotCaret::AnnotCaret(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeCaret;

  annotObj.dictSet ("Subtype", obj1.initName ("Caret"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotCaret::AnnotCaret(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeCaret;
  initialize(xrefA, catalog, dict);
}

AnnotCaret::~AnnotCaret() {
  delete caretRect;
}

void AnnotCaret::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  symbol = symbolNone;
  if (dict->lookup("Sy", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("P")) {
      symbol = symbolP;
    } else if (!typeName.cmp("None")) {
      symbol = symbolNone;
    }
  }
  obj1.free();

  if (dict->lookup("RD", &obj1)->isArray()) {
    caretRect = parseDiffRectangle(obj1.getArray(), rect);
  } else {
    caretRect = NULL;
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotInk
//------------------------------------------------------------------------
AnnotInk::AnnotInk(XRef *xrefA, PDFRectangle *rect, AnnotPath **paths, int n_paths, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeInk;

  annotObj.dictSet ("Subtype", obj1.initName ("Ink"));

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < n_paths; ++i) {
    AnnotPath *path = paths[i];
    Object obj3;
    obj3.initArray (xrefA);

    for (int j = 0; j < path->getCoordsLength(); ++j) {
      Object obj4;

      obj3.arrayAdd (obj4.initReal (path->getX(j)));
      obj3.arrayAdd (obj4.initReal (path->getY(j)));
    }

    obj2.arrayAdd (&obj3);
  }

  annotObj.dictSet ("InkList", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotInk::AnnotInk(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeInk;
  initialize(xrefA, catalog, dict);
}

AnnotInk::~AnnotInk() {
  if (inkList) {
    for (int i = 0; i < inkListLength; ++i)
      delete inkList[i];
    gfree(inkList);
  }
}

void AnnotInk::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("InkList", &obj1)->isArray()) {
    Array *array = obj1.getArray();
    inkListLength = array->getLength();
    inkList = (AnnotPath **) gmallocn ((inkListLength), sizeof(AnnotPath *));
    memset(inkList, 0, inkListLength * sizeof(AnnotPath *));
    for (int i = 0; i < inkListLength; i++) {
      Object obj2;
      if (array->get(i, &obj2)->isArray())
        inkList[i] = new AnnotPath(obj2.getArray());
      obj2.free();
    }
  } else {
    inkListLength = 0;
    inkList = NULL;
    error(-1, "Bad Annot Ink List");
    ok = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotFileAttachment
//------------------------------------------------------------------------
AnnotFileAttachment::AnnotFileAttachment(XRef *xrefA, PDFRectangle *rect, GooString *filename, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeFileAttachment;

  annotObj.dictSet ("Subtype", obj1.initName ("FileAttachment"));

  Object obj2;
  obj2.initString(filename->copy());
  annotObj.dictSet ("FS", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotFileAttachment::AnnotFileAttachment(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeFileAttachment;
  initialize(xrefA, catalog, dict);
}

AnnotFileAttachment::~AnnotFileAttachment() {
  file.free();

  if (name)
    delete name;
}

void AnnotFileAttachment::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("FS", &obj1)->isDict() || dict->lookup("FS", &obj1)->isString()) {
    obj1.copy(&file);
  } else {
    error(-1, "Bad Annot File Attachment");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    name = new GooString(obj1.getName());
  } else {
    name = new GooString("PushPin");
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotSound
//------------------------------------------------------------------------
AnnotSound::AnnotSound(XRef *xrefA, PDFRectangle *rect, Sound *soundA, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeSound;

  annotObj.dictSet ("Subtype", obj1.initName ("Sound"));

  Object obj2;
  Stream *str = soundA->getStream();
  obj2.initStream (str);
  str->incRef(); //FIXME: initStream should do this?
  annotObj.dictSet ("Sound", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotSound::AnnotSound(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeSound;
  initialize(xrefA, catalog, dict);
}

AnnotSound::~AnnotSound() {
  delete sound;

  delete name;
}

void AnnotSound::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  sound = Sound::parseSound(dict->lookup("Sound", &obj1));
  if (!sound) {
    error(-1, "Bad Annot Sound");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    name = new GooString(obj1.getName());
  } else {
    name = new GooString("Speaker");
  }
  obj1.free();
}

//------------------------------------------------------------------------
// Annot3D
//------------------------------------------------------------------------
Annot3D::Annot3D(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = type3D;

  annotObj.dictSet ("Subtype", obj1.initName ("3D"));

  initialize(xrefA, catalog, annotObj.getDict());
}

Annot3D::Annot3D(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = type3D;
  initialize(xrefA, catalog, dict);
}

Annot3D::~Annot3D() {
  if (activation)
    delete activation;
}

void Annot3D::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("3DA", &obj1)->isDict()) {
    activation = new Activation(obj1.getDict());
  } else {
    activation = NULL;
  }
  obj1.free();
}

Annot3D::Activation::Activation(Dict *dict) {
  Object obj1;

  if (dict->lookup("A", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("PO")) {
      aTrigger = aTriggerPageOpened;
    } else if(!name->cmp("PV")) {
      aTrigger = aTriggerPageVisible;
    } else if(!name->cmp("XA")) {
      aTrigger = aTriggerUserAction;
    } else {
      aTrigger = aTriggerUnknown;
    }
    delete name;
  } else {
    aTrigger = aTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("AIS", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("I")) {
      aState = aStateEnabled;
    } else if(!name->cmp("L")) {
      aState = aStateDisabled;
    } else {
      aState = aStateUnknown;
    }
    delete name;
  } else {
    aState = aStateUnknown;
  }
  obj1.free();

  if(dict->lookup("D", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("PC")) {
      dTrigger = dTriggerPageClosed;
    } else if(!name->cmp("PI")) {
      dTrigger = dTriggerPageInvisible;
    } else if(!name->cmp("XD")) {
      dTrigger = dTriggerUserAction;
    } else {
      dTrigger = dTriggerUnknown;
    }
    delete name;
  } else {
    dTrigger = dTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("DIS", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("U")) {
      dState = dStateUninstantiaded;
    } else if(!name->cmp("I")) {
      dState = dStateInstantiated;
    } else if(!name->cmp("L")) {
      dState = dStateLive;
    } else {
      dState = dStateUnknown;
    }
    delete name;
  } else {
    dState = dStateUnknown;
  }
  obj1.free();

  if (dict->lookup("TB", &obj1)->isBool()) {
    displayToolbar = obj1.getBool();
  } else {
    displayToolbar = gTrue;
  }
  obj1.free();

  if (dict->lookup("NP", &obj1)->isBool()) {
    displayNavigation = obj1.getBool();
  } else {
    displayNavigation = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(XRef *xref, Catalog *catalog, Object *annotsObj) {
  Annot *annot;
  Object obj1;
  int size;
  int i;

  annots = NULL;
  size = 0;
  nAnnots = 0;

  if (annotsObj->isArray()) {
    for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
      //get the Ref to this annot and pass it to Annot constructor 
      //this way, it'll be possible for the annot to retrieve the corresponding
      //form widget
      Object obj2;
      if (annotsObj->arrayGet(i, &obj1)->isDict()) {
        annotsObj->arrayGetNF(i, &obj2);
        annot = createAnnot (xref, obj1.getDict(), catalog, &obj2);
        if (annot && annot->isOk()) {
          if (nAnnots >= size) {
            size += 16;
            annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
          }
          annots[nAnnots++] = annot;
        } else {
          delete annot;
        }
      }
      obj2.free();
      obj1.free();
    }
  }
}

Annot *Annots::createAnnot(XRef *xref, Dict* dict, Catalog *catalog, Object *obj) {
  Annot *annot;
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Text")) {
      annot = new AnnotText(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Link")) {
      annot = new AnnotLink(xref, dict, catalog, obj);
    } else if (!typeName->cmp("FreeText")) {
      annot = new AnnotFreeText(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Line")) {
      annot = new AnnotLine(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Square")) {
      annot = new AnnotGeometry(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Circle")) {
      annot = new AnnotGeometry(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Polygon")) {
      annot = new AnnotPolygon(xref, dict, catalog, obj);
    } else if (!typeName->cmp("PolyLine")) {
      annot = new AnnotPolygon(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Highlight")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Underline")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Squiggly")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("StrikeOut")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Stamp")) {
      annot = new AnnotStamp(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Caret")) {
      annot = new AnnotCaret(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Ink")) {
      annot = new AnnotInk(xref, dict, catalog, obj);
    } else if (!typeName->cmp("FileAttachment")) {
      annot = new AnnotFileAttachment(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Sound")) {
      annot = new AnnotSound(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Movie")) {
      annot = new AnnotMovie(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Widget")) {
      annot = new AnnotWidget(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Screen")) {
      annot = new AnnotScreen(xref, dict, catalog, obj);
    } else if(!typeName->cmp("PrinterMark")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("TrapNet")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Watermark")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("3D")) {
      annot = new Annot3D(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Popup")) {
      /* Popup annots are already handled by markup annots
       * Here we only care about popup annots without a
       * markup annotation associated
       */
      Object obj2;

      if (dict->lookup("Parent", &obj2)->isNull())
        annot = new AnnotPopup(xref, dict, catalog, obj);
      else
        annot = NULL;
      
      obj2.free();
    } else {
      annot = new Annot(xref, dict, catalog, obj);
    }

    delete typeName;
  } else {
    annot = NULL;
  }
  obj1.free();

  return annot;
}

Annot *Annots::findAnnot(Ref *ref) {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    if (annots[i]->match(ref)) {
      return annots[i];
    }
  }
  return NULL;
}


Annots::~Annots() {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    delete annots[i];
  }
  gfree(annots);
}
