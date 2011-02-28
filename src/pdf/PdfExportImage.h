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

#ifndef __PDFEXPORTIMAGE_H__
#define __PDFEXPORTIMAGE_H__

#include <poppler/Object.h>

class PdfExportImage {
public:
	PdfExportImage(int objectId, Ref ref, Object * object, int imageId, XojPopplerDocument doc) {
		this->objectId = objectId;
		this->ref = ref;
		this->imageId = imageId;
		this->doc = doc;
		this->object = object;
	}

	~PdfExportImage() {
		if (this->object) {
			delete this->object;
		}
		this->object = NULL;
	}

	bool equalsRef(const Ref & ref) {
		return (this->ref.gen == ref.gen && this->ref.num == ref.num);
	}

public:
	int objectId;
	int imageId;
	Ref ref;
	XojPopplerDocument doc;
	Object * object;
};

#endif /* __PDFEXPORTIMAGE_H__ */
