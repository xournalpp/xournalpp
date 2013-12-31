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

#ifndef __PDFREFENTRY_H__
#define __PDFREFENTRY_H__

#include "poppler-0.24.1/poppler/Object.h"
#include "poppler/XojPopplerDocument.h"

enum PdfRefEntryType
{
    PDF_REF_ENTRY_TYPE_REF,
    PDF_REF_ENTRY_TYPE_DICT
};

class PdfRefEntry
{
public:
	PdfRefEntry(PdfRefEntryType type, int objectId, Object* object, int refSourceId,
	            Ref ref, XojPopplerDocument doc);
	virtual ~PdfRefEntry();

public:
	bool equalsRef(const Ref& ref);

public:
	XOJ_TYPE_ATTRIB;

	PdfRefEntryType type;
	int objectId;
	int refSourceId;
	XojPopplerDocument doc;
	Object* object;
	Ref ref;

public:
	/**
	 * Mark this reference as used
	 */
	void markAsUsed();

	bool isUsed();
private:
	bool used;
};

#endif /* __PDFREFENTRY_H__ */
