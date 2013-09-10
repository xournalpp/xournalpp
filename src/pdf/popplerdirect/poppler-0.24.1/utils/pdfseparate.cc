//========================================================================
//
// pdfseparate.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011, 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
//
//========================================================================
#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "parseargs.h"
#include "goo/GooString.h"
#include "PDFDoc.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"

static int firstPage = 0;
static int lastPage = 0;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-f", argInt, &firstPage, 0,
   "first page to extract"},
  {"-l", argInt, &lastPage, 0,
   "last page to extract"},
  {"-v", argFlag, &printVersion, 0,
   "print copyright and version info"},
  {"-h", argFlag, &printHelp, 0,
   "print usage information"},
  {"-help", argFlag, &printHelp, 0,
   "print usage information"},
  {"--help", argFlag, &printHelp, 0,
   "print usage information"},
  {"-?", argFlag, &printHelp, 0,
   "print usage information"},
  {NULL}
};

bool extractPages (const char *srcFileName, const char *destFileName) {
  char pathName[1024];
  GooString *gfileName = new GooString (srcFileName);
  PDFDoc *doc = new PDFDoc (gfileName, NULL, NULL, NULL);

  if (!doc->isOk()) {
    error(errSyntaxError, -1, "Could not extract page(s) from damaged file ('{0:s}')", srcFileName);
    return false;
  }

  if (firstPage == 0 && lastPage == 0) {
    firstPage = 1;
    lastPage = doc->getNumPages();
  }
  if (lastPage == 0)
    lastPage = doc->getNumPages();
  if (firstPage == 0)
    firstPage = 1;
  if (firstPage != lastPage && strstr(destFileName, "%d") == NULL) {
    error(errSyntaxError, -1, "'{0:s}' must contain '%%d' if more than one page should be extracted", destFileName);
    return false;
  }
  for (int pageNo = firstPage; pageNo <= lastPage; pageNo++) {
    sprintf (pathName, destFileName, pageNo);
    GooString *gpageName = new GooString (pathName);
    int errCode = doc->savePageAs(gpageName, pageNo);
    if ( errCode != errNone) {
      delete gpageName;
      delete gfileName;
      return false;
    }
    delete gpageName;
  }
  delete gfileName;
  return true;
}

int
main (int argc, char *argv[])
{
  Object info;
  GBool ok;
  int exitCode;

  exitCode = 99;

  // parse args
  ok = parseArgs (argDesc, &argc, argv);
  if (!ok || argc != 3 || printVersion || printHelp)
    {
      fprintf (stderr, "pdfseparate version %s\n", PACKAGE_VERSION);
      fprintf (stderr, "%s\n", popplerCopyright);
      fprintf (stderr, "%s\n", xpdfCopyright);
      if (!printVersion)
	{
	  printUsage ("pdfseparate", "<PDF-sourcefile> <PDF-pattern-destfile>",
		      argDesc);
	}
      if (printVersion || printHelp)
	exitCode = 0;
      goto err0;
    }
  globalParams = new GlobalParams();
  ok = extractPages (argv[1], argv[2]);
  if (ok) {
    exitCode = 0;
  }
  delete globalParams;

err0:

  return exitCode;
}
