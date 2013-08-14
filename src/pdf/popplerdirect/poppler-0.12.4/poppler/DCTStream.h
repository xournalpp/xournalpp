//========================================================================
//
// DCTStream.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright 2005-2007, 2009 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef DCTSTREAM_H
#define DCTSTREAM_H
#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif


#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "poppler-config.h"
#include "Error.h"
#include "Object.h"
#include "Decrypt.h"
#include "Stream.h"

extern "C" {
#include <jpeglib.h>
}

struct str_src_mgr {
    struct jpeg_source_mgr pub;
    JOCTET buffer;
    Stream *str;
    int index;
    bool abort;
};


class DCTStream: public FilterStream {
public:

  DCTStream(Stream *strA, int colorXformA);
  virtual ~DCTStream();
  virtual StreamKind getKind() { return strDCT; }
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  Stream *getRawStream() { return str; }

private:
  void init();

  JSAMPLE *current;
  JSAMPLE *limit;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  struct str_src_mgr src;
  JSAMPARRAY row_buffer;
};

#endif 
