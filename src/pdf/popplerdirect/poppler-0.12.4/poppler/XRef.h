//========================================================================
//
// XRef.h
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
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "../goo/gtypes.h"
#include "Object.h"

class Dict;
class Stream;
class Parser;
class ObjectStream;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

enum XRefEntryType {
  xrefEntryFree,
  xrefEntryUncompressed,
  xrefEntryCompressed
};

struct XRefEntry {
  Guint offset;
  int gen;
  XRefEntryType type;
  bool updated;
  Object obj; //if this entry was updated, obj will contains the updated object
};

class XRef {
public:

  // Constructor, create an empty XRef, used for PDF writing
  XRef();
  // Constructor.  Read xref table from stream.
  XRef(BaseStream *strA);

  // Destructor.
  ~XRef();

  // Is xref table valid?
  GBool isOk() { return ok; }

  // Get the error code (if isOk() returns false).
  int getErrorCode() { return errCode; }

  // Set the encryption parameters.
  void setEncryption(int permFlagsA, GBool ownerPasswordOkA,
		     Guchar *fileKeyA, int keyLengthA,
		     int encVersionA, int encRevisionA,
		     CryptAlgorithm encAlgorithmA);

  // Is the file encrypted?
  GBool isEncrypted() { return encrypted; }

  // Check various permissions.
  GBool okToPrint(GBool ignoreOwnerPW = gFalse);
  GBool okToPrintHighRes(GBool ignoreOwnerPW = gFalse);
  GBool okToChange(GBool ignoreOwnerPW = gFalse);
  GBool okToCopy(GBool ignoreOwnerPW = gFalse);
  GBool okToAddNotes(GBool ignoreOwnerPW = gFalse);
  GBool okToFillForm(GBool ignoreOwnerPW = gFalse);
  GBool okToAccessibility(GBool ignoreOwnerPW = gFalse);
  GBool okToAssemble(GBool ignoreOwnerPW = gFalse);

  // Get catalog object.
  Object *getCatalog(Object *obj) { return fetch(rootNum, rootGen, obj); }

  // Fetch an indirect reference.
  Object *fetch(int num, int gen, Object *obj);

  // Return the document's Info dictionary (if any).
  Object *getDocInfo(Object *obj);
  Object *getDocInfoNF(Object *obj);

  // Return the number of objects in the xref table.
  int getNumObjects() { return size; }

  // Return the offset of the last xref table.
  Guint getLastXRefPos() { return lastXRefPos; }

  // Return the catalog object reference.
  int getRootNum() { return rootNum; }
  int getRootGen() { return rootGen; }

  // Get end position for a stream in a damaged file.
  // Returns false if unknown or file is not damaged.
  GBool getStreamEnd(Guint streamStart, Guint *streamEnd);

  // Retuns the entry that belongs to the offset
  int getNumEntry(Guint offset) const;

  // Direct access.
  int getSize() { return size; }
  XRefEntry *getEntry(int i) { return &entries[i]; }
  Object *getTrailerDict() { return &trailerDict; }

  // Write access
  void setModifiedObject(Object* o, Ref r);
  Ref addIndirectObject (Object* o);
  void add(int num, int gen,  Guint offs, GBool used);
  void writeToFile(OutStream* outStr, GBool writeAllEntries);

private:

  BaseStream *str;		// input stream
  Guint start;			// offset in file (to allow for garbage
				//   at beginning of file)
  XRefEntry *entries;		// xref entries
  int size;			// size of <entries> array
  int rootNum, rootGen;		// catalog dict
  GBool ok;			// true if xref table is valid
  int errCode;			// error code (if <ok> is false)
  Object trailerDict;		// trailer dictionary
  Guint lastXRefPos;		// offset of last xref table
  Guint *streamEnds;		// 'endstream' positions - only used in
				//   damaged files
  int streamEndsLen;		// number of valid entries in streamEnds
  ObjectStream *objStr;		// cached object stream
  GBool encrypted;		// true if file is encrypted
  int encRevision;		
  int encVersion;		// encryption algorithm
  CryptAlgorithm encAlgorithm;	// encryption algorithm
  int keyLength;		// length of key, in bytes
  int permFlags;		// permission bits
  Guchar fileKey[16];		// file decryption key
  GBool ownerPasswordOk;	// true if owner password is correct

  Guint getStartXref();
  GBool readXRef(Guint *pos);
  GBool readXRefTable(Parser *parser, Guint *pos);
  GBool readXRefStreamSection(Stream *xrefStr, int *w, int first, int n);
  GBool readXRefStream(Stream *xrefStr, Guint *pos);
  GBool constructXRef();
  Guint strToUnsigned(char *s);
};

#endif
