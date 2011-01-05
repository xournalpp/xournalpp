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

#include "XojConstants.h"

#include "../model/Document.h"
#include "../model/Stroke.h"
#include "../model/Text.h"
#include "../model/Image.h"
#include <zlib.h>

enum ParserPosition {
	PARSER_POS_NOT_STARTED = 1, // Waiting for opening <xounal> tag
	PARSER_POS_STARTED,         // Waiting for Metainfo or contents like <page>
	PARSER_POS_IN_PAGE,         // Starting page tag read
	PARSER_POS_IN_LAYER,        // Starting layer tag read
	PARSER_POS_IN_STROKE,       // Starting layer tag read
	PARSER_POS_IN_TEXT,         // Starting text tag read
	PARSER_POS_IN_IMAGE,        // Starting image tag read

	PASER_POS_FINISHED          // Document is parsed
};

class LoadHandler {
public:
	LoadHandler();
	virtual ~LoadHandler();

public:
	bool loadDocument(String filename, Document * doc);
	String getLastError();

	void parseStart();
	void parseContents();
	void parsePage();
	void parseLayer();

	void parseStroke();
	void parseText();
	void parseImage();
private:
	String readLine();
	int readFile(char * buffer, int len);
	bool closeFile();
	bool openFile(String filename);
	bool parseXml();

	bool parseColor(const char * text, int & color);

	static void parserText(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer userdata,
			GError **error);
	static void parserEndElement(GMarkupParseContext *context, const gchar *element_name, gpointer userdata,
			GError **error);
	static void parserStartElement(GMarkupParseContext *context, const gchar *element_name,
			const gchar **attribute_names, const gchar **attribute_values, gpointer userdata, GError **error);

	const char * getAttrib(const char * name, bool optional = false);
	double getAttribDouble(const char * name);
	int getAttribInt(const char * name);

	void parseBgSolid();
	void parseBgPixmap();
	void parseBgPdf();

private:
	String lastError;

	String filename;

	ParserPosition pos;

	String creator;
	int fileversion;

	gzFile fp;


	XojPage * page;
	Layer * layer;
	Stroke * stroke;
	Text * text;
	Image * image;

	String xournalFilename;
	String attachPdfFileNotFound;

	GError * error;
	const gchar **attributeNames;
	const gchar **attributeValues;
	const gchar * elementName;

	DocumentHandler dHanlder;
	Document doc;
};

#endif /* __LOADHANDLER_H__ */
