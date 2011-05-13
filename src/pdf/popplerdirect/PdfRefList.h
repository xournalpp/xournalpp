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
#include "../../util/String.h"
#include <poppler/Object.h>

#include "PdfObjectWriter.h"
#include "poppler/XojPopplerDocument.h"

class PdfXRef;
class PdfExport;
class PdfWriter;
class PdfRefEntry;

class RefReplacement {
public:
	RefReplacement(String name, int newId, const char * type, PdfRefEntry * refEntry);
	virtual ~RefReplacement();

public:
	String name;
	int newId;
	char * type;

public:
	/**
	 * Mark this reference as used
	 */
	void markAsUsed();

private:
	XOJ_TYPE_ATTRIB;

	bool used;

	PdfRefEntry * refEntry;
};

class PdfRefList {
public:
	/**
	 * Type char is ownd by PdfRefList and should not be freed
	 */
	PdfRefList(PdfXRef * xref, PdfObjectWriter * objectWriter, PdfWriter * writer, char * type);
	virtual ~PdfRefList();

public:
	void writeObjects();
	void writeRefList(const char * type);

	int lookup(Ref ref, Object * object, XojPopplerDocument doc, PdfRefEntry * &refEntry);
	void parse(Dict * dict, int index, XojPopplerDocument doc, GList * &replacementList);

	static void deletePdfRefList(PdfRefList * ref);

private:
	XOJ_TYPE_ATTRIB;

	int id;
	GList * data;

	PdfXRef * xref;
	PdfObjectWriter * objectWriter;
	PdfWriter * writer;
	char * type;
};

#endif /* PDFREFLIST_H_ */
