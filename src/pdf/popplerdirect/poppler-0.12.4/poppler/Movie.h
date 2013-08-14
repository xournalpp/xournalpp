//*********************************************************************************
//                               Movie.h
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//*********************************************************************************

#ifndef _MOVIE_H_
#define _MOVIE_H_

#include "Object.h"
#include "Annot.h"

class GooList;


struct MovieWindowParameters {

  MovieWindowParameters();
  ~MovieWindowParameters();

  // parse from a floating window parameters dictionary
  void parseFWParams(Object* obj);

  enum MovieWindowType {
    movieWindowFloating = 0,
    movieWindowFullscreen,
    movieWindowHidden,  // ?
    movieWindowEmbedded
  };

  enum MovieWindowRelativeTo {
    windowRelativeToDocument = 0,
    windowRelativeToApplication,
    windowRelativeToDesktop
  };


                                         // DEFAULT VALUE

  MovieWindowType type;                  // movieWindowEmbedded
  

  int width;                             // -1
  int height;                            // -1
  
  // floating window position
  MovieWindowRelativeTo relativeTo;      // windowRelativeToDocument (or to desktop)
  double XPosition;                      // 0.5
  double YPosition;                      // 0.5

  GBool hasTitleBar;                      // true
  GBool hasCloseButton;                   // true
  GBool isResizeable;                     // true
};


struct MovieParameters {

  MovieParameters();
  ~MovieParameters();

  // parse from a "Media Play Parameters" dictionary
  void parseMediaPlayParameters(Object* playObj);
  // parse from a "Media Screen Parameters" dictionary
  void parseMediaScreenParameters(Object* screenObj);
  // parse from a AnnotMovie object
  void parseAnnotMovie(AnnotMovie* annot);

  enum MovieFittingPolicy {
    fittingMeet = 0,
    fittingSlice,
    fittingFill,
    fittingScroll,
    fittingHidden,
    fittingUndefined
  };

  struct MovieTime {
    MovieTime() { units_per_second = 0; }
    Gulong units;
    int units_per_second; // 0 : defined by movie
  };
  
  struct Color {
    double r, g, b;
  };

  Gushort rotationAngle;                   // 0

  MovieTime start;                         // 0
  MovieTime duration;                      // 0

  double rate;                             // 1.0

  int volume;                              // 100

  // defined in media play parameters, p 770
  // correspond to 'fit' SMIL's attribute
  MovieFittingPolicy fittingPolicy;        // fittingUndefined

  GBool autoPlay;                          // true

  // repeat count, can be real values, 0 means forever
  double repeatCount;                      // 1.0

  // background color                      // black = (0.0 0.0 0.0)
  Color bgColor;
  
  // opacity in [0.0 1.0]
  double opacity;                          // 1.0
  

  GBool showControls;                      // false

  MovieWindowParameters windowParams;
};

class Movie {
 public:
  Movie();
  ~Movie();

  void parseAnnotMovie(AnnotMovie* annot);
  void parseMediaRendition(Object* obj);

  MovieParameters* getMHParameters() { return &MH; }
  MovieParameters* getBEParameters() { return &BE; }

  GooString* getContentType() { return contentType; }
  GooString* getFileName() { return fileName; }

  GBool getIsEmbedded() { return isEmbedded; }
  Stream* getEmbbededStream() { return embeddedStream; }
  // write embedded stream to file
  void outputToFile(FILE*);

  Stream* getPosterStream() { return posterStream; }

  Movie* copy();

 private:
  // "Must Honor" parameters
  MovieParameters MH;
  // "Best Effort" parameters
  MovieParameters BE;

  Stream* posterStream;

  GBool isEmbedded;

  GooString* contentType;

  // if it's embedded
  Stream* embeddedStream;

  // if it's not embedded
  GooString* fileName;
};

#endif

