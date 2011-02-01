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

class PdfExportImage {
public:
	PdfExportImage(int objectId) {
		this->objectId = objectId;
	}

	~PdfExportImage() {
	}

public:
	int getObjectId() {
		return this->objectId;
	}

private:
	int objectId;
};

#endif /* __PDFEXPORTIMAGE_H__ */
