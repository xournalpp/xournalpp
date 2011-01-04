#include "LoadHandler.h"
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "../gettext.h"
#include <string.h>
#include <stdlib.h>

const char *color_names[COLOR_MAX] = { "black", "blue", "red", "green", "gray", "lightblue", "lightgreen", "magenta",
		"orange", "yellow", "white" };
unsigned int predef_colors_rgba[COLOR_MAX] = { 0x000000, 0x3333cc, 0xff0000, 0x008000, 0x808080, 0x00c0ff, 0x00ff00,
		0xff00ff, 0xff8000, 0xffff00, 0xffffff };

LoadHandler::LoadHandler() :
	doc(&dHanlder) {
	fp = NULL;
	error = NULL;
	attributeNames = NULL;
	attributeValues = NULL;
	elementName = NULL;

	page = NULL;
	layer = NULL;
	stroke = NULL;
	image = NULL;
}

LoadHandler::~LoadHandler() {
}

String LoadHandler::getLastError() {
	return lastError;
}

bool LoadHandler::openFile(String filename) {
	fp = gzopen(filename.c_str(), "r");
	if (!fp) {
		lastError = _("Could not open file");
		return false;
	}
	return true;
}

// TODO: use this
//GFile * file = g_file_new_for_uri(filename.c_str());
//if (!file) {
//	lastError = _("Could not open file");
//	return false;
//}
//if (!g_file_query_exists(file, NULL)) {
//	lastError = _("File does not exists!");
//	return false;
//}
//
//fileContents = NULL;
//fileLength = 0;
//GError * error = NULL;
//
//bool read = g_file_load_contents(file, NULL, (char **)&fileContents, &fileLength, NULL, &error);
//if (!read) {
//	lastError = _("Could not read file: \"");
//	lastError += error->message;
//	lastError += "\"";
//	g_error_free(error);
//	g_object_unref(file);
//	return false;
//}
//
//g_object_unref(file);
//
//return true;

bool LoadHandler::closeFile() {
	return gzclose(fp);
}

int LoadHandler::readFile(char * buffer, int len) {
	if (gzeof(fp)) {
		return -1;
	}
	return gzread(fp, buffer, len);
}

#define error2(var, ...) if(var == NULL) var = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);
#define error(...) if(error == NULL) error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT, __VA_ARGS__);

const char * LoadHandler::getAttrib(const char * name) {
	const gchar ** aName = attributeNames;
	const gchar ** aValue = attributeValues;

	while (*aName != NULL) {
		if (!strcmp(*aName, name)) {
			return *aValue;
		}
		aName++;
		aValue++;
	}

	g_warning("Parser: attribute %s not found!", name);
	return NULL;
}

double LoadHandler::getAttribDouble(const char * name) {
	const char * attrib = getAttrib(name);

	if (attrib == NULL) {
		error("Attribute \"%s\" could not be parsed as double, the value is NULL", name);
		return 0;
	}

	char * ptr = NULL;
	double val = g_ascii_strtod(attrib, &ptr);
	if (ptr == attrib) {
		error("Attribute \"%s\" could not be parsed as double, the value is \"%s\"", name, attrib);
	}

	return val;
}

int LoadHandler::getAttribInt(const char * name) {
	const char * attrib = getAttrib(name);

	if (attrib == NULL) {
		error("Attribute \"%s\" could not be parsed as int, the value is NULL", name);
		return 0;
	}

	char * ptr = NULL;
	int val = strtol(attrib, &ptr, 10);
	if (ptr == attrib) {
		error("Attribute \"%s\" could not be parsed as int, the value is \"%s\"", name, attrib);
	}

	return val;
}

void LoadHandler::parseStart() {
	if (strcmp(elementName, "xournal") == 0) {
		// Read the document version, and if its an extended file

		const char * version = getAttrib("version");
		if (version) {
			this->creatorVersion = version;
		} else {
			error("<xournal> without attribute version!");
		}

		const char * extended = getAttrib("extended");
		if (extended) {
			// Overwrite xournal version, don't matter in this case...
			this->creatorVersion = extended;
			this->isXournalExtended = true;
		}

		this->pos = PARSER_POS_STARTED;
	} else {
		error("Unexpected root tag: %s", elementName);
	}
}

void LoadHandler::parseContents() {
	if (strcmp(elementName, "page") == 0) {
		this->pos = PARSER_POS_IN_PAGE;
		this->page = new XojPage();
		this->page->reference();

		double width = getAttribDouble("width");
		double height = getAttribDouble("height");

		this->doc.setPageSize(page, width, height);

		this->doc.addPage(this->page);
	} else if (strcmp(elementName, "title") == 0) {
		// Ignore this tag, it says nothing...
	} else {
		error("Unexpected tag in document: \"%s\"", elementName);
	}
}
const char * RULINGSTR_NONE = "plain";
const char * RULINGSTR_LINED = "lined";
const char * RULINGSTR_RULED = "ruled";
const char * RULINGSTR_GRAPH = "graph";

void LoadHandler::parseBgSolid() {
	const char * style = getAttrib("style");

	if (strcmp("plain", style) == 0) {
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (strcmp("lined", style) == 0) {
		this->page->setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (strcmp("ruled", style) == 0) {
		this->page->setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (strcmp("graph", style) == 0) {
		this->page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else {
		this->page->setBackgroundType(BACKGROUND_TYPE_NONE);
		error ("Unknown ruling type parsed: %s", style);
	}

	const char * sColor = getAttrib("color");
	int color = 0;

	if (strcmp("blue", sColor) == 0) {
		color = 0xa0e8ff;
	} else if (strcmp("pink", sColor) == 0) {
		color = 0xffc0d4;
	} else if (strcmp("green", sColor) == 0) {
		color = 0x80FFC0;
	} else if (strcmp("orange", sColor) == 0) {
		color = 0xFFC080;
	} else if (strcmp("yellow", sColor) == 0) {
		color = 0xFFFF80;
	} else if (strcmp("white", sColor) == 0) {
		color = 0xffffff;
	} else {
		parseColor(sColor, color);
	}

	this->page->setBackgroundColor(color);
}

void LoadHandler::parseBgPixmap() {
	//TODO:

}

void LoadHandler::parseBgPdf() {
	int pageno = getAttribInt("pageno");

	if (!doc.isPdfDocumentLoaded()) {
		const char * domain = getAttrib("domain");
		const char * sFilename = getAttrib("filename");
		String filename;

		if (sFilename == NULL) {
			error("PDF Filename missing!");
			return;
		}

		filename = sFilename;

		if (!strcmp("absolute", domain)) { // Absolute OR relative path
			if (!g_file_test(sFilename, G_FILE_TEST_EXISTS)) {
				gchar * dirname = NULL;
				if (xournalFilename.startsWith("file://")) {
					dirname = g_path_get_dirname(xournalFilename.substring(7).c_str());
				} else {
					dirname = g_path_get_dirname(xournalFilename.c_str());
				}
				gchar * file = g_path_get_basename(sFilename);

				gchar * tmpFilename = g_build_path(G_DIR_SEPARATOR_S, dirname, file, NULL);

				if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS)) {
					filename = tmpFilename;
				}

				g_free(tmpFilename);
				g_free(dirname);
				g_free(file);
			}
		} else if (!strcmp("attach", domain)) {
			gchar * tmpFilename = g_strdup_printf("%s.bg.pdf", xournalFilename.c_str());

			if (g_file_test(tmpFilename, G_FILE_TEST_EXISTS)) {
				filename = tmpFilename;
			}

			g_free(tmpFilename);
		} else {
			error("Unknown domain type: %s", domain);
			return;
		}

		if (g_file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
			gchar * uri = g_strdup_printf("file://%s", filename.c_str());
			doc.readPdf(uri, false);
			g_free(uri);
		} else {
			if (!strcmp("attach", domain)) {
				attachPdfFileNotFound = "Attached PDF";
			} else {
				attachPdfFileNotFound = filename;
			}
		}
	}

	page->setBackgroundPdfPageNr(pageno - 1);
}

void LoadHandler::parsePage() {

	if (!strcmp(elementName, "background")) {
		const char * type = getAttrib("type");

		if (strcmp("solid", type) == 0) {
			parseBgSolid();
		} else if (strcmp("pixmap", type) == 0) {
			parseBgPixmap();
		} else if (strcmp("pdf", type) == 0) {
			parseBgPdf();
		} else {
			error("Unknown background type: %s", type);
		}
	} else if (!strcmp(elementName, "layer")) {
		this->pos = PARSER_POS_IN_LAYER;
		this->layer = new Layer();
		this->page->addLayer(this->layer);
	}

	// TODO: check special chars
	// TODO: add recent items to gtk list

	//			} else if (!strcmp(*attribute_names, "filename")) {
	//				if (tmpPage->bg->type <= BG_SOLID || (has_attr != 9)) {
	//					*error = xoj_invalid();
	//					return;
	//				}
	//				if (tmpPage->bg->file_domain == DOMAIN_CLONE) {
	//					// filename is a page number
	//					i = strtol(*attribute_values, &ptr, 10);
	//					if (ptr == *attribute_values || i < 0 || i > tmpJournal->getPageCount() - 2) {
	//						*error = xoj_invalid();
	//						return;
	//					}
	//					tmpbg = (control->getJournal()->getPage(i))->bg;
	//					if (tmpbg->type != tmpPage->bg->type) {
	//						*error = xoj_invalid();
	//						return;
	//					}
	//					tmpPage->bg->filename = tmpbg->filename;
	//					tmpPage->bg->pixbuf = tmpbg->pixbuf;
	//					if (tmpbg->pixbuf != NULL)
	//						gdk_pixbuf_ref(tmpbg->pixbuf);
	//					tmpPage->bg->file_domain = tmpbg->file_domain;
	//				} else {
	//					tmpPage->bg->filename = *attribute_values;
	//					if (tmpPage->bg->type == BG_PIXMAP) {
	//						if (tmpPage->bg->file_domain == DOMAIN_ATTACH) {
	//							tmpbg_filename = g_strdup_printf("%s.%s", tmpFilename, *attribute_values);
	//							if (sscanf(*attribute_values, "bg_%d.png", &i) == 1)
	//								control->getJournal()->setMinAttachmentNo(i);
	//						} else
	//							tmpbg_filename = g_strdup(*attribute_values);
	//						tmpPage->bg->pixbuf = gdk_pixbuf_new_from_file(tmpbg_filename, NULL);
	//						if (tmpPage->bg->pixbuf == NULL) {
	//							dialog = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
	//									GTK_BUTTONS_OK, _("Could not open background '%s'. Setting background to white."),
	//									tmpbg_filename);
	//							gtk_dialog_run(GTK_DIALOG(dialog));
	//							gtk_widget_destroy(dialog);
	//							tmpPage->bg->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
	//							gdk_pixbuf_fill(tmpPage->bg->pixbuf, 0xffffffff); // solid white
	//						}
	//						g_free(tmpbg_filename);
	//					}
	//				}
	//				has_attr |= 16;
}

bool LoadHandler::parseColor(const char * text, int & color) {
	if (text == NULL) {
		error("Attribute color not set!");
		return false;
	}

	if (text[0] == '#') {
		gchar * ptr = NULL;
		int c = g_ascii_strtoull(&text[1], &ptr, 16);
		if (ptr != text + strlen(text)) {
			error("Unknown color value \"%s\"", text);
			return false;
		}

		color = c >> 8;

		return true;
	} else {
		for (int i = 0; i < COLOR_MAX; i++) {
			if (!strcmp(text, color_names[i])) {
				color = predef_colors_rgba[i];
				return true;
			}
		}
		error("Color \"%s\" unknown (not defined in default color list)!", text);
		return false;
	}
}

void LoadHandler::parseStroke() {
	this->stroke = new Stroke();
	this->layer->addElement(this->stroke);

	const char * width = getAttrib("width");
	char * ptr = NULL;
	char * tmpptr = NULL;
	double val = 0;

	stroke->setWidth(g_ascii_strtod(width, &ptr));
	if (ptr == width) {
		error("Error reading width of a stroke: %s", width);
		return;
	}

	while (*ptr != 0) {
		val = g_ascii_strtod(ptr, &tmpptr);
		if (tmpptr == ptr) {
			break;
		}
		ptr = tmpptr;
		stroke->addWidthValue(val);
	}
	stroke->freeUnusedWidthItems();

	int color = 0;
	const char * sColor = getAttrib("color");
	if (!parseColor(sColor, color)) {
		return;
	}
	stroke->setColor(color);

	const char * tool = getAttrib("tool");

	if (strcmp("eraser", tool) == 0) {
		stroke->setToolType(STROKE_TOOL_ERASER);
	} else if (strcmp("pen", tool) == 0) {
		stroke->setToolType(STROKE_TOOL_PEN);
	} else if (strcmp("highlighter", tool) == 0) {
		stroke->setToolType(STROKE_TOOL_HIGHLIGHTER);
	} else {
		g_warning("Unknown stroke type: \"%s\", assume pen", tool);
	}

	// TODO: implementieren

	//		if (tmpItem->brush.tool_type == TOOL_HIGHLIGHTER) {
	//			if (tmpItem->brush.color_no >= 0)
	//				tmpItem->brush.color_rgba &= ui.hiliter_alpha_mask;
	//		}
}

void LoadHandler::parseText() {
	this->text = new Text();
	this->layer->addElement(this->text);

	const char * sFont = getAttrib("font");
	double fontSize = getAttribDouble("size");
	double x = getAttribDouble("x");
	double y = getAttribDouble("y");

	this->text->setX(x);
	this->text->setY(y);

	XFont & f = text->getFont();
	if (sFont) {
		PangoFontDescription * desc;
		desc = pango_font_description_from_string(sFont);

		f.setName(pango_font_description_get_family(desc));
		f.setItalic(pango_font_description_get_style(desc) != PANGO_STYLE_NORMAL);
		f.setBold(pango_font_description_get_weight(desc) == PANGO_WEIGHT_BOLD);

		pango_font_description_free(desc);
	}

	f.setSize(fontSize);
	const char * sColor = getAttrib("color");
	int color = 0;
	parseColor(sColor, color);
	text->setColor(color);
}

void LoadHandler::parseImage() {
	//		tmpItem = (struct Item *) g_malloc0(sizeof(struct Item));
	//		tmpItem->type = ITEM_IMAGE;
	//		tmpItem->canvas_item = NULL;
	//		tmpItem->image = NULL;
	//		tmpItem->image_scaled = NULL;
	//		tmpItem->image_png = NULL;
	//		tmpItem->image_png_len = 0;
	//		tmpLayer->items = g_list_append(tmpLayer->items, tmpItem);
	//		tmpLayer->nitems++;
	//		// scan for x, y
	//		has_attr = 0;
	//		while (*attribute_names != NULL) {
	//			if (!strcmp(*attribute_names, "left")) {
	//				if (has_attr & 1)
	//					*error = xoj_invalid();
	//				cleanup_numeric((gchar *) *attribute_values);
	//				tmpItem->bbox.left = g_ascii_strtod(*attribute_values, &ptr);
	//				if (ptr == *attribute_values)
	//					*error = xoj_invalid();
	//				has_attr |= 1;
	//			} else if (!strcmp(*attribute_names, "top")) {
	//				if (has_attr & 2)
	//					*error = xoj_invalid();
	//				cleanup_numeric((gchar *) *attribute_values);
	//				tmpItem->bbox.top = g_ascii_strtod(*attribute_values, &ptr);
	//				if (ptr == *attribute_values)
	//					*error = xoj_invalid();
	//				has_attr |= 2;
	//			} else if (!strcmp(*attribute_names, "right")) {
	//				if (has_attr & 4)
	//					*error = xoj_invalid();
	//				cleanup_numeric((gchar *) *attribute_values);
	//				tmpItem->bbox.right = g_ascii_strtod(*attribute_values, &ptr);
	//				if (ptr == *attribute_values)
	//					*error = xoj_invalid();
	//				has_attr |= 4;
	//			} else if (!strcmp(*attribute_names, "bottom")) {
	//				if (has_attr & 8)
	//					*error = xoj_invalid();
	//				cleanup_numeric((gchar *) *attribute_values);
	//				tmpItem->bbox.bottom = g_ascii_strtod(*attribute_values, &ptr);
	//				if (ptr == *attribute_values)
	//					*error = xoj_invalid();
	//				has_attr |= 8;
	//			} else
	//				*error = xoj_invalid();
	//			attribute_names++;
	//			attribute_values++;
	//		}
	//		if (has_attr != 15)
	//			*error = xoj_invalid();
	//	}
}

void LoadHandler::parseLayer() {
	if (!strcmp(elementName, "stroke")) { // start of a stroke
		pos = PARSER_POS_IN_STROKE;
		parseStroke();
	} else if (!strcmp(elementName, "text")) { // start of a text item
		pos = PARSER_POS_IN_TEXT;
		parseText();
	} else if (!strcmp(elementName, "image")) { // start of a image item
		pos = PARSER_POS_IN_IMAGE;
		parseImage();
	}
}

void LoadHandler::parserStartElement(GMarkupParseContext *context, const gchar *elementName,
		const gchar **attributeNames, const gchar **attributeValues, gpointer userdata, GError **error) {
	LoadHandler * handler = (LoadHandler *) userdata;
	// Return on error
	if (*error) {
		return;
	}

	handler->attributeNames = attributeNames;
	handler->attributeValues = attributeValues;
	handler->elementName = elementName;

	if (handler->pos == PARSER_POS_NOT_STARTED) {
		handler->parseStart();
	} else if (handler->pos == PARSER_POS_STARTED) {
		handler->parseContents();
	} else if (handler->pos == PARSER_POS_IN_PAGE) {
		handler->parsePage();
	} else if (handler->pos == PARSER_POS_IN_LAYER) {
		handler->parseLayer();
	}

	handler->attributeNames = NULL;
	handler->attributeValues = NULL;
	handler->elementName = NULL;
}

void LoadHandler::parserEndElement(GMarkupParseContext *context, const gchar *element_name, gpointer userdata,
		GError **error) {
	// Return on error
	if (*error) {
		return;
	}

	LoadHandler * handler = (LoadHandler *) userdata;

	if (handler->pos == PARSER_POS_STARTED && strcmp(element_name, "xournal") == 0) {
		handler->pos = PASER_POS_FINISHED;
	} else if (handler->pos == PARSER_POS_IN_PAGE && strcmp(element_name, "page") == 0) {
		handler->pos = PARSER_POS_STARTED;
		handler->page->unreference();
		handler->page = NULL;
		//		if (tmpPage->getLayerCount() == 0 || tmpPage->bg->type < 0)
		//			*error = xoj_invalid();
	} else if (handler->pos == PARSER_POS_IN_LAYER && strcmp(element_name, "layer") == 0) {
		handler->pos = PARSER_POS_IN_PAGE;
		handler->layer = NULL;
	} else if (handler->pos == PARSER_POS_IN_STROKE && strcmp(element_name, "stroke") == 0) {
		handler->pos = PARSER_POS_IN_LAYER;
		handler->stroke = NULL;
	} else if (handler->pos == PARSER_POS_IN_TEXT && strcmp(element_name, "text") == 0) {
		handler->pos = PARSER_POS_IN_LAYER;
		handler->text = NULL;
	} else if (handler->pos == PARSER_POS_IN_IMAGE && strcmp(element_name, "image") == 0) {
		handler->pos = PARSER_POS_IN_LAYER;
		handler->image = NULL;
	}
}

void LoadHandler::parserText(GMarkupParseContext *context, const gchar *text, gsize textLen, gpointer userdata,
		GError **error) {
	// Return on error
	if (*error) {
		return;
	}

	LoadHandler * handler = (LoadHandler *) userdata;

	if (handler->pos == PARSER_POS_IN_STROKE) {
		const char * ptr = text;
		int n = 0;

		bool xRead = false;
		double x = 0;

		while (textLen > 0) {
			double tmp = g_ascii_strtod(text, (char **) (&ptr));
			if (ptr == text) {
				break;
			}
			textLen -= (ptr - text);
			text = ptr;
			n++;

			if (!xRead) {
				xRead = true;
				x = tmp;
			} else {
				xRead = false;
				handler->stroke->addPoint(Point(x, tmp));
			}
		}
		handler->stroke->freeUnusedPointItems();

		if (n < 4 || n & 1) {
			error2(*error, "Wrong count of points (%i)", n);
			return;
		}
		if (handler->stroke->getWidthCount() != 0 && n / 2 != handler->stroke->getWidthCount() + 1) {
			error2(*error, "Wrong count of points, get %i, excpected %i", n, handler->stroke->getWidthCount());
			return;
		}
	} else if (handler->pos == PARSER_POS_IN_TEXT) {
		gchar * txt = g_strndup(text, textLen);
		handler->text->setText(txt);
		g_free(txt);
	} else if (handler->pos == PARSER_POS_IN_IMAGE) {
		//		tmpItem->image = read_pixbuf(text, text_len);
	}
}

bool LoadHandler::parseXml() {
	const GMarkupParser parser = { LoadHandler::parserStartElement, LoadHandler::parserEndElement,
			LoadHandler::parserText, NULL, NULL };
	GMarkupParseContext *context;
	this->error = NULL;
	gboolean valid = true;

	this->pos = PARSER_POS_NOT_STARTED;
	this->isXournalExtended = false;

	char buffer[1024];
	int len;

	context = g_markup_parse_context_new(&parser, (GMarkupParseFlags) 0, this, NULL);

	do {
		len = readFile(buffer, sizeof(buffer));
		if (len > 0) {
			valid = g_markup_parse_context_parse(context, buffer, len, &error);
		}

		if (error) {
			g_warning("error: %s\n", error->message);
		}
	} while (len >= 0 && valid && !error);

	if (valid) {
		valid = g_markup_parse_context_end_parse(context, &error);
	} else {
		if (error != NULL && error->message != NULL) {
			char * err = g_strdup_printf(_("XML Parser error: %s"), error->message);
			this->lastError = err;
			g_free(err);

			g_error_free(error);
		} else {
			this->lastError = _("Uknonwn parser error");
		}
		printf("error: %s\n", this->lastError.c_str());
	}

	g_markup_parse_context_free(context);

	if (this->pos != PASER_POS_FINISHED && this->lastError == NULL) {
		lastError = _("Document is not complete (may the end is cut off?)");
		return false;
	}

	printf("parsed version: %s::extended? %i\n", this->creatorVersion.c_str(), this->isXournalExtended);
	doc.setCreateBackupOnSave(!this->isXournalExtended);

	return valid;
}

bool LoadHandler::loadDocument(String filename, Document * doc) {
	if (filename.startsWith("file://")) {
		if (!openFile(filename.substring(7))) {
			return false;
		}
	} else {
		lastError = "Can only open file://";
		return false;
	}

	xournalFilename = filename;

	if (!parseXml()) {
		closeFile();
		return false;
	}
	doc->setFilename(filename.c_str());

	closeFile();

	*doc = this->doc;
	return true;
}

