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

#ifndef PDFEXPORT_H_
#define PDFEXPORT_H_

#include "../model/Document.h"
#include "../model/String.h"
#include <gtk/gtk.h>

class PdfExport {
public:
	PdfExport(Document * doc);
	virtual ~PdfExport();

	bool createPdf(String uri, bool * cancel);
	String getLastError();

private:
	bool writeLen(const char * data, int len);
	bool write(const char * data);
	bool writef(const char * data, ...);
	bool writeTxt(const char * data);
	bool write(int data);

	bool addPopplerPage(XojPopplerPage * pdf);
	bool writePage(int page);

	bool parseFooter();
	bool writeInfo();
	bool writeFooter();

	void startStream();
	void endStream();

	GList * exportBookmarksFromTreeModel(GtkTreeModel * model);
	void createBookmarks(GtkTreeModel * model, GList * &data, GtkTreeIter * iter, int level);

	bool writePagesindex();
	bool writeOutlines();
	bool writeCatalog();
	bool writeCrossRef();
	bool writeTrailer();
	bool writeXobjectdict();
	bool writeResourcedict();
	bool writeResources();
	bool writeObj();
	void addXref(int ref);

private:
	Document * doc;

	bool compressOutput;

	String lastError;

	GFileOutputStream * out;

	int objectId;
	int * xref;
	int xrefLenght;
	int xrefNr;
	int dataCount;
	int dataXrefStart;

	int pageCount;

	int outlineRoot;

	bool inStream;
	GString * stream;
};

class PdfPageDesc {
public:
	struct PdfObj *resources, *mediabox, *contents;
	int rotate;
};

class PdfInfo {
public:
	int startxref;
	struct PdfObj *trailerdict;
	int npages;
	struct PdfPageDesc *pages;
};

#endif /* PDFEXPORT_H_ */
