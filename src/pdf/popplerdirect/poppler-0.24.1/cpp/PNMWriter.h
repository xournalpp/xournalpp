//========================================================================
//
// PNMWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011 Pino Toscano <pino@kde.org>
//
//========================================================================

#ifndef PNMWRITER_H
#define PNMWRITER_H

#include "ImgWriter.h"

namespace poppler
{

class PNMWriter : public ImgWriter
{
  public:
    enum OutFormat { PBM, PGM, PPM };

    PNMWriter(OutFormat formatArg);
    ~PNMWriter();

    bool init(FILE *f, int width, int height, int hDPI, int vDPI);

    bool writePointers(unsigned char **rowPointers, int rowCount);
    bool writeRow(unsigned char **row);

    bool close();

  private:
    const OutFormat format;
    FILE *file;
    int imgWidth;
    int rowSize;
};

}

#endif
