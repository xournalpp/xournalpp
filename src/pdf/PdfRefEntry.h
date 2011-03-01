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

class PdfRefEntry {
public:
	PdfRefEntry(int objectId, Ref ref, Object * object, int imageId, XojPopplerDocument doc);
	~PdfRefEntry();

	bool equalsRef(const Ref & ref);

public:
	int objectId;
	int imageId;
	Ref ref;
	XojPopplerDocument doc;
	Object * object;
};

#endif /* __PDFREFENTRY_H__ */
