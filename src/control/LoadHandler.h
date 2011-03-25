/*
 * Xournal++
 *
 * Loads a document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __LOADHANDLER_H__
#define __LOADHANDLER_H__

#include "../model/Document.h"
#include "../model/Stroke.h"
#include "../model/Text.h"
#include "../model/Image.h"
#include "../util/XournalType.h"
#include <zlib.h>

enum ParserPosition {
	PARSER_POS_NOT_STARTED = 1, // Waiting for opening <xounal> tag
	PARSER_POS_STARTED, // Waiting for Metainfo or contents like <page>
	PARSER_POS_IN_PAGE, // Starting page tag read
	PARSER_POS_IN_LAYER, // Starting layer tag read
	PARSER_POS_IN_STROKE, // Starting layer tag read
	PARSER_POS_IN_TEXT, // Starting text tag read
	PARSER_POS_IN_IMAGE, // Starting image tag read

	PASER_POS_FINISHED // Document is parsed
};

class DoubleArrayBuffer {
public:
	DoubleArrayBuffer();
	virtual ~DoubleArrayBuffer();

public:
	void clear();
	const double * getData();
	int size();
	void add(double d);

private:
	XOJ_TYPE_ATTRIB;

	double * data;
	int len;
	int allocCount;
};

class LoadHandler {
public:
	LoadHandler();
	virtual ~LoadHandler();

public:
	Document * loadDocument(String filename);

	String getLastError();
	bool isAttachedPdfMissing();
	String getMissingPdfFilename();

	void removePdfBackground();
	void setPdfReplacement(String filename, bool attachToDocument);

private:
	void parseStart();
	void parseContents();
	void parsePage();
	void parseLayer();

	void parseStroke();
	void parseText();
	void parseImage();

private:
	void initAttributes();

	String readLine();
	int readFile(char * buffer, int len);
	bool closeFile();
	bool openFile(String filename);
	bool parseXml();

	bool parseColor(const char * text, int & color);

	static void parserText(GMarkupParseContext * context, const gchar * text, gsize text_len, gpointer userdata,
			GError ** error);
	static void parserEndElement(GMarkupParseContext * context, const gchar * element_name, gpointer userdata,
			GError ** error);
	static void parserStartElement(GMarkupParseContext * context, const gchar * element_name,
			const gchar ** attribute_names, const gchar ** attribute_values, gpointer userdata, GError ** error);

	const char * getAttrib(const char * name, bool optional = false);
	double getAttribDouble(const char * name);
	int getAttribInt(const char * name);

	void parseBgSolid();
	void parseBgPixmap();
	void parseBgPdf();

	void readImage(const gchar * base64_str, gsize base64_strlen);

private:
	XOJ_TYPE_ATTRIB;

	String lastError;
	String pdfMissing;
	bool attachedPdfMissing;

	bool removePdfBackgroundFlag;
	String pdfReplacementFilename;
	bool pdfReplacementAttach;

	String filename;

	bool pdfFilenameParsed;

	ParserPosition pos;

	String creator;
	int fileversion;

	gzFile fp;

	DoubleArrayBuffer pressureBuffer;

	XojPage * page;
	Layer * layer;
	Stroke * stroke;
	Text * text;
	Image * image;

	String xournalFilename;

	GError * error;
	const gchar **attributeNames;
	const gchar **attributeValues;
	const gchar * elementName;

	DocumentHandler dHanlder;
	Document doc;
};

#endif /* __LOADHANDLER_H__ */
