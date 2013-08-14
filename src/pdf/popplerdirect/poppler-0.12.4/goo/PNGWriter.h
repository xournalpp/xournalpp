//========================================================================
//
// PNGWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <config.h>

#ifdef ENABLE_LIBPNG

#include <cstdio>
#include <png.h>

class PNGWriter
{
	public:
		PNGWriter();
		~PNGWriter();
		
		bool init(FILE *f, int width, int height);
		
		bool writePointers(png_bytep *rowPointers);
		bool writeRow(png_bytep *row);
		
		bool close();
	
	private:
		png_structp png_ptr;
		png_infop info_ptr;
};

#endif

#endif