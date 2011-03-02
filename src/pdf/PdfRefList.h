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

#ifndef __PDFREFLIST_H__
#define __PDFREFLIST_H__

#include <glib.h>
#include "../util/String.h"
#include <poppler/Object.h>

#include "PdfObjectWriter.h"
#include "poppler/XojPopplerDocument.h"

class PdfXRef;
class PdfExport;
class PdfWriter;

class PdfRefList {
public:
	PdfRefList(PdfXRef * xref, PdfObjectWriter * objectWriter, PdfWriter * writer, const char * type);
	virtual ~PdfRefList();

public:
	void writeObjects();
	bool writeRefList();
	int lookup(String name, Ref ref, Object * object, XojPopplerDocument doc);

private:
	int id;
	GList * data;

	PdfXRef * xref;
	PdfObjectWriter * objectWriter;
	PdfWriter * writer;
	const char * type;
};

#endif /* PDFREFLIST_H_ */
