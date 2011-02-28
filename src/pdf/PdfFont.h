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

#ifndef __PDFFONT_H__
#define __PDFFONT_H__

#include "poppler/XojPopplerDocument.h"

class PdfFont {
public:
	PdfFont(XojPopplerDocument & doc, String originalName, int id, Object * object);
	~PdfFont();

public:
	XojPopplerDocument doc;
	String originalName;

	int id;
	int objectId;

	Object * object;
};

#endif /* __PDFFONT_H__ */
