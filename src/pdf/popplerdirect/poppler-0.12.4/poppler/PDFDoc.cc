//========================================================================
//
// PDFDoc.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005, 2007-2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Eric Toombs <ewtoombs@uwaterloo.ca>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2009 Axel Struebing <axel.struebing@freenet.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#  include <windows.h>
#endif
#include "goo/gstrtod.h"
#include "goo/GooString.h"
#include "poppler-config.h"
#include "GlobalParams.h"
#include "Page.h"
#include "Catalog.h"
#include "Stream.h"
#include "XRef.h"
#include "Link.h"
#include "OutputDev.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "Lexer.h"
#include "Parser.h"
#include "SecurityHandler.h"
#include "Decrypt.h"
#ifndef DISABLE_OUTLINE
#include "Outline.h"
#endif
#include "PDFDoc.h"

//------------------------------------------------------------------------

#define headerSearchSize 1024	// read this many bytes at beginning of
				//   file to look for '%PDF'

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

PDFDoc::PDFDoc(GooString *fileNameA, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
  Object obj;

  ok = gFalse;
  errCode = errNone;

  guiData = guiDataA;

  file = NULL;
  str = NULL;
  xref = NULL;
  catalog = NULL;
#ifndef DISABLE_OUTLINE
  outline = NULL;
#endif

  fileName = fileNameA;

  // try to open file
#ifdef VMS
  file = fopen(fileName->getCString(), "rb", "ctx=stm");
#else
  file = fopen(fileName->getCString(), "rb");
#endif
  if (file == NULL) {
    // fopen() has failed.
    // Keep a copy of the errno returned by fopen so that it can be 
    // referred to later.
    fopenErrno = errno;
    error(-1, "Couldn't open file '%s': %s.", fileName->getCString(),
                                              strerror(errno));
    errCode = errOpenFile;
    return;
  }

  // create stream
  obj.initNull();
  str = new FileStream(file, 0, gFalse, 0, &obj);

  ok = setup(ownerPassword, userPassword);
}

#ifdef _WIN32
PDFDoc::PDFDoc(wchar_t *fileNameA, int fileNameLen, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
  OSVERSIONINFO version;
  wchar_t fileName2[_MAX_PATH + 1];
  Object obj;
  int i;

  ok = gFalse;
  errCode = errNone;

  guiData = guiDataA;

  file = NULL;
  str = NULL;
  xref = NULL;
  catalog = NULL;
#ifndef DISABLE_OUTLINE
  outline = NULL;
#endif

  //~ file name should be stored in Unicode (?)
  fileName = new GooString();
  for (i = 0; i < fileNameLen; ++i) {
    fileName->append((char)fileNameA[i]);
  }

  // zero-terminate the file name string
  for (i = 0; i < fileNameLen && i < _MAX_PATH; ++i) {
    fileName2[i] = fileNameA[i];
  }
  fileName2[i] = 0;

  // try to open file
  // NB: _wfopen is only available in NT
  version.dwOSVersionInfoSize = sizeof(version);
  GetVersionEx(&version);
  if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    file = _wfopen(fileName2, L"rb");
  } else {
    file = fopen(fileName->getCString(), "rb");
  }
  if (!file) {
    error(-1, "Couldn't open file '%s'", fileName->getCString());
    errCode = errOpenFile;
    return;
  }

  // create stream
  obj.initNull();
  str = new FileStream(file, 0, gFalse, 0, &obj);

  ok = setup(ownerPassword, userPassword);
}
#endif

PDFDoc::PDFDoc(BaseStream *strA, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
  ok = gFalse;
  errCode = errNone;
  guiData = guiDataA;
  if (strA->getFileName()) {
    fileName = strA->getFileName()->copy();
  } else {
    fileName = NULL;
  }
  file = NULL;
  str = strA;
  xref = NULL;
  catalog = NULL;
#ifndef DISABLE_OUTLINE
  outline = NULL;
#endif
  ok = setup(ownerPassword, userPassword);
}

GBool PDFDoc::setup(GooString *ownerPassword, GooString *userPassword) {
  str->setPos(0, -1);
  if (str->getPos() < 0)
  {
    error(-1, "Document base stream is not seekable");
    return gFalse;
  }

  str->reset();

  // check footer
  // Adobe does not seem to enforce %%EOF, so we do the same
//  if (!checkFooter()) return gFalse;
  
  // check header
  checkHeader();

  // read xref table
  xref = new XRef(str);
  if (!xref->isOk()) {
    error(-1, "Couldn't read xref table");
    errCode = xref->getErrorCode();
    return gFalse;
  }

  // check for encryption
  if (!checkEncryption(ownerPassword, userPassword)) {
    errCode = errEncrypted;
    return gFalse;
  }

  // read catalog
  catalog = new Catalog(xref);
  if (!catalog->isOk()) {
    error(-1, "Couldn't read page catalog");
    errCode = errBadCatalog;
    return gFalse;
  }

#ifndef DISABLE_OUTLINE
  // read outline
  outline = new Outline(catalog->getOutline(), xref);
#endif

  // done
  return gTrue;
}

PDFDoc::~PDFDoc() {
#ifndef DISABLE_OUTLINE
  if (outline) {
    delete outline;
  }
#endif
  if (catalog) {
    delete catalog;
  }
  if (xref) {
    delete xref;
  }
  if (str) {
    delete str;
  }
  if (file) {
    fclose(file);
  }
  if (fileName) {
    delete fileName;
  }
}


// Check for a %%EOF at the end of this stream
GBool PDFDoc::checkFooter() {
  // we look in the last 1024 chars because Adobe does the same
  char *eof = new char[1025];
  int pos = str->getPos();
  str->setPos(1024, -1);
  int i, ch;
  for (i = 0; i < 1024; i++)
  {
    ch = str->getChar();
    if (ch == EOF)
      break;
    eof[i] = ch;
  }
  eof[i] = '\0';

  bool found = false;
  for (i = i - 5; i >= 0; i--) {
    if (strncmp (&eof[i], "%%EOF", 5) == 0) {
      found = true;
      break;
    }
  }
  if (!found)
  {
    error(-1, "Document has not the mandatory ending %%EOF");
    errCode = errDamaged;
    delete[] eof;
    return gFalse;
  }
  delete[] eof;
  str->setPos(pos);
  return gTrue;
}
  
// Check for a PDF header on this stream.  Skip past some garbage
// if necessary.
void PDFDoc::checkHeader() {
  char hdrBuf[headerSearchSize+1];
  char *p;
  int i;

  pdfMajorVersion = 0;
  pdfMinorVersion = 0;
  for (i = 0; i < headerSearchSize; ++i) {
    hdrBuf[i] = str->getChar();
  }
  hdrBuf[headerSearchSize] = '\0';
  for (i = 0; i < headerSearchSize - 5; ++i) {
    if (!strncmp(&hdrBuf[i], "%PDF-", 5)) {
      break;
    }
  }
  if (i >= headerSearchSize - 5) {
    error(-1, "May not be a PDF file (continuing anyway)");
    return;
  }
  str->moveStart(i);
  if (!(p = strtok(&hdrBuf[i+5], " \t\n\r"))) {
    error(-1, "May not be a PDF file (continuing anyway)");
    return;
  }
  sscanf(p, "%d.%d", &pdfMajorVersion, &pdfMinorVersion);
  // We don't do the version check. Don't add it back in.
}

GBool PDFDoc::checkEncryption(GooString *ownerPassword, GooString *userPassword) {
  Object encrypt;
  GBool encrypted;
  SecurityHandler *secHdlr;
  GBool ret;

  xref->getTrailerDict()->dictLookup("Encrypt", &encrypt);
  if ((encrypted = encrypt.isDict())) {
    if ((secHdlr = SecurityHandler::make(this, &encrypt))) {
      if (secHdlr->checkEncryption(ownerPassword, userPassword)) {
	// authorization succeeded
       	xref->setEncryption(secHdlr->getPermissionFlags(),
			    secHdlr->getOwnerPasswordOk(),
			    secHdlr->getFileKey(),
			    secHdlr->getFileKeyLength(),
			    secHdlr->getEncVersion(),
			    secHdlr->getEncRevision(),
			    secHdlr->getEncAlgorithm());
	ret = gTrue;
      } else {
	// authorization failed
	ret = gFalse;
      }
      delete secHdlr;
    } else {
      // couldn't find the matching security handler
      ret = gFalse;
    }
  } else {
    // document is not encrypted
    ret = gTrue;
  }
  encrypt.free();
  return ret;
}

void PDFDoc::displayPage(OutputDev *out, int page,
			 double hDPI, double vDPI, int rotate,
			 GBool useMediaBox, GBool crop, GBool printing,
			 GBool (*abortCheckCbk)(void *data),
			 void *abortCheckCbkData,
                         GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                         void *annotDisplayDecideCbkData) {
  if (globalParams->getPrintCommands()) {
    printf("***** page %d *****\n", page);
  }
  catalog->getPage(page)->display(out, hDPI, vDPI,
				  rotate, useMediaBox, crop, printing, catalog,
				  abortCheckCbk, abortCheckCbkData,
				  annotDisplayDecideCbk, annotDisplayDecideCbkData);
}

void PDFDoc::displayPages(OutputDev *out, int firstPage, int lastPage,
			  double hDPI, double vDPI, int rotate,
			  GBool useMediaBox, GBool crop, GBool printing,
			  GBool (*abortCheckCbk)(void *data),
			  void *abortCheckCbkData,
                          GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                          void *annotDisplayDecideCbkData) {
  int page;

  for (page = firstPage; page <= lastPage; ++page) {
    displayPage(out, page, hDPI, vDPI, rotate, useMediaBox, crop, printing,
		abortCheckCbk, abortCheckCbkData,
                annotDisplayDecideCbk, annotDisplayDecideCbkData);
  }
}

void PDFDoc::displayPageSlice(OutputDev *out, int page,
			      double hDPI, double vDPI, int rotate,
			      GBool useMediaBox, GBool crop, GBool printing,
			      int sliceX, int sliceY, int sliceW, int sliceH,
			      GBool (*abortCheckCbk)(void *data),
			      void *abortCheckCbkData,
                              GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                              void *annotDisplayDecideCbkData) {
  catalog->getPage(page)->displaySlice(out, hDPI, vDPI,
				       rotate, useMediaBox, crop,
				       sliceX, sliceY, sliceW, sliceH,
				       printing, catalog,
				       abortCheckCbk, abortCheckCbkData,
				       annotDisplayDecideCbk, annotDisplayDecideCbkData);
}

Links *PDFDoc::getLinks(int page) {
  return catalog->getPage(page)->getLinks(catalog);
}
  
void PDFDoc::processLinks(OutputDev *out, int page) {
  catalog->getPage(page)->processLinks(out, catalog);
}

GBool PDFDoc::isLinearized() {
  Parser *parser;
  Object obj1, obj2, obj3, obj4, obj5;
  GBool lin;

  lin = gFalse;
  obj1.initNull();
  parser = new Parser(xref,
	     new Lexer(xref,
	       str->makeSubStream(str->getStart(), gFalse, 0, &obj1)),
	     gTrue);
  parser->getObj(&obj1);
  parser->getObj(&obj2);
  parser->getObj(&obj3);
  parser->getObj(&obj4);
  if (obj1.isInt() && obj2.isInt() && obj3.isCmd("obj") &&
      obj4.isDict()) {
    obj4.dictLookup("Linearized", &obj5);
    if (obj5.isNum() && obj5.getNum() > 0) {
      lin = gTrue;
    }
    obj5.free();
  }
  obj4.free();
  obj3.free();
  obj2.free();
  obj1.free();
  delete parser;
  return lin;
}

int PDFDoc::saveAs(GooString *name, PDFWriteMode mode) {
  FILE *f;
  OutStream *outStr;
  int res;

  if (!(f = fopen(name->getCString(), "wb"))) {
    error(-1, "Couldn't open file '%s'", name->getCString());
    return errOpenFile;
  }
  outStr = new FileOutStream(f,0);
  res = saveAs(outStr, mode);
  delete outStr;
  fclose(f);
  return res;
}

int PDFDoc::saveAs(OutStream *outStr, PDFWriteMode mode) {

  // we don't support files with Encrypt at the moment
  Object obj;
  xref->getTrailerDict()->getDict()->lookupNF("Encrypt", &obj);
  if (!obj.isNull())
  {
    obj.free();
    return errEncrypted;
  }
  obj.free();

  if (mode == writeForceRewrite) {
    saveCompleteRewrite(outStr);
  } else if (mode == writeForceIncremental) {
    saveIncrementalUpdate(outStr); 
  } else { // let poppler decide
    // find if we have updated objects
    GBool updated = gFalse;
    for(int i=0; i<xref->getNumObjects(); i++) {
      if (xref->getEntry(i)->updated) {
        updated = gTrue;
        break;
      }
    }
    if(updated) { 
      saveIncrementalUpdate(outStr);
    } else {
      // simply copy the original file
      saveWithoutChangesAs (outStr);
    }
  }

  return errNone;
}

int PDFDoc::saveWithoutChangesAs(GooString *name) {
  FILE *f;
  OutStream *outStr;
  int res;

  if (!(f = fopen(name->getCString(), "wb"))) {
    error(-1, "Couldn't open file '%s'", name->getCString());
    return errOpenFile;
  }
  
  outStr = new FileOutStream(f,0);
  res = saveWithoutChangesAs(outStr);
  delete outStr;

  fclose(f);

  return res;
}

int PDFDoc::saveWithoutChangesAs(OutStream *outStr) {
  int c;
  
  str->reset();
  while ((c = str->getChar()) != EOF) {
    outStr->put(c);
  }
  str->close();

  return errNone;
}

void PDFDoc::saveIncrementalUpdate (OutStream* outStr)
{
  XRef *uxref;
  int c;
  //copy the original file
  str->reset();
  while ((c = str->getChar()) != EOF) {
    outStr->put(c);
  }
  str->close();

  uxref = new XRef();
  uxref->add(0, 65535, 0, gFalse);
  int objectsCount = 0; //count the number of objects in the XRef(s)
  for(int i=0; i<xref->getNumObjects(); i++) {
    if ((xref->getEntry(i)->type == xrefEntryFree) && 
        (xref->getEntry(i)->gen == 0)) //we skip the irrelevant free objects
      continue;
    objectsCount++;
    if (xref->getEntry(i)->updated) { //we have an updated object
      Object obj1;
      Ref ref;
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen;
      xref->fetch(ref.num, ref.gen, &obj1);
      Guint offset = writeObject(&obj1, &ref, outStr);
      uxref->add(ref.num, ref.gen, offset, gTrue);
      obj1.free();
    }
  }
  if (uxref->getSize() == 0) { //we have nothing to update
    delete uxref;
    return;
  }

  Guint uxrefOffset = outStr->getPos();
  uxref->writeToFile(outStr, gFalse /* do not write unnecessary entries */);

  writeTrailer(uxrefOffset, objectsCount, outStr, gTrue);

  delete uxref;
}

void PDFDoc::saveCompleteRewrite (OutStream* outStr)
{
  outStr->printf("%%PDF-%d.%d\r\n",pdfMajorVersion,pdfMinorVersion);
  XRef *uxref = new XRef();
  uxref->add(0, 65535, 0, gFalse);
  for(int i=0; i<xref->getNumObjects(); i++) {
    Object obj1;
    Ref ref;
    XRefEntryType type = xref->getEntry(i)->type;
    if (type == xrefEntryFree) {
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen;
      /* the XRef class adds a lot of irrelevant free entries, we only want the significant one
          and we don't want the one with num=0 because it has already been added (gen = 65535)*/
      if (ref.gen > 0 && ref.num > 0)
        uxref->add(ref.num, ref.gen, 0, gFalse);
    } else if (type == xrefEntryUncompressed){ 
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen;
      xref->fetch(ref.num, ref.gen, &obj1);
      Guint offset = writeObject(&obj1, &ref, outStr);
      uxref->add(ref.num, ref.gen, offset, gTrue);
      obj1.free();
    } else if (type == xrefEntryCompressed) {
      ref.num = i;
      ref.gen = 0; //compressed entries have gen == 0
      xref->fetch(ref.num, ref.gen, &obj1);
      Guint offset = writeObject(&obj1, &ref, outStr);
      uxref->add(ref.num, ref.gen, offset, gTrue);
      obj1.free();
    }
  }
  Guint uxrefOffset = outStr->getPos();
  uxref->writeToFile(outStr, gTrue /* write all entries */);

  writeTrailer(uxrefOffset, uxref->getSize(), outStr, gFalse);


  delete uxref;

}

void PDFDoc::writeDictionnary (Dict* dict, OutStream* outStr)
{
  Object obj1;
  outStr->printf("<<");
  for (int i=0; i<dict->getLength(); i++) {
    GooString keyName(dict->getKey(i));
    GooString *keyNameToPrint = keyName.sanitizedName(gFalse /* non ps mode */);
    outStr->printf("/%s ", keyNameToPrint->getCString());
    delete keyNameToPrint;
    writeObject(dict->getValNF(i, &obj1), NULL, outStr);
    obj1.free();
  }
  outStr->printf(">> ");
}

void PDFDoc::writeStream (Stream* str, OutStream* outStr)
{
  outStr->printf("stream\r\n");
  str->reset();
  for (int c=str->getChar(); c!= EOF; c=str->getChar()) {
    outStr->printf("%c", c);  
  }
  outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeRawStream (Stream* str, OutStream* outStr)
{
  Object obj1;
  str->getDict()->lookup("Length", &obj1);
  if (!obj1.isInt()) {
    error (-1, "PDFDoc::writeRawStream, no Length in stream dict");
    return;
  }

  const int length = obj1.getInt();
  obj1.free();

  outStr->printf("stream\r\n");
  str->unfilteredReset();
  for (int i=0; i<length; i++) {
    int c = str->getUnfilteredChar();
    outStr->printf("%c", c);  
  }
  str->reset();
  outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeString (GooString* s, OutStream* outStr)
{
  if (s->hasUnicodeMarker()) {
    //unicode string don't necessary end with \0
    const char* c = s->getCString();
    outStr->printf("(");
    for(int i=0; i<s->getLength(); i++) {
      char unescaped = *(c+i)&0x000000ff;
      //escape if needed
      if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
        outStr->printf("%c", '\\');
      outStr->printf("%c", unescaped);
    }
    outStr->printf(") ");
  } else {
    const char* c = s->getCString();
    outStr->printf("(");
    for(int i=0; i<s->getLength(); i++) {
      char unescaped = (*c)&0x000000ff;
      //escape if needed
      if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
        outStr->printf("%c", '\\');
      outStr->printf("%c", unescaped);
      c++;
    }
    outStr->printf(") ");
  }
}

Guint PDFDoc::writeObject (Object* obj, Ref* ref, OutStream* outStr)
{
  Array *array;
  Object obj1;
  Guint offset = outStr->getPos();
  int tmp;

  if(ref) 
    outStr->printf("%i %i obj ", ref->num, ref->gen);

  switch (obj->getType()) {
    case objBool:
      outStr->printf("%s ", obj->getBool()?"true":"false");
      break;
    case objInt:
      outStr->printf("%i ", obj->getInt());
      break;
    case objReal:
    {
      GooString s;
      s.appendf("{0:.10g}", obj->getReal());
      outStr->printf("%s ", s.getCString());
      break;
    }
    case objString:
      writeString(obj->getString(), outStr);
      break;
    case objName:
    {
      GooString name(obj->getName());
      GooString *nameToPrint = name.sanitizedName(gFalse /* non ps mode */);
      outStr->printf("/%s ", nameToPrint->getCString());
      delete nameToPrint;
      break;
    }
    case objNull:
      outStr->printf( "null ");
      break;
    case objArray:
      array = obj->getArray();
      outStr->printf("[");
      for (int i=0; i<array->getLength(); i++) {
        writeObject(array->getNF(i, &obj1), NULL,outStr);
        obj1.free();
      }
      outStr->printf("] ");
      break;
    case objDict:
      writeDictionnary (obj->getDict(),outStr);
      break;
    case objStream: 
      {
        //We can't modify stream with the current implementation (no write functions in Stream API)
        // => the only type of streams which that have been modified are internal streams (=strWeird)
        Stream *stream = obj->getStream();
        if (stream->getKind() == strWeird) {
          //we write the stream unencoded => TODO: write stream encoder
          stream->reset();
          //recalculate stream length
          tmp = 0;
          for (int c=stream->getChar(); c!=EOF; c=stream->getChar()) {
            tmp++;
          }
          obj1.initInt(tmp);
          stream->getDict()->set("Length", &obj1);

          //Remove Stream encoding
          stream->getDict()->remove("Filter");
          stream->getDict()->remove("DecodeParms");

          writeDictionnary (stream->getDict(),outStr);
          writeStream (stream,outStr);
          obj1.free();
        } else {
          //raw stream copy
          FilterStream *fs = dynamic_cast<FilterStream*>(stream);
          if (fs) {
            BaseStream *bs = fs->getBaseStream();
            if (bs) {
              Guint streamEnd;
                if (xref->getStreamEnd(bs->getStart(), &streamEnd)) {
                  Object val;
                  val.initInt(streamEnd - bs->getStart());
                  stream->getDict()->set("Length", &val);
                }
              }
          }
          writeDictionnary (stream->getDict(), outStr);
          writeRawStream (stream, outStr);
        }
        break;
      }
    case objRef:
      outStr->printf("%i %i R ", obj->getRef().num, obj->getRef().gen);
      break;
    case objCmd:
      outStr->printf("cmd\r\n");
      break;
    case objError:
      outStr->printf("error\r\n");
      break;
    case objEOF:
      outStr->printf("eof\r\n");
      break;
    case objNone:
      outStr->printf("none\r\n");
      break;
    default:
      error(-1,"Unhandled objType : %i, please report a bug with a testcase\r\n", obj->getType());
      break;
  }
  if (ref)
    outStr->printf("endobj\r\n");
  return offset;
}

void PDFDoc::writeTrailer (Guint uxrefOffset, int uxrefSize, OutStream* outStr, GBool incrUpdate)
{
  Dict *trailerDict = new Dict(xref);
  Object obj1;
  obj1.initInt(uxrefSize);
  trailerDict->set("Size", &obj1);
  obj1.free();


  //build a new ID, as recommended in the reference, uses:
  // - current time
  // - file name
  // - file size
  // - values of entry in information dictionnary
  GooString message;
  char buffer[256];
  sprintf(buffer, "%i", (int)time(NULL));
  message.append(buffer);
  if (fileName)
    message.append(fileName);
  else
    message.append("streamwithoutfilename.pdf");
  // file size
  unsigned int fileSize = 0;
  int c;
  str->reset();
  while ((c = str->getChar()) != EOF) {
    fileSize++;
  }
  str->close();
  sprintf(buffer, "%i", fileSize);
  message.append(buffer);

  //info dict -- only use text string
  if (xref->getDocInfo(&obj1)->isDict()) {
    for(int i=0; i<obj1.getDict()->getLength(); i++) {
      Object obj2;
      obj1.getDict()->getVal(i, &obj2);  
      if (obj2.isString()) {
        message.append(obj2.getString());
      }
      obj2.free();
    }
  }
  obj1.free();

  //calculate md5 digest
  Guchar digest[16];
  Decrypt::md5((Guchar*)message.getCString(), message.getLength(), digest);
  obj1.initString(new GooString((const char*)digest, 16));

  //create ID array
  Object obj2,obj3,obj4,obj5;
  obj2.initArray(xref);

  if (incrUpdate) {
    //only update the second part of the array
    if(xref->getTrailerDict()->getDict()->lookup("ID", &obj4) != NULL) {
      if (!obj4.isArray()) {
        error(-1, "PDFDoc::writeTrailer original file's ID entry isn't an array. Trying to continue");
      } else {
        //Get the first part of the ID
        obj4.arrayGet(0,&obj3); 

        obj2.arrayAdd(&obj3); 
        obj2.arrayAdd(&obj1);
        trailerDict->set("ID", &obj2);
      }
    }
  } else {
    //new file => same values for the two identifiers
    obj2.arrayAdd(&obj1);
    obj1.initString(new GooString((const char*)digest, 16));
    obj2.arrayAdd(&obj1);
    trailerDict->set("ID", &obj2);
  }


  obj1.initRef(xref->getRootNum(), xref->getRootGen());
  trailerDict->set("Root", &obj1);

  if (incrUpdate) { 
    obj1.initInt(xref->getLastXRefPos());
    trailerDict->set("Prev", &obj1);
  }
  
  xref->getDocInfoNF(&obj5);
  if (!obj5.isNull()) {
    trailerDict->set("Info", &obj5);
  }
  
  outStr->printf( "trailer\r\n");
  writeDictionnary(trailerDict, outStr);
  outStr->printf( "\r\nstartxref\r\n");
  outStr->printf( "%i\r\n", uxrefOffset);
  outStr->printf( "%%%%EOF\r\n");

  delete trailerDict;
}
