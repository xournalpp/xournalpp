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

#include <poppler/Object.h>
#include "poppler/XojPopplerDocument.h"

enum PdfRefEntryType {
	PDF_REF_ENTRY_TYPE_REF,
	PDF_REF_ENTRY_TYPE_DICT
};

class PdfRefEntry {
public:
	PdfRefEntry(PdfRefEntryType type, int objectId, Object * object, int imageId, XojPopplerDocument doc);
	~PdfRefEntry();

public:
	PdfRefEntryType type;
	int objectId;
	int refSourceId;
	XojPopplerDocument doc;
	Object * object;
};

#endif /* __PDFREFENTRY_H__ */
