#include "LoadHandler.h"

#include "model/XojPage.h"
#include "model/BackgroundImage.h"
#include "LoadHandlerHelper.h"

#include <config.h>
#include <GzUtil.h>
#include <i18n.h>

#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <stdlib.h>

#define error2(var, ...)																	\
	if (var == NULL)																		\
	{																						\
		var = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);		\
	}

#define error(...)																			\
	if (error == NULL)																		\
	{																						\
		error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);	\
	}

LoadHandler::LoadHandler()
 : attachedPdfMissing(false),
   removePdfBackgroundFlag(false),
   pdfReplacementAttach(false),
   pdfFilenameParsed(false),
   pos(PARSER_POS_NOT_STARTED),
   fileversion(0),
   fp(NULL),
   layer(NULL),
   stroke(NULL),
   text(NULL),
   image(NULL),
   teximage(NULL),
   attributeNames(NULL),
   attributeValues(NULL),
   elementName(NULL),
   loadedTimeStamp(0),
   doc(&dHanlder)
{
	XOJ_INIT_TYPE(LoadHandler);

	this->error = NULL;

	initAttributes();
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

string LoadHandler::getLastError()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->lastError;
}

bool LoadHandler::isAttachedPdfMissing()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->attachedPdfMissing;
}

string LoadHandler::getMissingPdfFilename()
{
	XOJ_CHECK_TYPE(LoadHandler);

	return this->pdfMissing;
}

void LoadHandler::removePdfBackground()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->removePdfBackgroundFlag = true;
}

void LoadHandler::setPdfReplacement(string filename, bool attachToDocument)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->pdfReplacementFilename = filename;
	this->pdfReplacementAttach = attachToDocument;
}

bool LoadHandler::openFile(string filename)
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->filename = filename;
	this->fp = GzUtil::openPath(filename, "r");
	if (!this->fp)
	{
		this->lastError = FS(_F("Could not open file: \"{1}\"") % filename);
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

bool LoadHandler::parseXml()
{
	XOJ_CHECK_TYPE(LoadHandler);

	const GMarkupParser parser = { LoadHandler::parserStartElement, LoadHandler::parserEndElement, LoadHandler::parserText, NULL, NULL };
	this->error = NULL;
	gboolean valid = true;

	this->pos = PARSER_POS_NOT_STARTED;
	this->creator = "Unknown";
	this->fileversion = 1;

	GMarkupParseContext* context = g_markup_parse_context_new(&parser, (GMarkupParseFlags) 0, this, NULL);

	int len = 0;
	do
	{
		char buffer[1024];
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
			this->lastError = FS(_F("XML Parser error: {1}") % error->message);
			g_error_free(error);
		}
		else
		{
			this->lastError = _("Unknown parser error");
		}
		g_warning("LoadHandler::parseXml: %s\n", this->lastError.c_str());
	}

	g_markup_parse_context_free(context);

	if (this->pos != PASER_POS_FINISHED && this->lastError == "")
	{
		lastError = _("Document is not complete (maybe the end is cut off?)");
		return false;
	}

	doc.setCreateBackupOnSave(this->fileversion >= 3);

	return valid;
}

void LoadHandler::parseStart()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (strcmp(elementName, "xournal") == 0)
	{
		endRootTag = "xournal";

		// Read the document version
		const char* version = LoadHandlerHelper::getAttrib("version", true, this);
		if (version)
		{
			this->creator = "Xournal ";
			this->creator += version;
		}

		const char* fileversion = LoadHandlerHelper::getAttrib("fileversion", true, this);
		if (fileversion)
		{
			this->fileversion = atoi(fileversion);
		}
		const char* creator = LoadHandlerHelper::getAttrib("creator", true, this);
		if (creator)
		{
			this->creator = creator;
		}

		this->pos = PARSER_POS_STARTED;
	}
	else if (strcmp(elementName, "MrWriter") == 0)
	{
		endRootTag = "MrWriter";

		// Read the document version
		const char* version = LoadHandlerHelper::getAttrib("version", true, this);
		if (version)
		{
			this->creator = "MrWriter ";
			this->creator += version;
		}

		// Document version 1:
		// Handle it the same as a Xournal document, and don't allow to overwrite
		this->fileversion = 1;
		this->pos = PARSER_POS_STARTED;
	}
	else
	{
		error("%s", FC(_F("Unexpected root tag: {1}") % elementName));
	}
}

void LoadHandler::parseContents()
{
	XOJ_CHECK_TYPE(LoadHandler);

	if (strcmp(elementName, "page") == 0)
	{
		this->pos = PARSER_POS_IN_PAGE;

		double width = LoadHandlerHelper::getAttribDouble("width", this);
		double height = LoadHandlerHelper::getAttribDouble("height", this);

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
		g_warning("%s", FC(_F("Unexpected tag in document: \"{1}\"") % elementName));
	}
}

void LoadHandler::parseBgSolid()
{
	XOJ_CHECK_TYPE(LoadHandler);

	PageType bg;
	const char* style = LoadHandlerHelper::getAttrib("style", false, this);
	if (style != NULL)
	{
		bg.format = style;
	}

	const char* config = LoadHandlerHelper::getAttrib("config", true, this);
	if (config != NULL)
	{
		bg.config = config;
	}

	this->page->setBackgroundType(bg);

	int color = LoadHandlerHelper::parseBackgroundColor(this);
	this->page->setBackgroundColor(color);
}

void LoadHandler::parseBgPixmap()
{
	XOJ_CHECK_TYPE(LoadHandler);

	const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
	const char* filename = LoadHandlerHelper::getAttrib("filename", false, this);

	string fileToLoad;
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
			error("%s", FC(_F("Could not read image: {1}. Error message: {2}") % fileToLoad % error->message));
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
		error("%s", FC(_F("Unknown pixmap::domain type: {1}") % domain));
	}

	this->page->setBackgroundType(PageType(":image"));
}

void LoadHandler::parseBgPdf()
{
	XOJ_CHECK_TYPE(LoadHandler);

	int pageno = LoadHandlerHelper::getAttribInt("pageno", this);
	bool attachToDocument = false;
	string pdfFilename;

	this->page->setBackgroundPdfPageNr(pageno - 1);

	if (!this->pdfFilenameParsed)
	{

		if (this->pdfReplacementFilename == "")
		{
			const char* domain = LoadHandlerHelper::getAttrib("domain", false, this);
			const char* sFilename = LoadHandlerHelper::getAttrib("filename", false, this);

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
				char* tmpFilename = g_strdup_printf("%s.%s", xournalFilename.c_str(), sFilename);

				if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS))
				{
					pdfFilename = tmpFilename;
				}

				g_free(tmpFilename);
			}
			else
			{
				error("%s", FC(_F("Unknown domain type: {1}") % domain));
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
			if (doc.getLastErrorMsg() != "")
			{
				error("%s", FC(_F("Error reading PDF: {1}") % doc.getLastErrorMsg()));
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
		const char* type = LoadHandlerHelper::getAttrib("type", false, this);

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
				this->page->setBackgroundType(PageType("plain"));
				this->page->setBackgroundColor(0xffffff);
			}
			else
			{
				parseBgPdf();
			}
		}
		else
		{
			error("%s", FC(_F("Unknown background type: {1}") % type));
		}
	}
	else if (!strcmp(elementName, "layer"))
	{
		this->pos = PARSER_POS_IN_LAYER;
		this->layer = new Layer();
		this->page->addLayer(this->layer);
	}
}

void LoadHandler::parseStroke()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->stroke = new Stroke();
	this->layer->addElement(this->stroke);

	const char* width = LoadHandlerHelper::getAttrib("width", false, this);

	char* endPtr = NULL;
	stroke->setWidth(g_ascii_strtod(width, &endPtr));
	if (endPtr == width)
	{
		error("%s", FC(_F("Error reading width of a stroke: {1}") % width));
		return;
	}

	const char* pressure = LoadHandlerHelper::getAttrib("pressures", true, this);
	if (pressure == NULL)
	{
		pressure = endPtr;
	}

	while (*pressure != 0)
	{
		char* tmpptr = NULL;
		double val = g_ascii_strtod(pressure, &tmpptr);
		if (tmpptr == pressure)
		{
			break;
		}
		pressure = tmpptr;
		this->pressureBuffer.push_back(val);
	}

	int color = 0;
	const char* sColor = LoadHandlerHelper::getAttrib("color", false, this);
	if (!LoadHandlerHelper::parseColor(sColor, color, this))
	{
		return;
	}
	stroke->setColor(color);

	/** read stroke timestamps (xopp fileformat) */
	const char* fn = LoadHandlerHelper::getAttrib("fn", true, this);
	if (fn != NULL)
	{
		stroke->setAudioFilename(fn);
	}

	int ts = 0;
	if (LoadHandlerHelper::getAttribInt("ts", true, this, ts))
	{
		stroke->setTimestamp(ts);
	}

	int fill = -1;
	if (LoadHandlerHelper::getAttribInt("fill", true, this, fill))
	{
		stroke->setFill(fill);
	}

	const char* tool = LoadHandlerHelper::getAttrib("tool", false, this);

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
		g_warning("%s", FC(_F("Unknown stroke type: \"{1}\", assuming pen") % tool));
	}

	/**
	 * For each stroke being read, set the timestamp value
	 * we've read just before. 
	 * Afterwards, clean the read timestamp data.
	 */
	if (loadedFilename.length() != 0)
	{
		this->stroke->setTimestamp(loadedTimeStamp);
		this->stroke->setAudioFilename(loadedFilename);
		loadedFilename = "";
		loadedTimeStamp = 0;
	}
}

void LoadHandler::parseText()
{
	XOJ_CHECK_TYPE(LoadHandler);

	this->text = new Text();
	this->layer->addElement(this->text);

	const char* sFont = LoadHandlerHelper::getAttrib("font", false, this);
	double fontSize = LoadHandlerHelper::getAttribDouble("size", this);
	double x = LoadHandlerHelper::getAttribDouble("x", this);
	double y = LoadHandlerHelper::getAttribDouble("y", this);

	this->text->setX(x);
	this->text->setY(y);

	XojFont& f = text->getFont();
	f.setName(sFont);
	f.setSize(fontSize);
	const char* sColor = LoadHandlerHelper::getAttrib("color", false, this);
	int color = 0;
	LoadHandlerHelper::parseColor(sColor, color, this);
	text->setColor(color);
}

void LoadHandler::parseImage()
{
	XOJ_CHECK_TYPE(LoadHandler);

	double left = LoadHandlerHelper::getAttribDouble("left", this);
	double top = LoadHandlerHelper::getAttribDouble("top", this);
	double right = LoadHandlerHelper::getAttribDouble("right", this);
	double bottom = LoadHandlerHelper::getAttribDouble("bottom", this);

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

	double left = LoadHandlerHelper::getAttribDouble("left", this);
	double top = LoadHandlerHelper::getAttribDouble("top", this);
	double right = LoadHandlerHelper::getAttribDouble("right", this);
	double bottom = LoadHandlerHelper::getAttribDouble("bottom", this);

	const char* imText = LoadHandlerHelper::getAttrib("text", false, this);
	const char* compatibilityTest = LoadHandlerHelper::getAttrib("texlength", false, this);
	int imTextLen = strlen(imText);
	if(compatibilityTest != NULL)
	{
		imTextLen = LoadHandlerHelper::getAttribInt("texlength", this);
	}

	this->teximage = new TexImage();
	this->layer->addElement(this->teximage);
	this->teximage->setX(left);
	this->teximage->setY(top);
	this->teximage->setWidth(right - left);
	this->teximage->setHeight(bottom - top);

	this->teximage->setText(string(imText, imTextLen));
}

void LoadHandler::parseLayer()
{
	XOJ_CHECK_TYPE(LoadHandler);

	/** 
	 * read the timestamp before each stroke. 
	 * Used for backwards compatibility 
	 * against xoj files with timestamps) 
	 **/
	if (!strcmp(elementName, "timestamp"))
	{
		loadedTimeStamp = LoadHandlerHelper::getAttribInt("ts", this);
		loadedFilename = LoadHandlerHelper::getAttrib("fn", false, this);
	}
	if (!strcmp(elementName, "stroke")) // start of a stroke
	{
		this->pos = PARSER_POS_IN_STROKE;
		parseStroke();
	}
	else if (!strcmp(elementName, "text")) // start of a text item
	{
		this->pos = PARSER_POS_IN_TEXT;
		parseText();
	}
	else if (!strcmp(elementName, "image")) // start of a image item
	{
		this->pos = PARSER_POS_IN_IMAGE;
		parseImage();
	}
	else if (!strcmp(elementName, "teximage")) // start of a image item
	{
		this->pos = PARSER_POS_IN_TEXIMAGE;
		parseTexImage();
	}
}

void LoadHandler::parserStartElement(GMarkupParseContext* context, const gchar* elementName, const gchar** attributeNames,
                                     const gchar** attributeValues, gpointer userdata, GError** error)
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

void LoadHandler::parserEndElement(GMarkupParseContext* context, const gchar* elementName,
								   gpointer userdata, GError** error)
{
	// Return on error
	if (*error)
	{
		return;
	}

	LoadHandler* handler = (LoadHandler*) userdata;
	XOJ_CHECK_TYPE_OBJ(handler, LoadHandler);

	if (handler->pos == PARSER_POS_STARTED && strcmp(elementName, handler->endRootTag) == 0)
	{
		handler->pos = PASER_POS_FINISHED;
	}
	else if (handler->pos == PARSER_POS_IN_PAGE && strcmp(elementName, "page") == 0)
	{
		handler->pos = PARSER_POS_STARTED;
		handler->page = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "layer") == 0)
	{
		handler->pos = PARSER_POS_IN_PAGE;
		handler->layer = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(elementName, "timestamp") == 0)
	{
		/** Used for backwards compatibility against xoj files with timestamps) */
		handler->pos = PARSER_POS_IN_LAYER;
		handler->stroke = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_STROKE && strcmp(elementName, "stroke") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->stroke = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_TEXT && strcmp(elementName, "text") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->text = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_IMAGE && strcmp(elementName, "image") == 0)
	{
		handler->pos = PARSER_POS_IN_LAYER;
		handler->image = NULL;
	}
	else if (handler->pos == PARSER_POS_IN_TEXIMAGE && strcmp(elementName, "teximage") == 0)
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

		if (n < 4 || (n & 1))
		{
			error2(*error, "%s", FC(_F("Wrong count of points ({1})") % n));
			return;
		}

		if (handler->pressureBuffer.size() != 0)
		{
			if (handler->pressureBuffer.size() >= handler->stroke->getPointCount() - 1)
			{
				handler->stroke->setPressure(handler->pressureBuffer);
				handler->pressureBuffer.clear();
			}
			else
			{
				g_warning("%s", FC(_F("xoj-File: {1}") % handler->filename));
				g_warning("%s", FC(_F("Wrong number of points, got {1}, expected {2}")
										% handler->pressureBuffer.size() % (handler->stroke->getPointCount() - 1)));
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

	this->image->setImage(string((char*)png_buf, png_buflen));
	g_free(png_buf);
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

	this->teximage->setImage(string((char*)png_buf, png_buflen));
	g_free(png_buf);
}

/**
 * Document should not be freed, it will be freed with LoadHandler!
 */
Document* LoadHandler::loadDocument(string filename)
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

	if (fileversion == 1)
	{
		// This is a Xournal document, not a Xournal++
		// Even if the new fileextension is .xopp, allow to
		// overwrite .xoj files which are created by Xournal++
		// Force the user to save is a bad idea, this will annoy the user
		// Rename files is also not that user friendly.

		doc.setFilename("");
	}
	else
	{
		doc.setFilename(filename.c_str());
	}

	closeFile();

	return &this->doc;
}

