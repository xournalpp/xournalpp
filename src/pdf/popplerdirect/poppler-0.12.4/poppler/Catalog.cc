//========================================================================
//
// Catalog.cc
//
// Copyright 1996-2007 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005-2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jeff Muizelaar <jrmuizel@nit.ca>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <stdlib.h>
#include "goo/gmem.h"
#include "Object.h"
#include "XRef.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Link.h"
#include "PageLabelInfo.h"
#include "Catalog.h"
#include "Form.h"
#include "OptionalContent.h"

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(XRef *xrefA) {
  Object catDict, pagesDict, pagesDictRef;
  Object obj, obj2;
  Object optContentProps;
  char *alreadyRead;
  int numPages0;
  int i;

  ok = gTrue;
  xref = xrefA;
  pages = NULL;
  pageRefs = NULL;
  numPages = pagesSize = 0;
  baseURI = NULL;
  pageLabelInfo = NULL;
  form = NULL;
  optContent = NULL;

  xref->getCatalog(&catDict);
  if (!catDict.isDict()) {
    error(-1, "Catalog object is wrong type (%s)", catDict.getTypeName());
    goto err1;
  }
  // get the AcroForm dictionary
  catDict.dictLookup("AcroForm", &acroForm);

  // load Forms
  if (acroForm.isDict()) {
    form = new Form(xref,&acroForm);
  }


  // read page tree
  catDict.dictLookup("Pages", &pagesDict);
  // This should really be isDict("Pages"), but I've seen at least one
  // PDF file where the /Type entry is missing.
  if (!pagesDict.isDict()) {
    error(-1, "Top-level pages object is wrong type (%s)",
	  pagesDict.getTypeName());
    goto err2;
  }
  pagesDict.dictLookup("Count", &obj);
  // some PDF files actually use real numbers here ("/Count 9.0")
  if (!obj.isNum()) {
    error(-1, "Page count in top-level pages object is wrong type (%s)",
	  obj.getTypeName());
    goto err3;
  }
  pagesSize = numPages0 = (int)obj.getNum();
  obj.free();
  pages = (Page **)gmallocn(pagesSize, sizeof(Page *));
  pageRefs = (Ref *)gmallocn(pagesSize, sizeof(Ref));
  for (i = 0; i < pagesSize; ++i) {
    pages[i] = NULL;
    pageRefs[i].num = -1;
    pageRefs[i].gen = -1;
  }
  alreadyRead = (char *)gmalloc(xref->getNumObjects());
  memset(alreadyRead, 0, xref->getNumObjects());
  if (catDict.dictLookupNF("Pages", &pagesDictRef)->isRef() &&
      pagesDictRef.getRefNum() >= 0 &&
      pagesDictRef.getRefNum() < xref->getNumObjects()) {
    alreadyRead[pagesDictRef.getRefNum()] = 1;
  }
  pagesDictRef.free();
  numPages = readPageTree(pagesDict.getDict(), NULL, 0, alreadyRead);
  gfree(alreadyRead);
  if (numPages != numPages0) {
    error(-1, "Page count in top-level pages object is incorrect");
  }
  pagesDict.free();

  // read named destination dictionary
  catDict.dictLookup("Dests", &dests);

  // read root of named destination tree - PDF1.6 table 3.28
  if (catDict.dictLookup("Names", &obj)->isDict()) {
    obj.dictLookup("Dests", &obj2);
    destNameTree.init(xref, &obj2);
    obj2.free();
    obj.dictLookup("EmbeddedFiles", &obj2);
    embeddedFileNameTree.init(xref, &obj2);
    obj2.free();
    obj.dictLookup("JavaScript", &obj2);
    jsNameTree.init(xref, &obj2);
    obj2.free();
  }
  obj.free();

  if (catDict.dictLookup("PageLabels", &obj)->isDict())
    pageLabelInfo = new PageLabelInfo(&obj, numPages);
  obj.free();

  // read page mode
  pageMode = pageModeNone;
  if (catDict.dictLookup("PageMode", &obj)->isName()) {
    if (obj.isName("UseNone"))
      pageMode = pageModeNone;
    else if (obj.isName("UseOutlines"))
      pageMode = pageModeOutlines;
    else if (obj.isName("UseThumbs"))
      pageMode = pageModeThumbs;
    else if (obj.isName("FullScreen"))
      pageMode = pageModeFullScreen;
    else if (obj.isName("UseOC"))
      pageMode = pageModeOC;
    else if (obj.isName("UseAttachments"))
      pageMode = pageModeAttach;
  }
  obj.free();

  pageLayout = pageLayoutNone;
  if (catDict.dictLookup("PageLayout", &obj)->isName()) {
    if (obj.isName("SinglePage"))
      pageLayout = pageLayoutSinglePage;
    if (obj.isName("OneColumn"))
      pageLayout = pageLayoutOneColumn;
    if (obj.isName("TwoColumnLeft"))
      pageLayout = pageLayoutTwoColumnLeft;
    if (obj.isName("TwoColumnRight"))
      pageLayout = pageLayoutTwoColumnRight;
    if (obj.isName("TwoPageLeft"))
      pageLayout = pageLayoutTwoPageLeft;
    if (obj.isName("TwoPageRight"))
      pageLayout = pageLayoutTwoPageRight;
  }
  obj.free();

  // read base URI
  if (catDict.dictLookup("URI", &obj)->isDict()) {
    if (obj.dictLookup("Base", &obj2)->isString()) {
      baseURI = obj2.getString()->copy();
    }
    obj2.free();
  }
  obj.free();

  // get the metadata stream
  catDict.dictLookup("Metadata", &metadata);

  // get the structure tree root
  catDict.dictLookup("StructTreeRoot", &structTreeRoot);

  // get the outline dictionary
  catDict.dictLookup("Outlines", &outline);

  // get the Optional Content dictionary
  if (catDict.dictLookup("OCProperties", &optContentProps)->isDict()) {
    optContent = new OCGs(&optContentProps, xref);
    if (!optContent->isOk ()) {
      delete optContent;
      optContent = NULL;
    }
  }
  optContentProps.free();

  // perform form-related loading after all widgets have been loaded
  if (form) 
    form->postWidgetsLoad();

  catDict.free();
  return;

 err3:
  obj.free();
 err2:
  pagesDict.free();
 err1:
  catDict.free();
  dests.initNull();
  ok = gFalse;
}

Catalog::~Catalog() {
  int i;

  if (pages) {
    for (i = 0; i < pagesSize; ++i) {
      if (pages[i]) {
	delete pages[i];
      }
    }
    gfree(pages);
    gfree(pageRefs);
  }
  dests.free();
  destNameTree.free();
  embeddedFileNameTree.free();
  jsNameTree.free();
  if (baseURI) {
    delete baseURI;
  }
  delete pageLabelInfo;
  delete form;
  delete optContent;
  metadata.free();
  structTreeRoot.free();
  outline.free();
  acroForm.free();
}

GooString *Catalog::readMetadata() {
  GooString *s;
  Dict *dict;
  Object obj;
  int c;

  if (!metadata.isStream()) {
    return NULL;
  }
  dict = metadata.streamGetDict();
  if (!dict->lookup("Subtype", &obj)->isName("XML")) {
    error(-1, "Unknown Metadata type: '%s'",
	  obj.isName() ? obj.getName() : "???");
  }
  obj.free();
  s = new GooString();
  metadata.streamReset();
  while ((c = metadata.streamGetChar()) != EOF) {
    s->append(c);
  }
  metadata.streamClose();
  return s;
}

int Catalog::readPageTree(Dict *pagesDict, PageAttrs *attrs, int start,
			  char *alreadyRead) {
  Object kids;
  Object kid;
  Object kidRef;
  PageAttrs *attrs1, *attrs2;
  Page *page;
  int i, j;

  attrs1 = new PageAttrs(attrs, pagesDict);
  pagesDict->lookup("Kids", &kids);
  if (!kids.isArray()) {
    error(-1, "Kids object (page %d) is wrong type (%s)",
	  start+1, kids.getTypeName());
    goto err1;
  }
  for (i = 0; i < kids.arrayGetLength(); ++i) {
    kids.arrayGetNF(i, &kidRef);
    if (kidRef.isRef() &&
	kidRef.getRefNum() >= 0 &&
	kidRef.getRefNum() < xref->getNumObjects()) {
      if (alreadyRead[kidRef.getRefNum()]) {
	error(-1, "Loop in Pages tree");
	kidRef.free();
	continue;
      }
      alreadyRead[kidRef.getRefNum()] = 1;
    }
    kids.arrayGet(i, &kid);
    if (kid.isDict("Page")) {
      attrs2 = new PageAttrs(attrs1, kid.getDict());
      page = new Page(xref, start+1, kid.getDict(), kidRef.getRef(), attrs2, form);
      if (!page->isOk()) {
	++start;
	goto err3;
      }
      if (start >= pagesSize) {
	pagesSize += 32;
	pages = (Page **)greallocn(pages, pagesSize, sizeof(Page *));
	pageRefs = (Ref *)greallocn(pageRefs, pagesSize, sizeof(Ref));
	for (j = pagesSize - 32; j < pagesSize; ++j) {
	  pages[j] = NULL;
	  pageRefs[j].num = -1;
	  pageRefs[j].gen = -1;
	}
      }
      pages[start] = page;
      if (kidRef.isRef()) {
	pageRefs[start].num = kidRef.getRefNum();
	pageRefs[start].gen = kidRef.getRefGen();
      }
      ++start;
    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    } else if (kid.isDict()) {
      if ((start = readPageTree(kid.getDict(), attrs1, start, alreadyRead))
	  < 0)
	goto err2;
    } else {
      error(-1, "Kid object (page %d) is wrong type (%s)",
	    start+1, kid.getTypeName());
    }
    kid.free();
    kidRef.free();
  }
  delete attrs1;
  kids.free();
  return start;

 err3:
  delete page;
 err2:
  kid.free();
  kidRef.free();
 err1:
  kids.free();
  delete attrs1;
  ok = gFalse;
  return -1;
}

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < numPages; ++i) {
    if (pageRefs[i].num == num && pageRefs[i].gen == gen)
      return i + 1;
  }
  return 0;
}

LinkDest *Catalog::findDest(GooString *name) {
  LinkDest *dest;
  Object obj1, obj2;
  GBool found;

  // try named destination dictionary then name tree
  found = gFalse;
  if (dests.isDict()) {
    if (!dests.dictLookup(name->getCString(), &obj1)->isNull())
      found = gTrue;
    else
      obj1.free();
  }
  if (!found) {
    if (destNameTree.lookup(name, &obj1))
      found = gTrue;
    else
      obj1.free();
  }
  if (!found)
    return NULL;

  // construct LinkDest
  dest = NULL;
  if (obj1.isArray()) {
    dest = new LinkDest(obj1.getArray());
  } else if (obj1.isDict()) {
    if (obj1.dictLookup("D", &obj2)->isArray())
      dest = new LinkDest(obj2.getArray());
    else
      error(-1, "Bad named destination value");
    obj2.free();
  } else {
    error(-1, "Bad named destination value");
  }
  obj1.free();
  if (dest && !dest->isOk()) {
    delete dest;
    dest = NULL;
  }

  return dest;
}

EmbFile *Catalog::embeddedFile(int i)
{
    Object efDict;
    Object obj;
    obj = embeddedFileNameTree.getValue(i);
    EmbFile *embeddedFile = 0;
    if (obj.isRef()) {
        GooString desc(embeddedFileNameTree.getName(i));
        embeddedFile = new EmbFile(obj.fetch(xref, &efDict), &desc);
        efDict.free();
    } else {
        Object null;
        embeddedFile = new EmbFile(&null);
    }
    return embeddedFile;
}

GooString *Catalog::getJS(int i)
{
  Object obj = jsNameTree.getValue(i);
  if (obj.isRef()) {
    Ref r = obj.getRef();
    obj.free();
    xref->fetch(r.num, r.gen, &obj);
  }

  if (!obj.isDict()) {
    obj.free();
    return 0;
  }
  Object obj2;
  if (!obj.dictLookup("S", &obj2)->isName()) {
    obj2.free();
    obj.free();
    return 0;
  }
  if (strcmp(obj2.getName(), "JavaScript")) {
    obj2.free();
    obj.free();
    return 0;
  }
  obj2.free();
  obj.dictLookup("JS", &obj2);
  GooString *js = 0;
  if (obj2.isString()) {
    js = new GooString(obj2.getString());
  }
  else if (obj2.isStream()) {
    Stream *stream = obj2.getStream();
    js = new GooString();
    stream->reset();
    int j;
    while ((j = stream->getChar()) != EOF) {
      js->append((char)j);
    }
  }
  obj2.free();
  obj.free();
  return js;
}

NameTree::NameTree()
{
  size = 0;
  length = 0;
  entries = NULL;
}

NameTree::Entry::Entry(Array *array, int index) {
    if (!array->getString(index, &name) || !array->getNF(index + 1, &value)) {
      Object aux;
      array->get(index, &aux);
      if (aux.isString() && array->getNF(index + 1, &value) )
      {
        name.append(aux.getString());
      }
      else
        error(-1, "Invalid page tree");
    }
}

NameTree::Entry::~Entry() {
  value.free();
}

void NameTree::addEntry(Entry *entry)
{
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    entries = (Entry **) grealloc (entries, sizeof (Entry *) * size);
  }

  entries[length] = entry;
  ++length;
}

void NameTree::init(XRef *xrefA, Object *tree) {
  xref = xrefA;
  parse(tree);
}

void NameTree::parse(Object *tree) {
  Object names;
  Object kids, kid;
  int i;

  if (!tree->isDict())
    return;

  // leaf node
  if (tree->dictLookup("Names", &names)->isArray()) {
    for (i = 0; i < names.arrayGetLength(); i += 2) {
      NameTree::Entry *entry;

      entry = new Entry(names.getArray(), i);
      addEntry(entry);
    }
  }
  names.free();

  // root or intermediate node
  if (tree->dictLookup("Kids", &kids)->isArray()) {
    for (i = 0; i < kids.arrayGetLength(); ++i) {
      if (kids.arrayGet(i, &kid)->isDict())
	parse(&kid);
      kid.free();
    }
  }
  kids.free();
}

int NameTree::Entry::cmp(const void *voidKey, const void *voidEntry)
{
  GooString *key = (GooString *) voidKey;
  Entry *entry = *(NameTree::Entry **) voidEntry;

  return key->cmp(&entry->name);
}

GBool NameTree::lookup(GooString *name, Object *obj)
{
  Entry **entry;

  entry = (Entry **) bsearch(name, entries,
			     length, sizeof(Entry *), Entry::cmp);
  if (entry != NULL) {
    (*entry)->value.fetch(xref, obj);
    return gTrue;
  } else {
    printf("failed to look up %s\n", name->getCString());
    obj->initNull();
    return gFalse;
  }
}

Object NameTree::getValue(int index)
{
  if (index < length) {
    return entries[index]->value;
  } else {
    return Object();
  }
}

GooString *NameTree::getName(int index)
{
    if (index < length) {
	return &entries[index]->name;
    } else {
	return NULL;
    }
}

void NameTree::free()
{
  int i;

  for (i = 0; i < length; i++)
    delete entries[i];

  gfree(entries);
}

GBool Catalog::labelToIndex(GooString *label, int *index)
{
  char *end;

  if (pageLabelInfo != NULL) {
    if (!pageLabelInfo->labelToIndex(label, index))
      return gFalse;
  } else {
    *index = strtol(label->getCString(), &end, 10) - 1;
    if (*end != '\0')
      return gFalse;
  }

  if (*index < 0 || *index >= numPages)
    return gFalse;

  return gTrue;
}

GBool Catalog::indexToLabel(int index, GooString *label)
{
  char buffer[32];

  if (index < 0 || index >= numPages)
    return gFalse;

  if (pageLabelInfo != NULL) {
    return pageLabelInfo->indexToLabel(index, label);
  } else {
    snprintf(buffer, sizeof (buffer), "%d", index + 1);
    label->append(buffer);	      
    return gTrue;
  }
}

EmbFile::EmbFile(Object *efDict, GooString *description)
{
  m_name = 0;
  m_description = 0;
  if (description)
    m_description = description->copy();
  m_size = -1;
  m_createDate = 0;
  m_modDate = 0;
  m_checksum = 0;
  m_mimetype = 0;
  if (efDict->isDict()) {
    Object fileSpec;
    Object fileDesc;
    Object paramDict;
    Object paramObj;
    Object obj2;
    Stream *efStream = NULL;
    // efDict matches Table 3.40 in the PDF1.6 spec
    efDict->dictLookup("F", &fileSpec);
    if (fileSpec.isString()) {
      m_name = new GooString(fileSpec.getString());
    }
    fileSpec.free();

    // the logic here is that the description from the name
    // dictionary is used if we don't have a more specific
    // description - see the Note: on page 157 of the PDF1.6 spec
    efDict->dictLookup("Desc", &fileDesc);
    if (fileDesc.isString()) {
      delete m_description;
      m_description = new GooString(fileDesc.getString());
    } else {
      efDict->dictLookup("Description", &fileDesc);
      if (fileDesc.isString()) {
        delete m_description;
        m_description = new GooString(fileDesc.getString());
      }
    }
    fileDesc.free();

    efDict->dictLookup("EF", &obj2);
    if (obj2.isDict()) {
      // This gives us the raw data stream bytes

      obj2.dictLookup("F", &m_objStr);
      if (m_objStr.isStream()) {
        efStream = m_objStr.getStream();

        // dataDict corresponds to Table 3.41 in the PDF1.6 spec.
        Dict *dataDict = efStream->getDict();

        // subtype is normally the mimetype
        Object subtypeName;
        if (dataDict->lookup("Subtype", &subtypeName)->isName()) {
          m_mimetype = new GooString(subtypeName.getName());
        }
        subtypeName.free();

        // paramDict corresponds to Table 3.42 in the PDF1.6 spec
        Object paramDict;
        dataDict->lookup( "Params", &paramDict );
        if (paramDict.isDict()) {
          paramDict.dictLookup("ModDate", &paramObj);
          if (paramObj.isString()) {
            m_modDate = new GooString(paramObj.getString());
          }
          paramObj.free();
          paramDict.dictLookup("CreationDate", &paramObj);
          if (paramObj.isString()) {
            m_createDate = new GooString(paramObj.getString());
          }
          paramObj.free();
          paramDict.dictLookup("Size", &paramObj);
          if (paramObj.isInt()) {
            m_size = paramObj.getInt();
          }
          paramObj.free();
          paramDict.dictLookup("CheckSum", &paramObj);
          if (paramObj.isString()) {
            m_checksum = new GooString(paramObj.getString());
          }
          paramObj.free();
        }
        paramDict.free();
      }
    }
    obj2.free();
  }
  if (!m_name)
    m_name = new GooString();
  if (!m_description)
    m_description = new GooString();
  if (!m_createDate)
    m_createDate = new GooString();
  if (!m_modDate)
    m_modDate = new GooString();
  if (!m_checksum)
    m_checksum = new GooString();
  if (!m_mimetype)
    m_mimetype = new GooString();
}
