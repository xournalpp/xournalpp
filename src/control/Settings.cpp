/*
 * Xournal Extended
 *
 * Xournal Settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#include <stdlib.h>
#include "../gettext.h"

#include "Settings.h"

#include <string.h>

#define DEFAULT_FONT "Sans"
#define DEFAULT_FONT_SIZE 12

#define WRITE_BOOL_PROP(var) xmlNode = saveProperty((const char *)#var, var ? "true" : "false", root)
#define WRITE_STRING_PROP(var) xmlNode = saveProperty((const char *)#var, var.isEmpty() ? "" : var.c_str(), root)
#define WRITE_INT_PROP(var) xmlNode = saveProperty((const char *)#var, var, root)
#define WRITE_DOUBLE_PROP(var) xmlNode = savePropertyDouble((const char *)#var, var, root)
#define WRITE_COMMENT(var) com = xmlNewComment((const xmlChar *)var); \
	xmlAddPrevSibling(xmlNode, com);

Settings::Settings(String filename) {
	this->filename = filename;
	this->timeoutId = 1;
	this->saved = true;

	loadDefault();

	checkCanXInput();
}

Settings::~Settings() {
	if (timeoutId) {
		save();
	}

	for (int i = 0; i < 4; i++) {
		delete buttonConfig[i];
	}
}

void Settings::loadDefault() {
	this->useXinput = true;
	this->presureSensitivity = true;
	this->saved = true;
	this->canXIput = false;

	this->displayDpi = 72;

	this->font.setName(DEFAULT_FONT);
	this->font.setSize(DEFAULT_FONT_SIZE);

	this->mainWndWidth = 800;
	this->mainWndHeight = 600;

	this->showSidebar = true;
	this->sidebarOnRight = false;

	this->scrollbarOnLeft = false;

	this->widthMinimumMultiplier = 0.0;
	this->widthMaximumMultiplier = 1.25;

	this->autoloadPdfXoj = true;

	this->autosaveTimeout = 1;
	this->autosaveEnabled = true;

	this->defaultSaveName = _("%F-Note-%H-%M.xoj");

	// Eraser
	buttonConfig[0] = new ButtonConfig(TOOL_ERASER, 0, TOOL_SIZE_NONE, false, false, ERASER_TYPE_NONE);
	// Middle button
	buttonConfig[1] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, false, false, ERASER_TYPE_NONE);
	// Right button
	buttonConfig[2] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, false, false, ERASER_TYPE_NONE);
	// Touch
	buttonConfig[3] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, false, false, ERASER_TYPE_NONE);

	fullscreenHideElements = "mainMenubar";
	presentationHideElements = "mainMenubar,sidebarContents";

	this->pageInsertType = PAGE_INSERT_TYPE_COPY;
	this->pageBackgroundColor = 0xffffff; //white

	this->selectionColor = 0xff0000;
}

void Settings::parseData(xmlNodePtr cur, SElement & elem) {

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
	xmlChar *name;
	xmlChar *value;

	// Parse data map
	if (!xmlStrcmp(cur->name, (const xmlChar *) "data")) {
		name = xmlGetProp(cur, (const xmlChar *) "name");
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

	name = xmlGetProp(cur, (const xmlChar *) "name");
	if (name == NULL) {
		g_warning("Settings::%s:No name property!\n", cur->name);
		return;
	}

	if (xmlStrcmp(name, (const xmlChar *) "font") == 0) {
		xmlFree(name);
		xmlChar *font;
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

	value = xmlGetProp(cur, (const xmlChar *) "value");
	if (value == NULL) {
		xmlFree(name);
		g_warning("No value property!\n");
		return;
	}

	if (xmlStrcmp(name, (const xmlChar *) "presureSensitivity") == 0) {
		setPresureSensitivity(xmlStrcmp(value, (const xmlChar *) "true") ? false : true);
	} else if (xmlStrcmp(name, (const xmlChar *) "useXinput") == 0) {
		// Value is update from main after the window is created
		useXinput = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "selectedToolbar") == 0) {
		selectedToolbar = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "lastSavePath") == 0) {
		lastSavePath = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "displayDpi") == 0) {
		displayDpi = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "mainWndWidth") == 0) {
		mainWndWidth = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "mainWndHeight") == 0) {
		mainWndHeight = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "maximized") == 0) {
		maximized = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "showSidebar") == 0) {
		showSidebar = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "widthMinimumMultiplier") == 0) {
		widthMinimumMultiplier = g_ascii_strtod((const char *) value, NULL);
	} else if (xmlStrcmp(name, (const xmlChar *) "widthMaximumMultiplier") == 0) {
		widthMaximumMultiplier = g_ascii_strtod((const char *) value, NULL);
	} else if (xmlStrcmp(name, (const xmlChar *) "sidebarOnRight") == 0) {
		sidebarOnRight = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "scrollbarOnLeft") == 0) {
		scrollbarOnLeft = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "showTwoPages") == 0) {
		showTwoPages = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "autoloadPdfXoj") == 0) {
		autoloadPdfXoj = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "defaultSaveName") == 0) {
		this->defaultSaveName = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "autosaveEnabled") == 0) {
		autosaveEnabled = xmlStrcmp(value, (const xmlChar *) "true") ? false : true;
	} else if (xmlStrcmp(name, (const xmlChar *) "autosaveTimeout") == 0) {
		autosaveTimeout = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "fullscreenHideElements") == 0) {
		this->fullscreenHideElements = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "presentationHideElements") == 0) {
		this->presentationHideElements = (const char *) value;
	} else if (xmlStrcmp(name, (const xmlChar *) "pageInsertType") == 0) {
		this->pageInsertType = pageInsertTypeFromString((const char *) value);
	} else if (xmlStrcmp(name, (const xmlChar *) "pageBackgroundColor") == 0) {
		this->pageBackgroundColor = g_ascii_strtoll((const char *) value, NULL, 10);
	} else if (xmlStrcmp(name, (const xmlChar *) "selectionColor") == 0) {
		this->selectionColor = g_ascii_strtoll((const char *) value, NULL, 10);
	}

	xmlFree(name);
	xmlFree(value);
}

void Settings::loadButtonConfig() {
	SElement & s = getElement("buttonConfig");

	const char * BUTTON_NAMES[] = { "middle", "right", "eraser", "touch" };

	for (int i = 0; i < 4; i++) {
		SElement & e = s.child(BUTTON_NAMES[i]);
		ButtonConfig * cfg = buttonConfig[i];

		String sType;
		if (e.getString("tool", sType)) {
			ToolType type = toolTypeFromString(sType);
			cfg->action = type;

			if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
				e.getBool("rouler", cfg->rouler);
				e.getBool("shapeRecognizer", cfg->shapeRecognizer);
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
	xmlDocPtr doc;
	xmlNodePtr cur;

	xmlKeepBlanksDefault(0);

	if (!g_file_test(filename.c_str(), G_FILE_TEST_EXISTS)) {
		g_warning("configfile does not exist %s\n", filename.c_str());
		return false;
	}

	doc = xmlParseFile(filename.c_str());

	if (doc == NULL) {
		g_warning("doc == null\n");
		return false;
	}

	cur = xmlDocGetRootElement(doc);
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

gboolean saveCallback(gpointer data) {
	((Settings*) data)->save();
	return false;
}

void Settings::saveTimeout() {
	if (timeoutId) {
		return;
	}

	timeoutId = g_timeout_add_seconds(2, (GSourceFunc) &saveCallback, this);
}

xmlNodePtr Settings::savePropertyDouble(const gchar *key, double value, xmlNodePtr parent) {
	char * text = g_strdup_printf("%0.3lf", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar *key, int value, xmlNodePtr parent) {
	char * text = g_strdup_printf("%i", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar *key, const gchar *value, xmlNodePtr parent) {
	xmlNodePtr xmlNode;

	xmlNode = xmlNewChild(parent, NULL, (const xmlChar *) "property", NULL);

	xmlSetProp(xmlNode, (const xmlChar *) "name", (const xmlChar *) key);

	xmlSetProp(xmlNode, (const xmlChar *) "value", (const xmlChar *) value);

	return xmlNode;
}

void Settings::saveButtonConfig() {
	SElement & s = getElement("buttonConfig");
	s.clear();

	const char * BUTTON_NAMES[] = { "middle", "right", "eraser", "touch" };

	for (int i = 0; i < 4; i++) {
		SElement & e = s.child(BUTTON_NAMES[i]);
		ButtonConfig * cfg = buttonConfig[i];

		ToolType type = cfg->action;
		e.setString("tool", toolTypeToString(type));

		if (type == TOOL_PEN || type == TOOL_HILIGHTER) {
			e.setBool("rouler", cfg->rouler);
			e.setBool("shapeRecognizer", cfg->shapeRecognizer);
			e .setString("size", toolSizeToString(cfg->size));
		}

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
	if (timeoutId) {
		g_source_remove(timeoutId);
		timeoutId = 0;
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

	WRITE_STRING_PROP(selectedToolbar);
	WRITE_STRING_PROP(lastSavePath);

	WRITE_INT_PROP(displayDpi);
	WRITE_INT_PROP(mainWndWidth);
	WRITE_INT_PROP(mainWndHeight);
	WRITE_BOOL_PROP(maximized);

	WRITE_BOOL_PROP(showSidebar);
	WRITE_BOOL_PROP(sidebarOnRight);
	WRITE_BOOL_PROP(scrollbarOnLeft);
	WRITE_BOOL_PROP(showTwoPages);

	WRITE_STRING_PROP(fullscreenHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)");

	WRITE_STRING_PROP(presentationHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Presentation mode, separated by a colon (,)");

	WRITE_BOOL_PROP(autoloadPdfXoj);
	WRITE_STRING_PROP(defaultSaveName);

	WRITE_BOOL_PROP(autosaveEnabled);
	WRITE_INT_PROP(autosaveTimeout);

	String pageInsertType = pageInsertTypeToString(this->pageInsertType);
	WRITE_STRING_PROP(pageInsertType);

	WRITE_INT_PROP(pageBackgroundColor);
	WRITE_INT_PROP(selectionColor);

	WRITE_DOUBLE_PROP(widthMinimumMultiplier);
	WRITE_COMMENT("The multiplier for the presure sensitivity of the pen");
	WRITE_DOUBLE_PROP(widthMaximumMultiplier);
	WRITE_COMMENT("The multiplier for the presure sensitivity of the pen");

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
	xmlNodePtr xmlNode;

	xmlNode = xmlNewChild(root, NULL, (const xmlChar *) "data", NULL);

	xmlSetProp(xmlNode, (const xmlChar *) "name", (const xmlChar *) name.c_str());

	std::map<String, SAttribute>::iterator it;
	for (it = elem.attributes().begin(); it != elem.attributes().end(); it++) {
		String aname = (*it).first;
		SAttribute & attrib = (*it).second;
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
	return presureSensitivity;
}

bool Settings::isXinputEnabled() {
	return useXinput;
}

bool Settings::isSidebarOnRight() {
	return sidebarOnRight;
}

void Settings::setSidebarOnRight(bool right) {
	if (sidebarOnRight == right) {
		return;
	}

	sidebarOnRight = right;

	saveTimeout();
}

bool Settings::isScrollbarOnLeft() {
	return scrollbarOnLeft;
}

void Settings::setScrollbarOnLeft(bool right) {
	if (scrollbarOnLeft == right) {
		return;
	}

	scrollbarOnLeft = right;

	saveTimeout();
}

int Settings::getAutosaveTimeout() {
	return this->autosaveTimeout;
}

void Settings::setAutosaveTimeout(int autosave) {
	if (this->autosaveTimeout == autosave) {
		return;
	}

	this->autosaveTimeout = autosave;

	saveTimeout();
}

bool Settings::isAutosaveEnabled() {
	return this->autosaveEnabled;
}

void Settings::setAutosaveEnabled(bool autosave) {
	if (this->autosaveEnabled == autosave) {
		return;
	}

	this->autosaveEnabled = autosave;

	saveTimeout();
}

bool Settings::isAutloadPdfXoj() {
	return this->autoloadPdfXoj;
}

void Settings::setAutoloadPdfXoj(bool load) {
	if (this->autoloadPdfXoj == load) {
		return;
	}
	this->autoloadPdfXoj = load;
	saveTimeout();
}

String Settings::getDefaultSaveName() {
	return this->defaultSaveName;
}

void Settings::setDefaultSaveName(String name) {
	if (this->defaultSaveName == name) {
		return;
	}

	this->defaultSaveName = name;

	saveTimeout();
}

void Settings::setShowTwoPages(bool showTwoPages) {
	if (this->showTwoPages == showTwoPages) {
		return;
	}

	this->showTwoPages = showTwoPages;
	saveTimeout();
}

bool Settings::isShowTwoPages() {
	return this->showTwoPages;
}

/**
 * XInput is available
 */
bool Settings::isXInputAvailable() {
	return canXIput;
}

/**
 * XInput should be used in the application
 */
bool Settings::isUseXInput() {
	return useXinput && canXIput;
}

void Settings::setPresureSensitivity(gboolean presureSensitivity) {
	if (this->presureSensitivity == presureSensitivity) {
		return;
	}
	this->presureSensitivity = presureSensitivity;

	saveTimeout();
}

void Settings::setLastSavePath(String path) {
	this->lastSavePath = path;
	saveTimeout();
}

String Settings::getLastSavePath() {
	return lastSavePath;
}

void Settings::setDisplayDpi(int dpi) {
	this->displayDpi = dpi;
}

int Settings::getDisplayDpi() {
	return displayDpi;
}

bool Settings::isSidebarVisible() {
	return showSidebar;
}

void Settings::setSidebarVisible(bool visible) {
	if (showSidebar == visible) {
		return;
	}
	showSidebar = visible;
	saveTimeout();
}

void Settings::checkCanXInput() {
	canXIput = FALSE;
	GList *devList = gdk_devices_list();
	GdkDevice *device;

	while (devList != NULL) {
		device = (GdkDevice *) devList->data;
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
	//	ui.use_xinput = ui.allow_xinput && can_xinput;
}

void Settings::setMainWndSize(int width, int height) {
	this->mainWndWidth = width;
	this->mainWndHeight = height;

	saveTimeout();
}

int Settings::getMainWndWidth() {
	return mainWndWidth;
}

int Settings::getMainWndHeight() {
	return mainWndHeight;
}

bool Settings::isMainWndMaximized() {
	return maximized;
}

void Settings::setMainWndMaximized(bool max) {
	maximized = max;
}

void Settings::setXinputEnabled(gboolean useXinput) {
	if (this->useXinput == useXinput) {
		return;
	}

	this->useXinput = useXinput;

	saveTimeout();
}

double Settings::getWidthMinimumMultiplier() {
	return widthMinimumMultiplier;
}

double Settings::getWidthMaximumMultiplier() {
	return widthMaximumMultiplier;
}

void Settings::setSelectedToolbar(String name) {
	if (selectedToolbar == name) {
		return;
	}
	selectedToolbar = name;
	saveTimeout();
}

String Settings::getSelectedToolbar() {
	return selectedToolbar;
}

SElement & Settings::getElement(String name) {
	return data[name];
}

void Settings::customSettingsChanged() {
	saveTimeout();
}

ButtonConfig * Settings::getButtonConfig(int id) {
	return buttonConfig[id];
}

ButtonConfig * Settings::getEraserButtonConfig() {
	return buttonConfig[0];
}

ButtonConfig * Settings::getMiddleButtonConfig() {
	return buttonConfig[1];
}

ButtonConfig * Settings::getRightButtonConfig() {
	return buttonConfig[2];
}

ButtonConfig * Settings::getTouchButtonConfig() {
	return buttonConfig[3];
}

String Settings::getFullscreenHideElements() {
	return fullscreenHideElements;
}

void Settings::setFullscreenHideElements(String elements) {
	this->fullscreenHideElements = elements;
	saveTimeout();
}

String Settings::getPresentationHideElements() {
	return presentationHideElements;
}

void Settings::setPresentationHideElements(String elements) {
	this->presentationHideElements = elements;
	saveTimeout();
}

PageInsertType Settings::getPageInsertType() {
	return this->pageInsertType;
}

void Settings::setPageInsertType(PageInsertType type) {
	if (this->pageInsertType == type) {
		return;
	}
	this->pageInsertType = type;
	saveTimeout();
}

int Settings::getPageBackgroundColor() {
	return this->pageBackgroundColor;
}

void Settings::setPageBackgroundColor(int color) {
	if (this->pageBackgroundColor == color) {
		return;
	}
	this->pageBackgroundColor = color;
	saveTimeout();
}

int Settings::getSelectionColor() {
	return this->selectionColor;
}

void Settings::setSelectionColor(int color) {
	if (this->selectionColor == color) {
		return;
	}
	this->selectionColor = color;
	saveTimeout();
}

XojFont & Settings::getFont() {
	return this->font;
}

void Settings::setFont(const XojFont & font) {
	this->font = font;
}

//////////////////////////////////////////////////

SAttribute::SAttribute() {
	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;
}

//////////////////////////////////////////////////


ButtonConfig::ButtonConfig(ToolType action, int color, ToolSize size, bool shapeRecognizer, bool rouler,
		EraserType eraserMode) {
	this->action = action;
	this->color = color;
	this->shapeRecognizer = shapeRecognizer;
	this->rouler = rouler;
	this->eraserMode = eraserMode;
	this->disableDrawing = false;
}

//////////////////////////////////////////////////

__RefSElement::__RefSElement() {
	this->refcount = 0;
}

__RefSElement::~__RefSElement() {
}

void __RefSElement::ref() {
	refcount++;
}
void __RefSElement::unref() {
	refcount--;
	if (refcount == 0) {
		delete this;
	}
}

SElement::SElement() {
	this->element = new __RefSElement(); this->element->ref();
}

SElement::SElement(const SElement& elem) {
	this->element = elem.element;
	this->element->ref();
}

SElement::~SElement() {
	this->element->unref();
	this->element = NULL;
}

void SElement::operator=(const SElement& elem) {
	this->element = elem.element;
	this->element->ref();
}

std::map<String, SAttribute> & SElement::attributes() {
	return this->element->attributes;
}

std::map<String, SElement> & SElement::children() {
	return this->element->children;
}

void SElement::clear() {
	this->element->attributes.clear();
	this->element->children.clear();
}

SElement & SElement::child(String name) {
	return this->element->children[name];
}

void SElement::setComment(const String name, const String comment) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.comment = comment;
}

void SElement::setIntHex(const String name, const int value) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setInt(const String name, const int value) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setBool(const String name, const bool value) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
}

void SElement::setString(const String name, const String value) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.sValue = value;
	attrib.type = ATTRIBUTE_TYPE_STRING;
}

void SElement::setDouble(const String name, const double value) {
	SAttribute & attrib = this->element->attributes[name];
	attrib.dValue = value;
	attrib.type = ATTRIBUTE_TYPE_DOUBLE;
}

bool SElement::getDouble(const String name, double & value) {
	SAttribute & attrib = this->element->attributes[name];
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
	SAttribute & attrib = this->element->attributes[name];
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
	SAttribute & attrib = this->element->attributes[name];
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
	SAttribute & attrib = this->element->attributes[name];
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

