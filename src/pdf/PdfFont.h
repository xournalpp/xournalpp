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
#include <poppler.h>

class PdfFont {
public:
	PdfFont(XojPopplerDocument & doc, String originalName, int id, Object * object) {
		this->doc = doc;
		this->originalName = originalName;
		this->id = id;
		this->objectId = -1;
		this->object = object;
	}

	~PdfFont() {
		if(this->object) {
			delete this->object;
		}
		this->object = NULL;
	}

public:
	XojPopplerDocument doc;
	String originalName;

	int id;
	int objectId;

	Object * object;
};

#endif /* __PDFFONT_H__ */
