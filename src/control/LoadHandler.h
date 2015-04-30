/*
 * Xournal++
 *
 * Loads a document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 * 
 * @revision MarPiRK – significant changes
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Document.h"
#include "model/Image.h"
#include "model/Stroke.h"
#include "model/TexImage.h"
#include "model/Text.h"

#include <XournalType.h>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
namespace bio = boost::iostreams;
#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

#include "pugixml/pugixml.hpp"
using namespace pugi;

#include <map>
#include <fstream>
#include <stdexcept>
using namespace std;

enum ParserPosition
{
	PARSER_POS_NOT_STARTED = 1, // Waiting for opening <xounal> tag
	PARSER_POS_STARTED,         // Waiting for Metainfo or contents like <page>
	PARSER_POS_IN_PAGE,         // Starting page tag read
	PARSER_POS_IN_LAYER,        // Starting layer tag read
	PARSER_POS_IN_STROKE,       // Starting layer tag read
	PARSER_POS_IN_TEXT,         // Starting text tag read
	PARSER_POS_IN_IMAGE,        // Starting image tag read
	PARSER_POS_IN_TEXIMAGE,     // Starting latex tag read

	PASER_POS_FINISHED          // Document is parsed
};

class ParseException : public std::runtime_error
{
public:
	ParseException(const char* attribute, bool notFound, string* value = NULL, string convError = "");
	ParseException(string msg);
	virtual ~ParseException();
	
	const char* getAttribute() const;
	string* getValue() const;
	bool isNotFound() const;
	string isConvError() const;
	
	virtual const char* what() const throw();
	
private:
	XOJ_TYPE_ATTRIB;
	
	string msg;
	
	const char* attribute;
	string* value;
	string convError;
	bool notFound;	
};

class LoadHandler
{
public:
	LoadHandler();
	virtual ~LoadHandler();

public:
	Document* loadDocument(path filename);

	string getLastError();
	bool isAttachedPdfMissing();
	path getMissingPdfFilename();

	void removePdfBackground();
	void setPdfReplacement(path filename, bool attachToDocument);

private:
	void initAttributes();

	bool openFile(path filename);
	bool closeFile();
	
	bool parseXml();
	
	void parseXmlXournal(xml_node* xml_xournal);
	
	void parseXmlPage(xml_node* xml_page);
	void parseXmlPageBgSolid(xml_node* xml_bg);
	void parseXmlPageBgPixmap(xml_node* xml_bg);
	void parseXmlPageBgPdf(xml_node* xml_bg);
	
	void parseXmlLayer(xml_node* xml_layer);
	void parseXmlLayerStroke(xml_node* child);
	void parseXmlLayerText(xml_node* child);
	void parseXmlLayerImage(xml_node* child);
	void parseXmlLayerTexImage(xml_node* child);

	int parseColor(const string& name);

private:
	XOJ_TYPE_ATTRIB;

	string lastError;
	path pdfMissing;
	bool attachedPdfMissing;

	bool removePdfBackgroundFlag;
	path pdfReplacementFilename;
	bool pdfReplacementAttach;

	path filename;

	bool pdfFilenameParsed;

	ParserPosition pos;

	string creator;
	int fileversion;

	ifstream file;
	bio::filtering_istreambuf inbuf;
	xml_document xml;

	PageRef page;
	Layer* layer;
	Stroke* stroke;
	TexImage* teximage;

	path xournalFilename;

	DocumentHandler dHanlder;
	Document doc;
};
