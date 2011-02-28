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
	PdfExportImage(int objectId, Ref ref) {
		this->objectId = objectId;
		this->ref = ref;
	}

	~PdfExportImage() {
	}

	bool equalsRef(const Ref & ref) {
		return (this->ref.gen == ref.gen && this->ref.num == ref.num);
	}

public:
	int objectId;
	Ref ref;
};

#endif /* __PDFEXPORTIMAGE_H__ */
