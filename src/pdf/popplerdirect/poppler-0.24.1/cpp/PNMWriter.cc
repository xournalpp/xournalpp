//========================================================================
//
// PNMWriter.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011 Pino Toscano <pino@kde.org>
//
//========================================================================

#include "PNMWriter.h"

#include <vector>

using namespace poppler;

PNMWriter::PNMWriter(OutFormat formatArg)
  : format(formatArg)
  , file(0)
  , imgWidth(0)
  , rowSize(0)
{
}

PNMWriter::~PNMWriter()
{
}

bool PNMWriter::init(FILE *f, int width, int height, int /*hDPI*/, int /*vDPI*/)
{
  file = f;
  imgWidth = width;

  switch (format)
  {
    case PNMWriter::PBM:
      fprintf(file, "P4\n%d %d\n", width, height);
      rowSize = (width + 7) >> 3;
      break;
    case PNMWriter::PGM:
      fprintf(file, "P5\n%d %d\n255\n", width, height);
      rowSize = width;
      break;
    case PNMWriter::PPM:
      fprintf(file, "P6\n%d %d\n255\n", width, height);
      rowSize = width * 3;
      break;
  }

  return true;
}

bool PNMWriter::writePointers(unsigned char **rowPointers, int rowCount)
{
  bool ret = true;
  for (int i = 0; ret && (i < rowCount); ++i) {
    ret = writeRow(&(rowPointers[i]));
  }

  return ret;
}

bool PNMWriter::writeRow(unsigned char **row)
{
  std::vector<unsigned char> newRow;
  unsigned char *rowPtr = *row;
  unsigned char *p = *row;

  switch (format)
  {
    case PNMWriter::PBM:
      newRow.resize(rowSize, 0);
      rowPtr = &newRow[0];
      for (int i = 0; i < imgWidth; ++i) {
        unsigned char pixel = p[0];
        if (p[0] == p[1] && p[1] == p[2]) {
          // gray, stored already
        } else {
          pixel = static_cast<unsigned char>((p[0] * 11 + p[1] * 16 + p[2] * 5) / 32);
        }
        if (pixel < 0x7F) {
          *(rowPtr + (i >> 3)) |= (1 << (i & 7));
        }
        p += 3;
      }
      break;
    case PNMWriter::PGM:
      newRow.resize(rowSize, 0);
      rowPtr = &newRow[0];
      for (int i = 0; i < imgWidth; ++i) {
        if (p[0] == p[1] && p[1] == p[2]) {
          // gray, store directly
          newRow[i] = p[0];
        } else {
          // calculate the gray value
          newRow[i] = static_cast<unsigned char>((p[0] * 11 + p[1] * 16 + p[2] * 5) / 32);
        }
        p += 3;
      }
      break;
    case PNMWriter::PPM:
      break;
  }

  if (int(fwrite(rowPtr, 1, rowSize, file)) < rowSize) {
    return false;
  }

  return true;
}

bool PNMWriter::close()
{
  file = 0;
  imgWidth = 0;
  rowSize = 0;

  return true;
}
