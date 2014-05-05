#include "LoadHandler.h"

#include "../model/XojPage.h"
#include "../model/BackgroundImage.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <stdlib.h>

#include <config.h>
#include <glib/gi18n-lib.h>

typedef struct
{
	const char* name;
	const int rgb;
} PredefinedColor;

PredefinedColor PREDEFINED_COLORS[] =
{
	{ "black", 0x000000 },
	{ "blue", 0x3333cc },
	{ "red", 0xff0000 },
	{ "green", 0x008000 },
	{ "gray", 0x808080 },
	{ "lightblue", 0x00c0ff },
	{ "lightgreen", 0x00ff00 },
	{ "magenta", 0xff00ff },
	{ "orange", 0xff8000 },
	{ "yellow", 0xffff00 },
	{ "white", 0xffffff }
};

const int COLOR_COUNT = sizeof(PREDEFINED_COLORS) / sizeof(PredefinedColor);

LoadHandler::LoadHandler() :
	doc(&dHanlder)
{
	XOJ_INIT_TYPE(LoadHandler);

	initAttributes();
	this->removePdfBackgroundFlag = false;
	this->pdfReplacementAttach = false;
}

LoadHandler::~LoadHandler()
{
	XOJ_RELEASE_TYPE(LoadHandler);
}

void LoadHandler::initAttributes()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->fp = NULL;
	this->error = NULL;
	this->attributeNames = NULL;
	this->attributeValues = NULL;
	this->elementName = NULL;
	this->pdfFilenameParsed = false;
	this->attachedPdfMissing = false;

	this->page = NULL;
	this->layer = NULL;
	this->stroke = NULL;
	this->image = NULL;
	this->teximage = NULL;
	this->text = NULL;
}

String LoadHandler::getLastError()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->lastError;
}

bool LoadHandler::isAttachedPdfMissing()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->attachedPdfMissing;
}

String LoadHandler::getMissingPdfFilename()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->pdfMissing;
}

void LoadHandler::removePdfBackground()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->removePdfBackgroundFlag = true;
}

void LoadHandler::setPdfReplacement(String filename, bool attachToDocument)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->pdfReplacementFilename = filename;
	this->pdfReplacementAttach = attachToDocument;
}

bool LoadHandler::openFile(String filename)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->filename = filename;
	this->fp = gzopen(filename.c_str(), "r");
	if (!this->fp)
	{
		this->lastError = _("Could not open file: \"");
		this->lastError += filename;
		this->lastError += "\"";
		return false;
	}
	return true;
}

bool LoadHandler::closeFile()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return gzclose(this->fp);
}

int LoadHandler::readFile(char* buffer, int len)
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (gzeof(this->fp))
	{
		return -1;
	}
	return gzread(this->fp, buffer, len);
}

#define error2(var, ...) if(var == NULL) var = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);
#define error(...) if(error == NULL) error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);

const char* LoadHandler::getAttrib(const char* name, bool optional)
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char** aName = this->attributeNames;
	const char** aValue = this->attributeValues;

	while (*aName != NULL)
	{
		if (!strcmp(*aName, name))
		{
			return *aValue;
		}
		aName++;
		aValue++;
	}

	if (!optional)
	{
		g_warning("Parser: attribute %s not found!", name);
	}
	return NULL;
}

double LoadHandler::getAttribDouble(const char* name)
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char* attrib = getAttrib(name);

	if (attrib == NULL)
	{
		error(_("Attribute \"%s\" could not be parsed as double, the value is NULL"),
		      name);
		return 0;
	}

	char* ptr = NULL;
	double val = g_ascii_strtod(attrib, &ptr);
	if (ptr == attrib)
	{
		error(_("Attribute \"%s\" could not be parsed as double, the value is \"%s\""),
		      name, attrib);
	}

	return val;
}

int LoadHandler::getAttribInt(const char* name)
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char* attrib = getAttrib(name);

	if (attrib == NULL)
	{
		error(_("Attribute \"%s\" could not be parsed as int, the value is NULL"),
		      name);
		return 0;
	}

	char* ptr = NULL;
	int val = strtol(attrib, &ptr, 10);
	if (ptr == attrib)
	{
		error(_("Attribute \"%s\" could not be parsed as int, the value is \"%s\""),
		      name, attrib);
	}

	return val;
}

void LoadHandler::parseStart()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (strcmp(elementName, "xournal") == 0)
	{
		// Read the document version

		const char* version = getAttrib("version", true);
		if (version)
		{
			this->creator = "Xournal ";
			this->creator += version;
		}

		const char* fileversion = getAttrib("fileversion", true);
		if (fileversion)
		{
			this->fileversion = atoi(fileversion);
		}
		const char* creator = getAttrib("creator", true);
		if (creator)
		{
			this->creator = creator;
		}

		this->pos = PARSER_POS_STARTED;
	}
	else
	{
		error(_("Unexpected root tag: %s"), elementName);
	}
}

void LoadHandler::parseContents()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (strcmp(elementName, "page") == 0)
	{
		this->pos = PARSER_POS_IN_PAGE;

		double width = getAttribDouble("width");
		double height = getAttribDouble("height");

		this->page = new XojPage(width, height);

		this->doc.addPage(this->page);
	}
	else if (strcmp(elementName, "title") == 0)
	{
		// Ignore this tag, it says nothing...
	}
	else if (strcmp(elementName, "preview") == 0)
	{
		// Ignore this tag, we don't need a preview
	}
	else
	{
		//error(_("Unexpected tag in document: \"%s\""), elementName);
	}
}
const char* RULINGSTR_NONE = "plain";
const char* RULINGSTR_LINED = "lined";
const char* RULINGSTR_RULED = "ruled";
const char* RULINGSTR_GRAPH = "graph";

void LoadHandler::parseBgSolid()
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char* style = getAttrib("style");

	if (strcmp("plain", style) == 0)
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
	}
	else if (strcmp("lined", style) == 0)
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_LINED);
	}
	else if (strcmp("ruled", style) == 0)
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_RULED);
	}
	else if (strcmp("graph", style) == 0)
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	}
	else
	{
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
		error(_("Unknown background type parsed: \"%s\""), style);
	}

	const char* sColor = getAttrib("color");
	int color = 0;

	if (strcmp("blue", sColor) == 0)
	{
		color = 0xa0e8ff;
	}
	else if (strcmp("pink", sColor) == 0)
	{
		color = 0xffc0d4;
	}
	else if (strcmp("green", sColor) == 0)
	{
		color = 0x80FFC0;
	}
	else if (strcmp("orange", sColor) == 0)
	{
		color = 0xFFC080;
	}
	else if (strcmp("yellow", sColor) == 0)
	{
		color = 0xFFFF80;
	}
	else if (strcmp("white", sColor) == 0)
	{
		color = 0xffffff;
	}
	else
	{
		parseColor(sColor, color);
	}

	this->page->setBackgroundColor(color);
}

void LoadHandler::parseBgPixmap()
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char* domain = getAttrib("domain");
	const char* filename = getAttrib("filename");

	String fileToLoad;
	bool loadFile = false;

	if (!strcmp(domain, "absolute"))
	{
		fileToLoad = filename;
		loadFile = true;
	}
	else if (!strcmp(domain, "attach"))
	{
		fileToLoad = this->filename;
		fileToLoad += ".";
		fileToLoad += filename;
		loadFile = true;
	}

	if (loadFile)
	{
		GError* error = NULL;
		BackgroundImage img;
		img.loadFile(fileToLoad, &error);

		if (error)
		{
			error(_("could not read image: %s, Error message: %s"), fileToLoad.c_str(),
			      error->message);
			g_error_free(error);
		}

		this->page->setBackgroundImage(img);
	}
	else if (!strcmp(domain, "clone"))
	{
		PageRef p = doc.getPage(atoi(filename));

		if (p.isValid())
		{
			this->page->setBackgroundImage(p->getBackgroundImage());
		}
	}
	else
	{
		error(_("Unknown pixmap::domain type: %s"), domain);
	}
	this->page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
}

void LoadHandler::parseBgPdf()
{
	XOJ_CHECK_TYPE(LoadHandler);

	int pageno = getAttribInt("pageno");
	bool attachToDocument = false;
	String pdfFilename;

	this->page->setBackgroundPdfPageNr(pageno - 1);

	if (!this->pdfFilenameParsed)
	{

		if (this->pdfReplacementFilename.isEmpty())
		{
			const char* domain = getAttrib("domain");
			const char* sFilename = getAttrib("filename");

			if (sFilename == NULL)
			{
				error("PDF Filename missing!");
				return;
			}

			pdfFilename = sFilename;

			if (!strcmp("absolute", domain))   // Absolute OR relative path
			{
				if (!g_file_test(sFilename, G_FILE_TEST_EXISTS))
				{
					char* dirname = g_path_get_dirname(xournalFilename.c_str());
					char* file = g_path_get_basename(sFilename);

					char* tmpFilename = g_build_path(G_DIR_SEPARATOR_S, dirname, file, NULL);

					if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS))
					{
						pdfFilename = tmpFilename;
					}

					g_free(tmpFilename);
					g_free(dirname);
					g_free(file);
				}
			}
			else if (!strcmp("attach", domain))
			{
				attachToDocument = true;
				char* tmpFilename = g_strdup_printf("%s.%s", xournalFilename.c_str(),
				                                    sFilename);

				if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS))
				{
					pdfFilename = tmpFilename;
				}

				g_free(tmpFilename);
			}
			else
			{
				error(_("Unknown domain type: %s"), domain);
				return;
			}

		}
		else
		{
			pdfFilename = this->pdfReplacementFilename;
			attachToDocument = this->pdfReplacementAttach;
		}

		this->pdfFilenameParsed = true;

		if (g_file_test(pdfFilename.c_str(), G_FILE_TEST_EXISTS))
		{
			doc.readPdf(pdfFilename, false, attachToDocument);
			if (!doc.getLastErrorMsg().isEmpty())
			{
				error(_("Error reading PDF: %s"), doc.getLastErrorMsg().c_str());
			}
		}
		else
		{
			if (attachToDocument)
			{
				this->attachedPdfMissing = true;
			}
			else
			{
				this->pdfMissing = pdfFilename.c_str();
			}
		}
	}
}

void LoadHandler::parsePage()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (!strcmp(elementName, "background"))
	{
		const char* type = getAttrib("type");

		if (strcmp("solid", type) == 0)
		{
			parseBgSolid();
		}
		else if (strcmp("pixmap", type) == 0)
		{
			parseBgPixmap();
		}
		else if (strcmp("pdf", type) == 0)
		{
			if (this->removePdfBackgroundFlag)
			{
				this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
				this->page->setBackgroundColor(0xffffff);
			}
			else
			{
				parseBgPdf();
			}
		}
		else
		{
			error(_("Unknown background type: %s"), type);
		}
	}
	else if (!strcmp(elementName, "layer"))
	{
		this->pos = PARSER_POS_IN_LAYER;
		this->layer = new Layer();
		this->page->addLayer(this->layer);
	}
}

bool LoadHandler::parseColor(const char* text, int& color)
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (text == NULL)
	{
		error(_("Attribute color not set!"));
		return false;
	}

	if (text[0] == '#')
	{
		gchar* ptr = NULL;
		int c = g_ascii_strtoull(&text[1], &ptr, 16);
		if (ptr != text + strlen(text))
		{
			error(_("Unknown color value \"%s\""), text);
			return false;
		}

		color = c >> 8;

		return true;
	}
	else
	{
		for (int i = 0; i < COLOR_COUNT; i++)
		{
			if (!strcmp(text, PREDEFINED_COLORS[i].name))
			{
				color = PREDEFINED_COLORS[i].rgb;
				return true;
			}
		}
		error(_("Color \"%s\" unknown (not defined in default color list)!"), text);
		return false;
	}
}

void LoadHandler::parseStroke()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->stroke = new Stroke();
	this->layer->addElement(this->stroke);

	const char* width = getAttrib("width");
	char* ptr = NULL;
	char* tmpptr = NULL;
	double val = 0;

	stroke->setWidth(g_ascii_strtod(width, &ptr));
	if (ptr == width)
	{
		error(_("Error reading width of a stroke: %s"), width);
		return;
	}

	while (*ptr != 0)
	{
		val = g_ascii_strtod(ptr, &tmpptr);
		if (tmpptr == ptr)
		{
			break;
		}
		ptr = tmpptr;
		this->pressureBuffer.add(val);
	}

	int color = 0;
	const char* sColor = getAttrib("color");
	if (!parseColor(sColor, color))
	{
		return;
	}
	stroke->setColor(color);

	const char* tool = getAttrib("tool");

	if (strcmp("eraser", tool) == 0)
	{
		stroke->setToolType(STROKE_TOOL_ERASER);
	}
	else if (strcmp("pen", tool) == 0)
	{
		stroke->setToolType(STROKE_TOOL_PEN);
	}
	else if (strcmp("highlighter", tool) == 0)
	{
		stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
	}
	else
	{
		g_warning(_("Unknown stroke type: \"%s\", assume pen"), tool);
	}
}

void LoadHandler::parseText()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->text = new Text();
	this->layer->addElement(this->text);

	const char* sFont = getAttrib("font");
	double fontSize = getAttribDouble("size");
	double x = getAttribDouble("x");
	double y = getAttribDouble("y");

	this->text->setX(x);
	this->text->setY(y);

	XojFont& f = text->getFont();
	f.setName(sFont);
	f.setSize(fontSize);
	const char* sColor = getAttrib("color");
	int color = 0;
	parseColor(sColor, color);
	text->setColor(color);
}

void LoadHandler::parseImage()
{
	XOJ_CHECK_TYPE(LoadHandler);

	double left = getAttribDouble("left");
	double top = getAttribDouble("top");
	double right = getAttribDouble("right");
	double bottom = getAttribDouble("bottom");

	this->image = new Image();
	this->layer->addElement(this->image);
	this->image->setX(left);
	this->image->setY(top);
	this->image->setWidth(right - left);
	this->image->setHeight(bottom - top);
}

void LoadHandler::parseTexImage()
{
	XOJ_CHECK_TYPE(LoadHandler);

	double left = getAttribDouble("left");
	double top = getAttribDouble("top");
	double right = getAttribDouble("right");
	double bottom = getAttribDouble("bottom");

	const char* imText = getAttrib("text");
	const char* compatibilityTest = getAttrib("texlength");
	int imTextLen = 500;
	if(compatibilityTest != NULL)
	{
		imTextLen = getAttribInt("texlength");
	}


	this->teximage = new TexImage();
	this->layer->addElement(this->teximage);
	this->teximage->setX(left);
	this->teximage->setY(top);
	this->teximage->setWidth(right - left);
	this->teximage->setHeight(bottom - top);
	//need to allocate memory for the image
	char* tmp = new char[imTextLen + 1];
	strcpy(tmp, imText);
	this->teximage->setText(tmp, imTextLen);
}

void LoadHandler::parseLayer()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (!strcmp(elementName, "stroke"))   // start of a stroke
	{
		this->pos = PARSER_POS_IN_STROKE;
		parseStroke();
	}
	else if (!strcmp(elementName, "text"))     // start of a text item
	{
		this->pos = PARSER_POS_IN_TEXT;
		parseText();
	}
	else if (!strcmp(elementName, "image"))     // start of a image item
	{
		this->pos = PARSER_POS_IN_IMAGE;
		parseImage();
	}
	else if (!strcmp(elementName, "teximage"))     // start of a image item
	{
		this->pos = PARSER_POS_IN_TEXIMAGE;
		parseTexImage();
	}
}

void LoadHandler::parserStartElement(GMarkupParseContext* context,
                                     const gchar* elementName, const gchar** attributeNames,
                                     const gchar** attributeValues,
                                     gpointer userdata, GError** error)
{
	LoadHandler* handler = (LoadHandler*) userdata;
	// Return on error
	if (*error)
	{
		return;
	}

	XOJ_CHECK_TYPE_OBJ(handler, LoadHandler);


	handler->attributeNames = attributeNames;
	handler->attributeValues = attributeValues;
	handler->elementName = elementName;

	if (handler->pos == PARSER_POS_NOT_STARTED)
	{
		handler->parseStart();
	}
	else if (handler->pos == PARSER_POS_STARTED)
	{
		handler->parseContents();
	}
	else if (handler->pos == PARSER_POS_IN_PAGE)
	{
		handler->parsePage();
	}
	else if (handler->pos == PARSER_POS_IN_LAYER)
	{
		handler->parseLayer();
	}

	handler->attributeNames = NULL;
	handler->attributeValues = NULL;
	handler->elementName = NULL;
}

void LoadHandler::parserEndElement(GMarkupParseContext* context,
                                   const gchar* element_name, gpointer userdata, GError** error)
{
	// Return on error
	if (*error)
	{
		return;
	}

	LoadHandler* handler = (LoadHandler*) userdata;

	XOJ_CHECK_TYPE_OBJ(handler, LoadHandler);


	if (handler->pos == PARSER_POS_STARTED && strcmp(element_name, "xournal") == 0)
	{
		handler->pos = PASER_POS_FINISHED;
	}
	else if (handler->pos == PARSER_POS_IN_PAGE &&
	         strcmp(element_name, "page") == 0)
	{
		handler->pos = PARSER_POS_STARTED;
		handler->page = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_LAYER &&
	         strcmp(element_name, "layer") == 0)
	{
		handler->pos = PARSER_POS_IN_PAGE;
		handler->layer = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_STROKE &&
	         strcmp(element_name, "stroke") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->stroke = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_TEXT &&
	         strcmp(element_name, "text") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->text = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_IMAGE &&
	         strcmp(element_name, "image") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->image = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_TEXIMAGE &&
	         strcmp(element_name, "teximage") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->teximage = NULL;
	}
}

void LoadHandler::parserText(GMarkupParseContext* context, const gchar* text,
                             gsize textLen, gpointer userdata, GError** error)
{
	// Return on error
	if (*error)
	{
		return;
	}

	LoadHandler* handler = (LoadHandler*) userdata;

	XOJ_CHECK_TYPE_OBJ(handler, LoadHandler);


	if (handler->pos == PARSER_POS_IN_STROKE)
	{
		const char* ptr = text;
		int n = 0;

		bool xRead = false;
		double x = 0;

		while (textLen > 0)
		{
			double tmp = g_ascii_strtod(text, (char**) (&ptr));
			if (ptr == text)
			{
				break;
			}
			textLen -= (ptr - text);
			text = ptr;
			n++;

			if (!xRead)
			{
				xRead = true;
				x = tmp;
			}
			else
			{
				xRead = false;
				handler->stroke->addPoint(Point(x, tmp));
			}
		}
		handler->stroke->freeUnusedPointItems();

		if (n < 4 || n & 1)
		{
			error2(*error, _("Wrong count of points (%i)"), n);
			return;
		}

		if (handler->pressureBuffer.size() != 0)
		{
			if (handler->pressureBuffer.size() == handler->stroke->getPointCount() - 1)
			{
				const double* data = handler->pressureBuffer.getData();
				handler->stroke->setPressure(data);
				handler->pressureBuffer.clear();
			}
			else
			{
				g_warning(_("xoj-File: %s"), handler->filename.c_str());
				g_warning(_("Wrong count of points, get %i, expected %i"),
				          handler->pressureBuffer.size(), handler->stroke->getPointCount() - 1);
			}
		}
		handler->pressureBuffer.clear();
	}
	else if (handler->pos == PARSER_POS_IN_TEXT)
	{
		gchar* txt = g_strndup(text, textLen);
		handler->text->setText(txt);
		g_free(txt);
	}
	else if (handler->pos == PARSER_POS_IN_IMAGE)
	{
		handler->readImage(text, textLen);
	}
	else if (handler->pos == PARSER_POS_IN_TEXIMAGE)
	{
		handler->readTexImage(text, textLen);
	}
}

void LoadHandler::readImage(const gchar* base64_str, gsize base64_strlen)
{
	XOJ_CHECK_TYPE(LoadHandler);

	gsize png_buflen;

	// We have to copy the string in order to null terminate it, sigh.
	gchar* base64_str2 = (gchar*) g_memdup(base64_str, base64_strlen + 1);
	base64_str2[base64_strlen] = '\0';

	guchar* png_buf = g_base64_decode(base64_str2, &png_buflen);
	g_free(base64_str2);

	this->image->setImage(png_buf, png_buflen);
}

void LoadHandler::readTexImage(const gchar* base64_str, gsize base64_strlen)
{
	XOJ_CHECK_TYPE(LoadHandler);

	gsize png_buflen;

	// We have to copy the string in order to null terminate it, sigh.
	gchar* base64_str2 = (gchar*) g_memdup(base64_str, base64_strlen + 1);
	base64_str2[base64_strlen] = '\0';

	guchar* png_buf = g_base64_decode(base64_str2, &png_buflen);
	g_free(base64_str2);

	this->teximage->setImage(png_buf, png_buflen);
}


bool LoadHandler::parseXml()
{
	XOJ_CHECK_TYPE(LoadHandler);

	const GMarkupParser parser = { LoadHandler::parserStartElement, LoadHandler::parserEndElement, LoadHandler::parserText, NULL, NULL };
	GMarkupParseContext* context;
	this->error = NULL;
	gboolean valid = true;

	this->pos = PARSER_POS_NOT_STARTED;
	this->creator = "Unknown";
	this->fileversion = 1;

	char buffer[1024];
	int len;

	context = g_markup_parse_context_new(&parser, (GMarkupParseFlags) 0, this,
	                                     NULL);

	do
	{
		len = readFile(buffer, sizeof(buffer));
		if (len > 0)
		{
			valid = g_markup_parse_context_parse(context, buffer, len, &error);
		}

		if (error)
		{
			g_warning("LoadHandler::parseXml: %s\n", error->message);
			valid = false;
			break;
		}
	}
	while (len >= 0 && valid && !error);

	if (valid)
	{
		valid = g_markup_parse_context_end_parse(context, &error);
	}
	else
	{
		if (error != NULL && error->message != NULL)
		{
			char* err = g_strdup_printf(_("XML Parser error: %s"), error->message);
			this->lastError = err;
			g_free(err);

			g_error_free(error);
		}
		else
		{
			this->lastError = _("Unknown parser error");
		}
		g_warning("LoadHandler::parseXml: %s\n", this->lastError.c_str());
	}

	g_markup_parse_context_free(context);

	if (this->pos != PASER_POS_FINISHED && this->lastError == NULL)
	{
		lastError = _("Document is not complete (maybe the end is cut off?)");
		return false;
	}

	//	printf("parsed creator: %s::version: %i\n", this->creator.c_str(), this->fileversion);
	doc.setCreateBackupOnSave(!this->fileversion < 2);

	return valid;
}

/**
 * Document should not be freed, it will be freed with LoadHandler!
 */
Document* LoadHandler::loadDocument(String filename)
{
	XOJ_CHECK_TYPE(LoadHandler);

	initAttributes();
	doc.clearDocument();

	if (!openFile(filename))
	{
		return NULL;
	}

	xournalFilename = filename;

	this->pdfFilenameParsed = false;

	if (!parseXml())
	{
		closeFile();
		return NULL;
	}
	doc.setFilename(filename.c_str());

	closeFile();

	return &this->doc;
}

DoubleArrayBuffer::DoubleArrayBuffer()
{
	XOJ_INIT_TYPE(DoubleArrayBuffer);

	this->data = NULL;
	this->len = 0;
	this->allocCount = 0;
}

DoubleArrayBuffer::~DoubleArrayBuffer()
{
	XOJ_CHECK_TYPE(DoubleArrayBuffer);

	g_free(this->data);
	this->data = NULL;
	this->len = 0;
	this->allocCount = 0;

	XOJ_RELEASE_TYPE(DoubleArrayBuffer);
}

void DoubleArrayBuffer::clear()
{
	XOJ_CHECK_TYPE(DoubleArrayBuffer);

	this->len = 0;
}

const double* DoubleArrayBuffer::getData()
{
	XOJ_CHECK_TYPE(DoubleArrayBuffer);

	return this->data;
}

int DoubleArrayBuffer::size()
{
	XOJ_CHECK_TYPE(DoubleArrayBuffer);

	return this->len;
}

void DoubleArrayBuffer::add(double d)
{
	XOJ_CHECK_TYPE(DoubleArrayBuffer);

	if (this->len >= this->allocCount)
	{
		this->allocCount += 1024;
		this->data = (double*) g_realloc(this->data, this->allocCount * sizeof(double));
	}

	this->data[this->len++] = d;
}

