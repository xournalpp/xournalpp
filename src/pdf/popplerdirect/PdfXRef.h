/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFXREF_H__
#define __PDFXREF_H__

#include <XournalType.h>

class PdfXRef {
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

	int * xref;
	int xrefLenght;
	int xrefNr;

};

#endif /* __PDFXREF_H__ */
