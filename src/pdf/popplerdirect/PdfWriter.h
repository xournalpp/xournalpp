/*
 * Xournal++
 *
 * Part of the PDF export
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFWRITER_H__
#define __PDFWRITER_H__

#include <gtk/gtk.h>
#include <String.h>
#include "PdfXRef.h"

class PdfWriter {
public:
	PdfWriter(PdfXRef * xref);
	virtual ~PdfWriter();

public:
	void close();
	bool openFile(const char * uri);

public:
	bool writeLen(const char * data, int len);
	bool write(const char * data);
	bool writef(const char * data, ...);
	bool writeTxt(const char * data);
	bool write(int data);

	void startStream();
	void endStream();

	bool writeInfo(String title);
	bool writeObj();


	String getLastError();

	int getObjectId();
	int getNextObjectId();
	int getDataCount();

public:
	static void setCompressPdfOutput(bool compress);

private:
	XOJ_TYPE_ATTRIB;

	static bool compressPdfOutput;

	int dataCount;
	bool inStream;
	GString * stream;

	GFileOutputStream * out;

	String lastError;

	PdfXRef * xref;

	int objectId;

};

#endif /* __PDFWRITER_H__ */
