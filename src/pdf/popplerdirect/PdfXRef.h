/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <XournalType.h>

class PdfXRef
{
public:
	PdfXRef();
	virtual ~PdfXRef();

public:
	void addXref(int ref);
	void setXref(int id, int ref);

	int getXrefCount();
	int getXref(int id);

private:
	XOJ_TYPE_ATTRIB;

	int* xref;
	int xrefLenght;
	int xrefNr;

};
