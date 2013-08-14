/*
 * Xournal++
 *
 * Handles PDF Export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFOBJECTWRITER_H__
#define __PDFOBJECTWRITER_H__

#include "poppler/XojPopplerDocument.h"
#include "poppler-0.12.4/poppler/Object.h"

#include "PdfWriter.h"
#include "PdfXRef.h"

class PdfObjectWriter {
public:
	PdfObjectWriter(PdfWriter * writer, PdfXRef * xref);
	virtual ~PdfObjectWriter();

public:
	void writeObject(Object * obj, XojPopplerDocument doc);
	void writeString(GooString * s);
	void writeDictionnary(Dict * dict, XojPopplerDocument doc);
	void writeRawStream(Stream * str, XojPopplerDocument doc);
	void writeStream(Stream * str);
	void writeCopiedObjects();

private:
	XOJ_TYPE_ATTRIB;

	PdfWriter * writer;
	PdfXRef * xref;

	GHashTable * updatedReferenced;
};

#endif /* __PDFOBJECTWRITER_H__ */
