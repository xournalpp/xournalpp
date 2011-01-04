/*
 * Xournal++
 *
 * Handlers PDF Export
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
	static cairo_status_t writeOut(PdfExport *pdf, unsigned char *data, unsigned int length);
	bool write(const char * data, int len);
	bool write(const char * data);
	bool writeTxt(const char * data);
	bool write(int data);

	bool parseFooter();
	bool writeInfo();
	bool writeFooter();

	GList * exportBookmarksFromTreeModel(GtkTreeModel * model);
	void createBookmarks(GtkTreeModel * model, GList * &data, GtkTreeIter * iter, int level);

	bool writeOutlines();
	bool writeCatalog();
	bool writeCrossRef();
	bool writeTrailer();
	bool writeObj();
	void addXref(int ref);

private:
	Document * doc;

	String lastError;

	GFileOutputStream * out;

	GString * end;

	int objectId;
	int * xref;
	int xrefLenght;
	int xrefNr;
	int dataCount;
	int dataXrefStart;

	int outlineRoot;

	bool inPagesEnd;
	bool inSavestream;
	bool inFooter;
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
