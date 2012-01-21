#include <stdlib.h>
#include <string.h>
#include "Settings.h"
#include "ButtonConfig.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#define DEFAULT_FONT "Sans"
#define DEFAULT_FONT_SIZE 12

#define WRITE_BOOL_PROP(var) xmlNode = saveProperty((const char *)#var, var ? "true" : "false", root)
#define WRITE_STRING_PROP(var) xmlNode = saveProperty((const char *)#var, var.isEmpty() ? "" : var.c_str(), root)
#define WRITE_INT_PROP(var) xmlNode = saveProperty((const char *)#var, var, root)
#define WRITE_DOUBLE_PROP(var) xmlNode = savePropertyDouble((const char *)#var, var, root)
#define WRITE_COMMENT(var) com = xmlNewComment((const xmlChar *)var); \
	xmlAddPrevSibling(xmlNode, com);

const char * BUTTON_NAMES[] = { "middle", "right", "eraser", "touch", "default" };
const int BUTTON_COUNT = 5;

Settings::Settings(String filename) {
	XOJ_INIT_TYPE(Settings);

	this->filename = filename;
	this->timeoutId = 0;
	this->saved = true;

	loadDefault();

	checkCanXInput();
}

Settings::~Settings() {
	XOJ_CHECK_TYPE(Settings);

	if (this->timeoutId) {
		save();
	}

	for (int i = 0; i < 5; i++) {
		delete this->buttonConfig[i];
	}

	XOJ_RELEASE_TYPE(Settings);
}

void Settings::loadDefault() {
	XOJ_CHECK_TYPE(Settings);

	this->useXinput = true;
	this->presureSensitivity = true;
	this->ignoreCoreEvents = false;
	this->saved = true;
	this->canXIput = false;

	this->maximized = false;
	this->showTwoPages = false;

	this->displayDpi = 72;

	this->font.setName(DEFAULT_FONT);
	this->font.setSize(DEFAULT_FONT_SIZE);

	this->mainWndWidth = 800;
	this->mainWndHeight = 600;

	this->showSidebar = true;
	this->sidebarWidth = 150;

	this->sidebarOnRight = false;

	this->scrollbarOnLeft = false;

	this->widthMinimumMultiplier = 0.0;
	this->widthMaximumMultiplier = 1.25;

	this->autoloadPdfXoj = true;
	this->showBigCursor = false;
	this->scrollbarHideType = SCROLLBAR_HIDE_NONE;

	this->autosaveTimeout = 1;
	this->autosaveEnabled = true;

	this->allowScrollOutsideThePage = false;

	this->enableLeafEnterWorkaround = true;

	this->defaultSaveName = _("%F-Note-%H-%M.xoj");

	this->visiblePageFormats = GTK_PAPER_NAME_A4 "," GTK_PAPER_NAME_A5 "," GTK_PAPER_NAME_LETTER ","GTK_PAPER_NAME_LEGAL;

	// Eraser
	this->buttonConfig[0] = new ButtonConfig(TOOL_ERASER, 0, TOOL_SIZE_NONE, DRAWING_TYPE_NONE, ERASER_TYPE_NONE);
	// Middle button
	this->buttonConfig[1] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_NONE, ERASER_TYPE_NONE);
	// Right button
	this->buttonConfig[2] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_NONE, ERASER_TYPE_NONE);
	// Touch
	this->buttonConfig[3] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_NONE, ERASER_TYPE_NONE);
	// Default config
	this->buttonConfig[4] = new ButtonConfig(TOOL_PEN, 0, TOOL_SIZE_FINE, DRAWING_TYPE_NONE, ERASER_TYPE_NONE);

	this->fullscreenHideElements = "mainMenubar";
	this->presentationHideElements = "mainMenubar,sidebarContents";

	this->pageInsertType = PAGE_INSERT_TYPE_COPY;
	this->pageBackgroundColor = 0xffffff; //white

	this->pdfPageCacheSize = 10;

	this->selectionColor = 0xff0000;
}

void Settings::parseData(xmlNodePtr cur, SElement & elem) {
	XOJ_CHECK_TYPE(Settings);

	for (xmlNodePtr x = cur->children; x != NULL; x = x->next) {
		if (!xmlStrcmp(x->name, (const xmlChar *) "data")) {
			xmlChar * name = xmlGetProp(x, (const xmlChar *) "name");
			parseData(x, elem.child((const char *) name));
			xmlFree(name);
		} else if (!xmlStrcmp(x->name, (const xmlChar *) "attribute")) {
			xmlChar *name = xmlGetProp(x, (const xmlChar *) "name");
			xmlChar *value = xmlGetProp(x, (const xmlChar *) "value");
			xmlChar *type = xmlGetProp(x, (const xmlChar *) "type");

			String sType = (const char *) type;

			if (sType.equals("int")) {
				int i = atoi((const char *) value);
				elem.setInt((const char *) name, i);
			} else if (sType.equals("double")) {
				double d = atof((const char *) value);
				elem.setDouble((const char *) name, d);
			} else if (sType.equals("hex")) {
				int i = 0;
				if (sscanf((const char *) value, "%x", &i)) {
					elem.setIntHex((const char *) name, i);
				} else {
					g_warning("Settings::Unknown hex value: %s:%s\n", name, value);
				}
			} else if (sType.equals((const char *) "string")) {
				elem.setString((const char *) name, (const char *) value);
			} else if (sType.equals("boolean")) {
				elem.setBool((const char *) name, strcmp((const char *) value, "true") == 0);
			} else {
				g_warning("Settings::Unknown datatype: %s\n", sType.c_str());
			}

			xmlFree(name);
			xmlFree(type);
			xmlFree(value);
		} else {
			g_warning("Settings::parseData: Unknown XML node: %s\n", x->name);
			continue;
		}
	}

}

void Settings::parseItem(xmlDocPtr doc, xmlNodePtr cur) {
	XOJ_CHECK_TYPE(Settings);

	// Parse data map
	if (!xmlStrcmp(cur->name, (const xmlChar *) "data")) {
		xmlChar * name = xmlGetProp(cur, (const xmlChar *) "name");
		if (name == NULL) {
			g_warning("Settings::%s:No name property!\n", cur->name);
			return;
		}

		parseData(cur, data[(const char *) name]);

		xmlFree(name);
		return;
	}

	if (cur->type == XML_COMMENT_NODE) {
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "property")) {
		g_warning("Settings::Unknown XML node: %s\n", cur->name);
		return;
	}

	xmlChar * name = xmlGetProp(cur, (const xmlChar *) "name");
	if (name == NULL) {
		g_warning("Settings::%s:No name property!\n", cur->name);
		return;
	}

	if (xmlStrcmp(name, (const xmlChar *) "font") == 0) {
		xmlFree(name);
		xmlChar * font;
		xmlChar *size;

		font = xmlGetProp(cur, (const xmlChar *) "font");
		if (font) {
			this->font.setName((const char *) font);
			xmlFree(font);
		}

		size = xmlGetProp(cur, (const xmlChar *) "size");
		if (size) {
			double dSize = DEFAULT_FONT_SIZE;
			if (sscanf((const char *) size, "%lf", &dSize) == 1) {
				this->font.setSize(dSize);
			}

			xmlFree(size);
		}

		return;
	}

	xmlChar * value = xmlGetProp(cur, (const xmlChar *) "value");
	if (value == NULL) {
		xmlFree(name);
		g_warning("No value property!\n");
		return;
	}

	if (xmlStrcmp(name, (const xmlChar *) "presureSensitivity") == 0) {
		setPresureSensitivity(xmlStrcmp(value, (const xmlChar *) "true") ? false : true);
	} else if (xmlStrcmp(name, (const xmlChar *) "useXinput") == 0) {
		// Value is update from main after the window is created
		this->useXinput = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "ignoreCoreEvents") == 0) {
		this->ignoreCoreEvents = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "selectedToolbar") == 0) {
		this->selectedToolbar = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "lastSavePath") == 0) {
		this->lastSavePath = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "lastImagePath") == 0) {
		this->lastImagePath = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "displayDpi") == 0) {
		this->displayDpi = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "mainWndWidth") == 0) {
		this->mainWndWidth = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "mainWndHeight") == 0) {
		this->mainWndHeight = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "maximized") == 0) {
		this->maximized = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "showSidebar") == 0) {
		this->showSidebar = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "sidebarWidth") == 0) {
		this->sidebarWidth = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "widthMinimumMultiplier") == 0) {
		this->widthMinimumMultiplier = g_ascii_strtod((const char *) value, NULL);
	} else if (xmlStrcmp(name, (const xmlChar *) "widthMaximumMultiplier") == 0) {
		this->widthMaximumMultiplier = g_ascii_strtod((const char *) value, NULL);
	} else if (xmlStrcmp(name, (const xmlChar *) "sidebarOnRight") == 0) {
		this->sidebarOnRight = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "scrollbarOnLeft") == 0) {
		this->scrollbarOnLeft = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "showTwoPages") == 0) {
		this->showTwoPages = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "autoloadPdfXoj") == 0) {
		this->autoloadPdfXoj = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "showBigCursor") == 0) {
		this->showBigCursor = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "defaultSaveName") == 0) {
		this->defaultSaveName = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "visiblePageFormats") == 0) {
		this->visiblePageFormats = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "autosaveEnabled") == 0) {
		this->autosaveEnabled = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "autosaveTimeout") == 0) {
		this->autosaveTimeout = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "fullscreenHideElements") == 0) {
		this->fullscreenHideElements = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "presentationHideElements") == 0) {
		this->presentationHideElements = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "pageInsertType") == 0) {
		this->pageInsertType = pageInsertTypeFromString((const char *) value);
	} else if (xmlStrcmp(name, (const xmlChar *) "pageBackgroundColor") == 0) {
		this->pageBackgroundColor = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "pdfPageCacheSize") == 0) {
		this->pdfPageCacheSize = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "selectionColor") == 0) {
		this->selectionColor = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "allowScrollOutsideThePage") == 0) {
		this->allowScrollOutsideThePage = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "enableLeafEnterWorkaround") == 0) {
		this->enableLeafEnterWorkaround = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "scrollbarHideType") == 0) {
		if (xmlStrcmp(value, (const xmlChar *) "both") == 0) {
			this->scrollbarHideType = SCROLLBAR_HIDE_BOTH;
		} else if (xmlStrcmp(value, (const xmlChar *) "horizontal") == 0) {
			this->scrollbarHideType = SCROLLBAR_HIDE_HORIZONTAL;
		} else if (xmlStrcmp(value, (const xmlChar *) "vertical") == 0) {
			this->scrollbarHideType = SCROLLBAR_HIDE_VERTICAL;
		} else {
			this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
		}
	}

	xmlFree(name);
	xmlFree(value);
}

void Settings::loadButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	SElement & s = getCustomElement("buttonConfig");

	for (int i = 0; i < BUTTON_COUNT; i++) {
		SElement & e = s.child(BUTTON_NAMES[i]);
		ButtonConfig * cfg = buttonConfig[i];

		String sType;
		if (e.getString("tool", sType)) {
			ToolType type = toolTypeFromString(sType);
			cfg->action = type;

			if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
				bool change = false;
				bool ruler = false;
				bool shapeRecognizer = false;
				e.getBool("changeRulerShapeRecognizer", change);
				e.getBool("ruler", ruler);
				e.getBool("shapeRecognizer", shapeRecognizer);

				if (!change) {
					cfg->drawingType = DRAWING_TYPE_DONT_CHANGE;
				} else {
					if (ruler) {
						cfg->drawingType = DRAWING_TYPE_RULER;
					} else if (shapeRecognizer) {
						cfg->drawingType = DRAWING_TYPE_STROKE_RECOGNIZER;
					} else {
						cfg->drawingType = DRAWING_TYPE_NONE;
					}
				}

				String sSize;
				if (e.getString("size", sSize)) {
					cfg->size = toolSizeFromString(sSize);
				} else {
					// If not specified: do not change
					cfg->size = TOOL_SIZE_NONE;
				}
			}

			if (type == TOOL_PEN || type == TOOL_HILIGHTER || type == TOOL_TEXT) {
				e.getInt("color", cfg->color);
			}

			if (type == TOOL_ERASER) {
				String sEraserMode;
				if (e.getString("eraserMode", sEraserMode)) {
					cfg->eraserMode = eraserTypeFromString(sEraserMode);
				} else {
					// If not specified: do not change
					cfg->eraserMode = ERASER_TYPE_NONE;
				}
			}

			// Touch device
			if (i == 3) {
				if (!e.getString("device", cfg->device)) {
					cfg->device = "";
				}

				e.getBool("disableDrawing", cfg->disableDrawing);
			}

		} else {
			continue;
		}
	}
}

bool Settings::load() {
	XOJ_CHECK_TYPE(Settings);

	xmlKeepBlanksDefault(0);

	if (!g_file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
		g_warning("configfile does not exist %s\n", filename.c_str());
		return false;
	}

	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if (doc == NULL) {
		g_warning("Settings::load:: doc == null, could not load Settings!\n");
		return false;
	}

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		g_message("The settings file \"%s\" is empty", filename.c_str());
		xmlFreeDoc(doc);

		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "settings")) {
		g_message("File \"%s\" is of the wrong type", filename.c_str());
		xmlFreeDoc(doc);

		return false;
	}

	cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		parseItem(doc, cur);

		cur = cur->next;
	}

	xmlFreeDoc(doc);

	loadButtonConfig();

	return true;
}

gboolean Settings::saveCallback(Settings * data) {
	XOJ_CHECK_TYPE_OBJ(data, Settings);

	((Settings*) data)->save();
	return false;
}

void Settings::saveTimeout() {
	XOJ_CHECK_TYPE(Settings);

	if (this->timeoutId) {
		return;
	}

	this->timeoutId = g_timeout_add_seconds(2, (GSourceFunc) &saveCallback, this);
}

xmlNodePtr Settings::savePropertyDouble(const gchar * key, double value, xmlNodePtr parent) {
	XOJ_CHECK_TYPE(Settings);

	char * text = g_strdup_printf("%0.3lf", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar * key, int value, xmlNodePtr parent) {
	XOJ_CHECK_TYPE(Settings);

	char * text = g_strdup_printf("%i", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar * key, const gchar * value, xmlNodePtr parent) {
	XOJ_CHECK_TYPE(Settings);

	xmlNodePtr xmlNode = xmlNewChild(parent, NULL, (const xmlChar *) "property", NULL);

	xmlSetProp(xmlNode, (const xmlChar *) "name", (const xmlChar *) key);

	xmlSetProp(xmlNode, (const xmlChar *) "value", (const xmlChar *) value);

	return xmlNode;
}

void Settings::saveButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	SElement & s = getCustomElement("buttonConfig");
	s.clear();

	for (int i = 0; i < BUTTON_COUNT; i++) {
		SElement & e = s.child(BUTTON_NAMES[i]);
		ButtonConfig * cfg = buttonConfig[i];

		ToolType type = cfg->action;
		e.setString("tool", toolTypeToString(type));

		if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
			bool change = false;
			bool ruler = false;
			bool shapeRecognizer = false;
			if (cfg->drawingType == DRAWING_TYPE_DONT_CHANGE) {
				change = false;
				ruler = false;
				shapeRecognizer = false;
			} else if (cfg->drawingType == DRAWING_TYPE_STROKE_RECOGNIZER) {
				change = true;
				ruler = false;
				shapeRecognizer = true;
			} else if (cfg->drawingType == DRAWING_TYPE_RULER) {
				change = true;
				ruler = true;
				shapeRecognizer = false;
			} else if (cfg->drawingType == DRAWING_TYPE_NONE) {
				change = true;
				ruler = false;
				shapeRecognizer = false;
			}

			e.setBool("changeRulerShapeRecognizer", change);
			e.setBool("ruler", ruler);
			e.setBool("shapeRecognizer", shapeRecognizer);
			e.setString("size", toolSizeToString(cfg->size));
		} // end if pen or hilighter

		if (type == TOOL_PEN || type == TOOL_HILIGHTER || type == TOOL_TEXT) {
			e.setIntHex("color", cfg->color);
		}

		if (type == TOOL_ERASER) {
			e.setString("eraserMode", eraserTypeToString(cfg->eraserMode));
		}

		// Touch device
		if (i == 3) {
			e.setString("device", cfg->device);
			e.setBool("disableDrawing", cfg->disableDrawing);
		}
	}
}

void Settings::save() {
	XOJ_CHECK_TYPE(Settings);

	if (this->timeoutId) {
		g_source_remove(this->timeoutId);
		this->timeoutId = 0;
	}

	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr xmlNode;

	xmlIndentTreeOutput = TRUE;

	doc = xmlNewDoc((const xmlChar *) "1.0");
	if (doc == NULL) {
		return;
	}

	saveButtonConfig();

	/* Create metadata root */
	root = xmlNewDocNode(doc, NULL, (const xmlChar *) "settings", NULL);
	xmlDocSetRootElement(doc, root);
	xmlNodePtr com = xmlNewComment((const xmlChar *) "The Xournal++ settings file. Do not edit this file! "
		"The most settings are available in the Settings dialog, "
		"the others are commented in this file, but handle with care!");
	xmlAddPrevSibling(root, com);

	WRITE_BOOL_PROP(useXinput);
	WRITE_BOOL_PROP(presureSensitivity);
	WRITE_BOOL_PROP(ignoreCoreEvents);

	WRITE_STRING_PROP(selectedToolbar);
	WRITE_STRING_PROP(lastSavePath);
	WRITE_STRING_PROP(lastImagePath);

	WRITE_INT_PROP(displayDpi);
	WRITE_INT_PROP(mainWndWidth);
	WRITE_INT_PROP(mainWndHeight);
	WRITE_BOOL_PROP(maximized);

	WRITE_BOOL_PROP(showSidebar);
	WRITE_INT_PROP(sidebarWidth);

	WRITE_BOOL_PROP(sidebarOnRight);
	WRITE_BOOL_PROP(scrollbarOnLeft);
	WRITE_BOOL_PROP(showTwoPages);

	WRITE_STRING_PROP(fullscreenHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)");

	WRITE_STRING_PROP(presentationHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Presentation mode, separated by a colon (,)");

	WRITE_BOOL_PROP(showBigCursor);

	if (this->scrollbarHideType == SCROLLBAR_HIDE_BOTH) {
		saveProperty((const char *) "scrollbarHideType", "both", root);
	} else if (this->scrollbarHideType == SCROLLBAR_HIDE_HORIZONTAL) {
		saveProperty((const char *) "scrollbarHideType", "horizontal", root);
	} else if (this->scrollbarHideType == SCROLLBAR_HIDE_VERTICAL) {
		saveProperty((const char *) "scrollbarHideType", "vertical", root);
	} else {
		saveProperty((const char *) "scrollbarHideType", "none", root);
	}


	WRITE_BOOL_PROP(autoloadPdfXoj);
	WRITE_COMMENT("Hides scroolbars in the main window, allowed values: \"none\", \"horizontal\", \"vertical\", \"both\"");

	WRITE_STRING_PROP(defaultSaveName);

	WRITE_STRING_PROP(visiblePageFormats);
	WRITE_COMMENT("This paper format is visible in the paper format dialog, separated by a colon");

	WRITE_BOOL_PROP(autosaveEnabled);
	WRITE_INT_PROP(autosaveTimeout);

	WRITE_BOOL_PROP(allowScrollOutsideThePage);

	WRITE_BOOL_PROP(enableLeafEnterWorkaround);
	WRITE_COMMENT("If Xournal crashes if you e.g. unplug your mouse set this to true. If you have input problems, you can turn it of with false.");

	String pageInsertType = pageInsertTypeToString(this->pageInsertType);
	WRITE_STRING_PROP(pageInsertType);

	WRITE_INT_PROP(pageBackgroundColor);
	WRITE_INT_PROP(selectionColor);

	WRITE_INT_PROP(pdfPageCacheSize);
	WRITE_COMMENT("The count of rendered PDF pages which will be cached.");

	WRITE_DOUBLE_PROP(widthMinimumMultiplier);
	WRITE_COMMENT("The multiplier for the pressure sensitivity of the pen");
	WRITE_DOUBLE_PROP(widthMaximumMultiplier);
	WRITE_COMMENT("The multiplier for the pressure sensitivity of the pen");

	xmlNodePtr xmlFont;
	xmlFont = xmlNewChild(root, NULL, (const xmlChar *) "property", NULL);
	xmlSetProp(xmlFont, (const xmlChar *) "name", (const xmlChar *) "font");
	xmlSetProp(xmlFont, (const xmlChar *) "font", (const xmlChar *) this->font.getName().c_str());
	gchar * sSize = g_strdup_printf("%0.1lf", this->font.getSize());
	xmlSetProp(xmlFont, (const xmlChar *) "size", (const xmlChar *) sSize);
	g_free(sSize);

	std::map<String, SElement>::iterator it;
	for (it = data.begin(); it != data.end(); it++) {
		saveData(root, (*it).first, (*it).second);
	}

	xmlSaveFormatFile(filename.c_str(), doc, 1);
	xmlFreeDoc(doc);
}

void Settings::saveData(xmlNodePtr root, String name, SElement & elem) {
	XOJ_CHECK_TYPE(Settings);

	xmlNodePtr xmlNode = xmlNewChild(root, NULL, (const xmlChar *) "data", NULL);

	xmlSetProp(xmlNode, (const xmlChar *) "name", (const xmlChar *) name.c_str());

	std::map<String, SAttribute>::iterator it;
	for (it = elem.attributes().begin(); it != elem.attributes().end(); it++) {
		String aname = (*it).first;
		SAttribute & attrib = (*it).second;

		XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

		String type;
		String value;

		if (attrib.type == ATTRIBUTE_TYPE_BOOLEAN) {
			type = "boolean";

			if (attrib.iValue) {
				value = "true";
			} else {
				value = "false";
			}
		} else if (attrib.type == ATTRIBUTE_TYPE_INT) {
			type = "int";

			char * tmp = g_strdup_printf("%i", attrib.iValue);
			value = tmp;
			g_free(tmp);
		} else if (attrib.type == ATTRIBUTE_TYPE_DOUBLE) {
			type = "double";

			char * tmp = g_strdup_printf("%lf", attrib.dValue);
			value = tmp;
			g_free(tmp);
		} else if (attrib.type == ATTRIBUTE_TYPE_INT_HEX) {
			type = "hex";

			char * tmp = g_strdup_printf("%06x", attrib.iValue);
			value = tmp;
			g_free(tmp);
		} else if (attrib.type == ATTRIBUTE_TYPE_STRING) {
			type = "string";
			value = attrib.sValue;
		} else {
			// Unknown type or empty attribute
			continue;
		}

		xmlNodePtr at;
		at = xmlNewChild(xmlNode, NULL, (const xmlChar *) "attribute", NULL);

		xmlSetProp(at, (const xmlChar *) "name", (const xmlChar *) aname.c_str());
		xmlSetProp(at, (const xmlChar *) "type", (const xmlChar *) type.c_str());
		xmlSetProp(at, (const xmlChar *) "value", (const xmlChar *) value.c_str());

		if (!attrib.comment.isEmpty()) {
			xmlNodePtr com = xmlNewComment((const xmlChar *) attrib.comment.c_str());
			xmlAddPrevSibling(xmlNode, com);
		}
	}

	std::map<String, SElement>::iterator i;
	for (i = elem.children().begin(); i != elem.children().end(); i++) {
		saveData(xmlNode, (*i).first, (*i).second);
	}
}

// Getter- / Setter
bool Settings::isPresureSensitivity() {
	XOJ_CHECK_TYPE(Settings);

	return this->presureSensitivity;
}

bool Settings::isXinputEnabled() {
	XOJ_CHECK_TYPE(Settings);

	return this->useXinput;
}

bool Settings::isIgnoreCoreEvents() {
	XOJ_CHECK_TYPE(Settings);

	return this->ignoreCoreEvents;
}

void Settings::setIgnoreCoreEvents(bool ignor) {
	XOJ_CHECK_TYPE(Settings);

	if(this->ignoreCoreEvents == ignor) {
		return;
	}

	this->ignoreCoreEvents = ignor;
	saveTimeout();
}

bool Settings::isSidebarOnRight() {
	XOJ_CHECK_TYPE(Settings);

	return this->sidebarOnRight;
}

void Settings::setSidebarOnRight(bool right) {
	XOJ_CHECK_TYPE(Settings);

	if (this->sidebarOnRight == right) {
		return;
	}

	this->sidebarOnRight = right;

	saveTimeout();
}

bool Settings::isScrollbarOnLeft() {
	XOJ_CHECK_TYPE(Settings);

	return this->scrollbarOnLeft;
}

void Settings::setScrollbarOnLeft(bool right) {
	XOJ_CHECK_TYPE(Settings);

	if (this->scrollbarOnLeft == right) {
		return;
	}

	this->scrollbarOnLeft = right;

	saveTimeout();
}

int Settings::getAutosaveTimeout() {
	return this->autosaveTimeout;
}

void Settings::setAutosaveTimeout(int autosave) {
	XOJ_CHECK_TYPE(Settings);

	if (this->autosaveTimeout == autosave) {
		return;
	}

	this->autosaveTimeout = autosave;

	saveTimeout();
}

bool Settings::isAutosaveEnabled() {
	XOJ_CHECK_TYPE(Settings);

	return this->autosaveEnabled;
}

void Settings::setAutosaveEnabled(bool autosave) {
	XOJ_CHECK_TYPE(Settings);

	if (this->autosaveEnabled == autosave) {
		return;
	}

	this->autosaveEnabled = autosave;

	saveTimeout();
}

bool Settings::isAllowScrollOutsideThePage() {
	XOJ_CHECK_TYPE(Settings);

	return this->allowScrollOutsideThePage;
}

void Settings::setAllowScrollOutsideThePage(bool outside) {
	XOJ_CHECK_TYPE(Settings);

	this->allowScrollOutsideThePage = outside;

	saveTimeout();
}

bool Settings::isEnableLeafEnterWorkaround() {
	XOJ_CHECK_TYPE(Settings);

	return this->enableLeafEnterWorkaround;
}

void Settings::setEnableLeafEnterWorkaround(bool enable) {
	XOJ_CHECK_TYPE(Settings);

	this->enableLeafEnterWorkaround = enable;

	saveTimeout();
}

bool Settings::isShowBigCursor() {
	XOJ_CHECK_TYPE(Settings);

	return this->showBigCursor;
}

void Settings::setShowBigCursor(bool b) {
	XOJ_CHECK_TYPE(Settings);

	if (this->showBigCursor == b) {
		return;
	}

	this->showBigCursor = b;
	saveTimeout();
}

ScrollbarHideType Settings::getScrollbarHideType() {
	XOJ_CHECK_TYPE(Settings);

	return this->scrollbarHideType;
}

void Settings::setScrollbarHideType(ScrollbarHideType type) {
	XOJ_CHECK_TYPE(Settings);

	if (this->scrollbarHideType == type) {
		return;
	}

	this->scrollbarHideType = type;

	saveTimeout();
}

bool Settings::isAutloadPdfXoj() {
	XOJ_CHECK_TYPE(Settings);

	return this->autoloadPdfXoj;
}

void Settings::setAutoloadPdfXoj(bool load) {
	XOJ_CHECK_TYPE(Settings);

	if (this->autoloadPdfXoj == load) {
		return;
	}
	this->autoloadPdfXoj = load;
	saveTimeout();
}

String Settings::getDefaultSaveName() {
	XOJ_CHECK_TYPE(Settings);

	return this->defaultSaveName;
}

void Settings::setDefaultSaveName(String name) {
	XOJ_CHECK_TYPE(Settings);

	if (this->defaultSaveName == name) {
		return;
	}

	this->defaultSaveName = name;

	saveTimeout();
}

String Settings::getVisiblePageFormats() {
	XOJ_CHECK_TYPE(Settings);

	return this->visiblePageFormats;
}

void Settings::setShowTwoPages(bool showTwoPages) {
	XOJ_CHECK_TYPE(Settings);

	if (this->showTwoPages == showTwoPages) {
		return;
	}

	this->showTwoPages = showTwoPages;
	saveTimeout();
}

bool Settings::isShowTwoPages() {
	XOJ_CHECK_TYPE(Settings);

	return this->showTwoPages;
}

/**
 * XInput is available
 */
bool Settings::isXInputAvailable() {
	XOJ_CHECK_TYPE(Settings);

	return this->canXIput;
}

/**
 * XInput should be used in the application
 */
bool Settings::isUseXInput() {
	XOJ_CHECK_TYPE(Settings);

	return this->useXinput && this->canXIput;
}

void Settings::setPresureSensitivity(gboolean presureSensitivity) {
	XOJ_CHECK_TYPE(Settings);

	if (this->presureSensitivity == presureSensitivity) {
		return;
	}
	this->presureSensitivity = presureSensitivity;

	saveTimeout();
}

void Settings::setLastSavePath(String path) {
	XOJ_CHECK_TYPE(Settings);

	this->lastSavePath = path;
	saveTimeout();
}

String Settings::getLastSavePath() {
	XOJ_CHECK_TYPE(Settings);

	return this->lastSavePath;
}

void Settings::setLastImagePath(String path) {
	XOJ_CHECK_TYPE(Settings);

	if (this->lastImagePath == path) {
		return;
	}
	this->lastImagePath = path;
	saveTimeout();
}

String Settings::getLastImagePath() {
	XOJ_CHECK_TYPE(Settings);

	return this->lastImagePath;
}

void Settings::setDisplayDpi(int dpi) {
	XOJ_CHECK_TYPE(Settings);

	this->displayDpi = dpi;
}

int Settings::getDisplayDpi() {
	return this->displayDpi;
}

bool Settings::isSidebarVisible() {
	XOJ_CHECK_TYPE(Settings);

	return this->showSidebar;
}

void Settings::setSidebarVisible(bool visible) {
	XOJ_CHECK_TYPE(Settings);

	if (this->showSidebar == visible) {
		return;
	}
	this->showSidebar = visible;
	saveTimeout();
}

int Settings::getSidebarWidth() {
	XOJ_CHECK_TYPE(Settings);

	return this->sidebarWidth;
}

void Settings::setSidebarWidth(int width) {
	XOJ_CHECK_TYPE(Settings);

	if (this->sidebarWidth == width) {
		return;
	}
	this->sidebarWidth = width;
	saveTimeout();
}

void Settings::checkCanXInput() {
	XOJ_CHECK_TYPE(Settings);

	this->canXIput = FALSE;
	GList * devList = gdk_devices_list();

	while (devList != NULL) {
		GdkDevice * device = (GdkDevice *) devList->data;
		if (device != gdk_device_get_core_pointer()) {

			// get around a GDK bug: map the valuator range CORRECTLY to [0,1]
#ifdef ENABLE_XINPUT_BUGFIX
			gdk_device_set_axis_use(device, 0, GDK_AXIS_IGNORE);
			gdk_device_set_axis_use(device, 1, GDK_AXIS_IGNORE);
#endif
			gdk_device_set_mode(device, GDK_MODE_SCREEN);
			if (g_str_has_suffix(device->name, "eraser")) {
				gdk_device_set_source(device, GDK_SOURCE_ERASER);
			}
			canXIput = TRUE;
		}
		devList = devList->next;
	}
}

void Settings::setMainWndSize(int width, int height) {
	XOJ_CHECK_TYPE(Settings);

	this->mainWndWidth = width;
	this->mainWndHeight = height;

	saveTimeout();
}

int Settings::getMainWndWidth() {
	XOJ_CHECK_TYPE(Settings);

	return this->mainWndWidth;
}

int Settings::getMainWndHeight() {
	XOJ_CHECK_TYPE(Settings);

	return this->mainWndHeight;
}

bool Settings::isMainWndMaximized() {
	XOJ_CHECK_TYPE(Settings);

	return this->maximized;
}

void Settings::setMainWndMaximized(bool max) {
	XOJ_CHECK_TYPE(Settings);

	this->maximized = max;
}

void Settings::setXinputEnabled(gboolean useXinput) {
	XOJ_CHECK_TYPE(Settings);

	if (this->useXinput == useXinput) {
		return;
	}

	this->useXinput = useXinput;

	saveTimeout();
}

double Settings::getWidthMinimumMultiplier() {
	XOJ_CHECK_TYPE(Settings);

	return this->widthMinimumMultiplier;
}

double Settings::getWidthMaximumMultiplier() {
	XOJ_CHECK_TYPE(Settings);

	return this->widthMaximumMultiplier;
}

void Settings::setSelectedToolbar(String name) {
	XOJ_CHECK_TYPE(Settings);

	if (this->selectedToolbar == name) {
		return;
	}
	this->selectedToolbar = name;
	saveTimeout();
}

String Settings::getSelectedToolbar() {
	XOJ_CHECK_TYPE(Settings);

	return this->selectedToolbar;
}

SElement & Settings::getCustomElement(String name) {
	XOJ_CHECK_TYPE(Settings);

	return this->data[name];
}

void Settings::customSettingsChanged() {
	XOJ_CHECK_TYPE(Settings);

	saveTimeout();
}

ButtonConfig * Settings::getButtonConfig(int id) {
	XOJ_CHECK_TYPE(Settings);

	if (id < 0 || id >= BUTTON_COUNT) {
		g_error("Settings::getButtonConfig try to get id=%i out of range!", id);
		return NULL;
	}
	return this->buttonConfig[id];
}

ButtonConfig * Settings::getEraserButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[0];
}

ButtonConfig * Settings::getMiddleButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[1];
}

ButtonConfig * Settings::getRightButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[2];
}

ButtonConfig * Settings::getTouchButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[3];
}

ButtonConfig * Settings::getDefaultButtonConfig() {
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[4];
}

String Settings::getFullscreenHideElements() {
	XOJ_CHECK_TYPE(Settings);

	return this->fullscreenHideElements;
}

void Settings::setFullscreenHideElements(String elements) {
	this->fullscreenHideElements = elements;
	saveTimeout();
}

String Settings::getPresentationHideElements() {
	XOJ_CHECK_TYPE(Settings);

	return this->presentationHideElements;
}

void Settings::setPresentationHideElements(String elements) {
	XOJ_CHECK_TYPE(Settings);

	this->presentationHideElements = elements;
	saveTimeout();
}

PageInsertType Settings::getPageInsertType() {
	XOJ_CHECK_TYPE(Settings);

	return this->pageInsertType;
}

void Settings::setPageInsertType(PageInsertType type) {
	XOJ_CHECK_TYPE(Settings);

	if (this->pageInsertType == type) {
		return;
	}
	this->pageInsertType = type;
	saveTimeout();
}

int Settings::getPageBackgroundColor() {
	XOJ_CHECK_TYPE(Settings);

	return this->pageBackgroundColor;
}

void Settings::setPageBackgroundColor(int color) {
	XOJ_CHECK_TYPE(Settings);

	if (this->pageBackgroundColor == color) {
		return;
	}
	this->pageBackgroundColor = color;
	saveTimeout();
}

int Settings::getSelectionColor() {
	XOJ_CHECK_TYPE(Settings);

	return this->selectionColor;
}

int Settings::getPdfPageCacheSize() {
	XOJ_CHECK_TYPE(Settings);

	return this->pdfPageCacheSize;
}

void Settings::setPdfPageCacheSize(int size) {
	XOJ_CHECK_TYPE(Settings);

	if (this->pdfPageCacheSize == size) {
		return;
	}
	this->pdfPageCacheSize = size;
	saveTimeout();
}

void Settings::setSelectionColor(int color) {
	XOJ_CHECK_TYPE(Settings);

	if (this->selectionColor == color) {
		return;
	}
	this->selectionColor = color;
	saveTimeout();
}

XojFont & Settings::getFont() {
	XOJ_CHECK_TYPE(Settings);

	return this->font;
}

void Settings::setFont(const XojFont & font) {
	XOJ_CHECK_TYPE(Settings);

	this->font = font;
}

//////////////////////////////////////////////////

SAttribute::SAttribute() {
	XOJ_INIT_TYPE(SAttribute);

	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;
}

SAttribute::SAttribute(const SAttribute & attrib) {
	XOJ_INIT_TYPE(SAttribute);

	*this = attrib;
}

SAttribute::~SAttribute() {
	XOJ_CHECK_TYPE(SAttribute);

	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;

	XOJ_RELEASE_TYPE(SAttribute);
}

//////////////////////////////////////////////////

__RefSElement::__RefSElement() {
	XOJ_INIT_TYPE(__RefSElement);

	this->refcount = 0;
}

__RefSElement::~__RefSElement() {
	XOJ_RELEASE_TYPE(__RefSElement);
}

void __RefSElement::ref() {
	XOJ_CHECK_TYPE(__RefSElement);

	this->refcount++;
}
void __RefSElement::unref() {
	XOJ_CHECK_TYPE(__RefSElement);

	this->refcount--;
	if (this->refcount == 0) {
		delete this;
	}
}

SElement::SElement() {
	XOJ_INIT_TYPE(SElement);

	this->element = new __RefSElement(); this->element->ref();
}

SElement::SElement(const SElement& elem) {
	XOJ_INIT_TYPE(SElement);

	this->element = elem.element;
	this->element->ref();
}

SElement::~SElement() {
	XOJ_CHECK_TYPE(SElement);

	this->element->unref();
	this->element = NULL;

	XOJ_RELEASE_TYPE(SElement);
}

void SElement::operator=(const SElement& elem) {
	XOJ_CHECK_TYPE(SElement);

	this->element = elem.element;
	this->element->ref();
}

std::map<String, SAttribute> & SElement::attributes() {
	XOJ_CHECK_TYPE(SElement);

	return this->element->attributes;
}

std::map<String, SElement> & SElement::children() {
	XOJ_CHECK_TYPE(SElement);

	return this->element->children;
}

void SElement::clear() {
	XOJ_CHECK_TYPE(SElement);

	this->element->attributes.clear();
	this->element->children.clear();
}

SElement & SElement::child(String name) {
	XOJ_CHECK_TYPE(SElement);

	return this->element->children[name];
}

void SElement::setComment(const String name, const String comment) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.comment = comment;
}

void SElement::setIntHex(const String name, const int value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setInt(const String name, const int value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setBool(const String name, const bool value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
}

void SElement::setString(const String name, const String value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.sValue = value;
	attrib.type = ATTRIBUTE_TYPE_STRING;
}

void SElement::setDouble(const String name, const double value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.dValue = value;
	attrib.type = ATTRIBUTE_TYPE_DOUBLE;
}

bool SElement::getDouble(const String name, double & value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE) {
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_DOUBLE) {
		return false;
	}

	value = attrib.dValue;

	return true;
}

bool SElement::getInt(const String name, int & value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE) {
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_INT && attrib.type != ATTRIBUTE_TYPE_INT_HEX) {
		return false;
	}

	value = attrib.iValue;

	return true;
}

bool SElement::getBool(const String name, bool & value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE) {
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_BOOLEAN) {
		return false;
	}

	value = attrib.iValue;

	return true;
}

bool SElement::getString(const String name, String & value) {
	XOJ_CHECK_TYPE(SElement);

	SAttribute & attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE) {
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_STRING) {
		return false;
	}

	value = attrib.sValue;

	return true;

}

