/*
 * Xournal++
 *
 * Handles PDF Export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "PdfWriter.h"
#include "PdfXRef.h"

#include "poppler/XojPopplerDocument.h"
#include <poppler/Object.h>

class PdfObjectWriter
{
public:
	PdfObjectWriter(PdfWriter* writer, PdfXRef* xref);
	virtual ~PdfObjectWriter();

public:
	void writeObject(Object* obj, XojPopplerDocument doc);
	void writeString(const GooString* s);
	void writeDictionnary(Dict* dict, XojPopplerDocument doc);
	void writeRawStream(Stream* str, XojPopplerDocument doc);
	void writeStream(Stream* str);
	void writeCopiedObjects();

private:
	XOJ_TYPE_ATTRIB;

	PdfWriter* writer;
	PdfXRef* xref;

	GHashTable* updatedReferenced;
};
