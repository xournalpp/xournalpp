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

#include "PdfObjectWriter.h"
#include "poppler/XojPopplerDocument.h"
#include "poppler-0.24.1/poppler/Object.h"

#include <StringUtils.h>

#include <glib.h>

class PdfExport;
class PdfRefEntry;
class PdfWriter;
class PdfXRef;

class RefReplacement
{
public:
	RefReplacement(string name, int newId, const char* type, PdfRefEntry* refEntry);
	virtual ~RefReplacement();

public:
	string name;
	int newId;
	char* type;

public:
	/**
	 * Mark this reference as used
	 */
	void markAsUsed();

private:
	XOJ_TYPE_ATTRIB;

	bool used;

	PdfRefEntry* refEntry;
};

class PdfRefList
{
public:
	/**
	 * Type char is ownd by PdfRefList and should not be freed
	 */
	PdfRefList(PdfXRef* xref, PdfObjectWriter* objectWriter, PdfWriter* writer, char* type);
	virtual ~PdfRefList();

public:
	void writeObjects();
	void writeRefList(const char* type);

	int lookup(Ref ref, Object* object, XojPopplerDocument doc, PdfRefEntry*& refEntry);
	void parse(Dict* dict, int index, XojPopplerDocument doc, GList*& replacementList);

	static void deletePdfRefList(PdfRefList* ref);

private:
	XOJ_TYPE_ATTRIB;

	int id;
	GList* data;

	PdfXRef* xref;
	PdfObjectWriter* objectWriter;
	PdfWriter* writer;
	char* type;
};
