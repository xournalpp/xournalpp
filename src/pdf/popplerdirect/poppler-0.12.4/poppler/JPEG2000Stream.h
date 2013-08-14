//========================================================================
//
// JPEG2000Stream.h
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008 Albert Astals Cid <aacid@kde.org>
//
// Licensed under GPLv2 or later
//
//========================================================================


#ifndef JPEG2000STREAM_H
#define JPEG2000STREAM_H

#include <openjpeg.h>

#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"

class JPXStream: public FilterStream {
public:

  JPXStream(Stream *strA);
  virtual ~JPXStream();
  virtual StreamKind getKind() { return strJPX; }
  virtual void reset();
  virtual void close();
  virtual int getPos();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual void getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode);

private:
  void init();
  void init2(unsigned char *buf, int bufLen, OPJ_CODEC_FORMAT format);

  opj_image_t *image;
  opj_dinfo_t *dinfo;
  int counter;
  GBool inited;
};

#endif
