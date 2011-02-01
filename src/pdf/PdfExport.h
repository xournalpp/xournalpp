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

	static bool isWhitespace(int c);
private:
	bool writeLen(const char * data, int len);
	bool write(const char * data);
	bool writef(const char * data, ...);
	bool writeTxt(const char * data);
	bool write(int data);

	void addPopplerDocument(XojPopplerDocument doc);

	bool addPopplerPage(XojPopplerPage * pdf, XojPopplerDocument doc);
	bool writePage(int page);

	void writeDictionnary(Dict* dict, XojPopplerDocument doc);
	void writeRawStream(Stream* str, XojPopplerDocument doc);
	void writeStream(Stream* str);
	void writeObject(Object* obj, XojPopplerDocument doc);
	void writeString(GooString* s);

	void writeGzStream(Stream* str);
	void writePlainStream(Stream* str);

	int lookupFont(String name);

	void writeStreamLine(char * line, int len);

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

	bool writeFonts();
	bool writeImages();

	bool writeObj();
	void addXref(int ref);

private:
	Document * doc;
	XojPopplerDocument currentPdfDoc;

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

	Dict * resources;

	GList * images;

	GList * documents;

	int fontId;
	GList * fonts;

	GHashTable * updatedReferenced;
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

class UpdateRefKey {
public:
	UpdateRefKey(Ref ref, XojPopplerDocument doc) {
		this->ref = ref;
		this->doc = doc;
	}

public:
	static guint hashFunction(UpdateRefKey * key);
	static bool equalFunction(UpdateRefKey * a, UpdateRefKey * b);

public:

	Ref ref;
	XojPopplerDocument doc;
};

class UpdateRef {
public:
	UpdateRef(int objectId, XojPopplerDocument doc) {
		this->objectId = objectId;
		this->wroteOut = false;
		this->doc = doc;
	}
public:
	int objectId;
	bool wroteOut;

	Object object;
	XojPopplerDocument doc;
};

#endif /* PDFEXPORT_H_ */
