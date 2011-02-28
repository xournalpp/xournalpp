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

#ifndef __PDFEXPORT_H__
#define __PDFEXPORT_H__

#include "../model/Document.h"
#include "../util/String.h"
#include "../control/jobs/ProgressListener.h"
#include "cairo/CairoPdf.h"
#include "PdfXRef.h"
#include "PdfBookmark.h"
#include "PdfWriter.h"
#include <glib.h>

class PdfExport {
public:
	PdfExport(Document * doc, ProgressListener * progressListener);
	virtual ~PdfExport();

public:
	bool createPdf(String uri);
	String getLastError();

private:

	void addPopplerDocument(XojPopplerDocument doc);

	bool addPopplerPage(XojPopplerPage * pdf, XojPopplerDocument doc);
	bool writePage(int page);

	void writeDictionnary(Dict * dict, XojPopplerDocument doc);
	void writeRawStream(Stream * str, XojPopplerDocument doc);
	void writeStream(Stream * str);
	void writeObject(Object * obj, XojPopplerDocument doc);
	void writeString(GooString * s);

	void writeGzStream(Stream * str, GList * replacementList);
	void writePlainStream(Stream * str, GList * replacementList);

	void writeStream(const char * str, int len, GList * replacementList);

	int lookupFont(String name, Ref ref);
	int lookupImage(String name, Ref ref, Object * object);

	bool parseFooter();
	bool writeFooter();

	bool writePagesindex();
	bool writeCatalog();
	bool writeCrossRef();
	bool writeTrailer();
	bool writeXobjectdict();
	bool writeResourcedict();
	bool writeResources();

	bool writeFonts();
	bool writeImages();
	bool writeCopiedObjects();

private:
	Document * doc;
	XojPopplerDocument currentPdfDoc;

	ProgressListener * progressListener;

	String lastError;

	int dataXrefStart;

	int pageCount;

	int outlineRoot;

	Dict * resources;

	GList * documents;

	int fontId;
	GList * fonts;

	int imageId;
	GList * images;

	PdfXRef * xref;
	PdfBookmarks bookmarks;
	PdfWriter * writer;

	CairoPdf cPdf;

	GHashTable * updatedReferenced;
};

#endif /* __PDFEXPORT_H__ */
