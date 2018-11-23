#include "Settings.h"

#include "ButtonConfig.h"
#include "model/FormatDefinitions.h"

#include <config.h>
#include <i18n.h>

#include <boost/filesystem.hpp>

#include <string.h>

#define DEFAULT_FONT "Sans"
#define DEFAULT_FONT_SIZE 12

#define WRITE_BOOL_PROP(var) xmlNode = saveProperty((const char *)#var, var ? "true" : "false", root)
#define WRITE_STRING_PROP(var) xmlNode = saveProperty((const char *)#var, var.empty() ? "" : var.c_str(), root)
#define WRITE_INT_PROP(var) xmlNode = saveProperty((const char *)#var, var, root)
#define WRITE_DOUBLE_PROP(var) xmlNode = savePropertyDouble((const char *)#var, var, root)
#define WRITE_COMMENT(var) com = xmlNewComment((const xmlChar *)var); xmlAddPrevSibling(xmlNode, com);

const char* BUTTON_NAMES[] = {"middle", "right", "eraser", "touch", "default", "stylus", "stylus2"};

Settings::Settings(path filename)
{
	XOJ_INIT_TYPE(Settings);

	this->filename = filename;

	loadDefault();
}

Settings::~Settings()
{
	XOJ_CHECK_TYPE(Settings);

	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		delete this->buttonConfig[i];
		this->buttonConfig[i] = NULL;
	}

	XOJ_RELEASE_TYPE(Settings);
}

void Settings::loadDefault()
{
	XOJ_CHECK_TYPE(Settings);

	this->presureSensitivity = true;
	this->maximized = false;
	this->showTwoPages = false;
	this->presentationMode = false;

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

	//Set this for autosave frequency in minutes.
	this->autosaveTimeout = 3;
	this->autosaveEnabled = true;

	this->addHorizontalSpace = false;
	this->addVerticalSpace = false;

	this->enableLeafEnterWorkaround = true;

	this->defaultSaveName = _("%F-Note-%H-%M.xoj");

	// Eraser
	this->buttonConfig[0] = new ButtonConfig(TOOL_ERASER, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Middle button
	this->buttonConfig[1] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Right button
	this->buttonConfig[2] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Touch
	this->buttonConfig[3] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Default config
	this->buttonConfig[4] = new ButtonConfig(TOOL_PEN, 0, TOOL_SIZE_FINE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Pen button 1
	this->buttonConfig[5] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
	// Pen button 2
	this->buttonConfig[6] = new ButtonConfig(TOOL_NONE, 0, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);

	this->fullscreenHideElements = "mainMenubar";
	this->presentationHideElements = "mainMenubar,sidebarContents";

	this->pdfPageCacheSize = 10;

	this->selectionColor = 0xff0000;

	this->backgroundColor = 0xDCDAD5;

	this->eventCompression = true;

	this->pageTemplate = "xoj/template\ncopyLastPageSettings=true\nsize=595.275591x841.889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";

	inTransaction = false;
}

void Settings::parseData(xmlNodePtr cur, SElement& elem)
{
	XOJ_CHECK_TYPE(Settings);

	for (xmlNodePtr x = cur->children; x != NULL; x = x->next)
	{
		if (!xmlStrcmp(x->name, (const xmlChar*) "data"))
		{
			xmlChar* name = xmlGetProp(x, (const xmlChar*) "name");
			parseData(x, elem.child((const char*) name));
			xmlFree(name);
		}
		else if (!xmlStrcmp(x->name, (const xmlChar*) "attribute"))
		{
			xmlChar* name = xmlGetProp(x, (const xmlChar*) "name");
			xmlChar* value = xmlGetProp(x, (const xmlChar*) "value");
			xmlChar* type = xmlGetProp(x, (const xmlChar*) "type");

			string sType = (const char*) type;

			if (sType == "int")
			{
				int i = atoi((const char*) value);
				elem.setInt((const char*) name, i);
			}
			else if (sType == "double")
			{
				double d = atof((const char*) value);
				elem.setDouble((const char*) name, d);
			}
			else if (sType == "hex")
			{
				int i = 0;
				if (sscanf((const char*) value, "%x", &i))
				{
					elem.setIntHex((const char*) name, i);
				}
				else
				{
					g_warning("Settings::Unknown hex value: %s:%s\n", name, value);
				}
			}
			else if (sType == "string")
			{
				elem.setString((const char*) name, (const char*) value);
			}
			else if (sType == "boolean")
			{
				elem.setBool((const char*) name, strcmp((const char*) value, "true") == 0);
			}
			else
			{
				g_warning("Settings::Unknown datatype: %s\n", sType.c_str());
			}

			xmlFree(name);
			xmlFree(type);
			xmlFree(value);
		}
		else
		{
			g_warning("Settings::parseData: Unknown XML node: %s\n", x->name);
			continue;
		}
	}

}

void Settings::parseItem(xmlDocPtr doc, xmlNodePtr cur)
{
	XOJ_CHECK_TYPE(Settings);

	// Parse data map
	if (!xmlStrcmp(cur->name, (const xmlChar*) "data"))
	{
		xmlChar* name = xmlGetProp(cur, (const xmlChar*) "name");
		if (name == NULL)
		{
			g_warning("Settings::%s:No name property!\n", cur->name);
			return;
		}

		parseData(cur, data[(const char*) name]);

		xmlFree(name);
		return;
	}

	if (cur->type == XML_COMMENT_NODE)
	{
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar*) "property"))
	{
		g_warning("Settings::Unknown XML node: %s\n", cur->name);
		return;
	}

	xmlChar* name = xmlGetProp(cur, (const xmlChar*) "name");
	if (name == NULL)
	{
		g_warning("Settings::%s:No name property!\n", cur->name);
		return;
	}

	if (xmlStrcmp(name, (const xmlChar*) "font") == 0)
	{
		xmlFree(name);
		xmlChar* font;
		xmlChar* size;

		font = xmlGetProp(cur, (const xmlChar*) "font");
		if (font)
		{
			this->font.setName((const char*) font);
			xmlFree(font);
		}

		size = xmlGetProp(cur, (const xmlChar*) "size");
		if (size)
		{
			double dSize = DEFAULT_FONT_SIZE;
			if (sscanf((const char*) size, "%lf", &dSize) == 1)
			{
				this->font.setSize(dSize);
			}

			xmlFree(size);
		}

		return;
	}

	xmlChar* value = xmlGetProp(cur, (const xmlChar*) "value");
	if (value == NULL)
	{
		xmlFree(name);
		g_warning("No value property!\n");
		return;
	}

	if (xmlStrcmp(name, (const xmlChar*) "presureSensitivity") == 0)
	{
		setPresureSensitivity(xmlStrcmp(value, (const xmlChar*) "true") ? false : true);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "selectedToolbar") == 0)
	{
		this->selectedToolbar = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "lastSavePath") == 0)
	{
		this->lastSavePath = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "lastImagePath") == 0)
	{
		this->lastImagePath = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "displayDpi") == 0)
	{
		this->displayDpi = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "mainWndWidth") == 0)
	{
		this->mainWndWidth = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "mainWndHeight") == 0)
	{
		this->mainWndHeight = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "maximized") == 0)
	{
		this->maximized = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "showSidebar") == 0)
	{
		this->showSidebar = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "sidebarWidth") == 0)
	{
		this->sidebarWidth = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "widthMinimumMultiplier") == 0)
	{
		this->widthMinimumMultiplier = g_ascii_strtod((const char*) value, NULL);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "widthMaximumMultiplier") == 0)
	{
		this->widthMaximumMultiplier = g_ascii_strtod((const char*) value, NULL);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "eventCompression") == 0)
	{
		this->eventCompression = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "sidebarOnRight") == 0)
	{
		this->sidebarOnRight = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "scrollbarOnLeft") == 0)
	{
		this->scrollbarOnLeft = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "showTwoPages") == 0)
	{
		this->showTwoPages = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "presentationMode") == 0)
	{
		this->presentationMode = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "autoloadPdfXoj") == 0)
	{
		this->autoloadPdfXoj = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "showBigCursor") == 0)
	{
		this->showBigCursor = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "defaultSaveName") == 0)
	{
		this->defaultSaveName = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "pageTemplate") == 0)
	{
		this->pageTemplate = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "sizeUnit") == 0)
	{
		this->sizeUnit = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "autosaveEnabled") == 0)
	{
		this->autosaveEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "autosaveTimeout") == 0)
	{
		this->autosaveTimeout = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "fullscreenHideElements") == 0)
	{
		this->fullscreenHideElements = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "presentationHideElements") == 0)
	{
		this->presentationHideElements = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "pdfPageCacheSize") == 0)
	{
		this->pdfPageCacheSize = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "selectionColor") == 0)
	{
		this->selectionColor = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "backgroundColor") == 0)
	{
		this->backgroundColor = g_ascii_strtoll((const char*) value, NULL, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addVerticalSpace") == 0)
	{
		this->addVerticalSpace = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addHorizontalSpace") == 0)
	{
		this->addHorizontalSpace = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "enableLeafEnterWorkaround") == 0)
	{
		this->enableLeafEnterWorkaround = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "scrollbarHideType") == 0)
	{
		if (xmlStrcmp(value, (const xmlChar*) "both") == 0)
		{
			this->scrollbarHideType = SCROLLBAR_HIDE_BOTH;
		}
		else if (xmlStrcmp(value, (const xmlChar*) "horizontal") == 0)
		{
			this->scrollbarHideType = SCROLLBAR_HIDE_HORIZONTAL;
		}
		else if (xmlStrcmp(value, (const xmlChar*) "vertical") == 0)
		{
			this->scrollbarHideType = SCROLLBAR_HIDE_VERTICAL;
		}
		else
		{
			this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
		}
	}

	xmlFree(name);
	xmlFree(value);
}

void Settings::loadButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	SElement& s = getCustomElement("buttonConfig");

	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		SElement& e = s.child(BUTTON_NAMES[i]);
		ButtonConfig* cfg = buttonConfig[i];

		string sType;
		if (e.getString("tool", sType))
		{
			ToolType type = toolTypeFromString(sType);
			cfg->action = type;

			if (type == TOOL_PEN || type == TOOL_HILIGHTER)
			{
				string drawingType;
				if (e.getString("drawingType", drawingType))
				{
					cfg->drawingType = drawingTypeFromString(drawingType);
				}

				string sSize;
				if (e.getString("size", sSize))
				{
					cfg->size = toolSizeFromString(sSize);
				}
				else
				{
					// If not specified: do not change
					cfg->size = TOOL_SIZE_NONE;
				}
			}

			if (type == TOOL_PEN || type == TOOL_HILIGHTER || type == TOOL_TEXT)
			{
				e.getInt("color", cfg->color);
			}

			if (type == TOOL_ERASER)
			{
				string sEraserMode;
				if (e.getString("eraserMode", sEraserMode))
				{
					cfg->eraserMode = eraserTypeFromString(sEraserMode);
				}
				else
				{
					// If not specified: do not change
					cfg->eraserMode = ERASER_TYPE_NONE;
				}
			}

			// Touch device
			if (i == 3)
			{
				if (!e.getString("device", cfg->device))
				{
					cfg->device = "";
				}

				e.getBool("disableDrawing", cfg->disableDrawing);
			}
		}
		else
		{
			continue;
		}
	}
}

bool Settings::load()
{
	XOJ_CHECK_TYPE(Settings);

	xmlKeepBlanksDefault(0);

	if (!boost::filesystem::exists(filename))
	{
		g_warning("configfile does not exist %s\n", filename.c_str());
		return false;
	}

	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if (doc == NULL)
	{
		g_warning("Settings::load:: doc == null, could not load Settings!\n");
		return false;
	}

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (cur == NULL)
	{
		g_message("The settings file \"%s\" is empty", filename.c_str());
		xmlFreeDoc(doc);

		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar*) "settings"))
	{
		g_message("File \"%s\" is of the wrong type", filename.c_str());
		xmlFreeDoc(doc);

		return false;
	}

	cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;

	while (cur != NULL)
	{
		parseItem(doc, cur);

		cur = cur->next;
	}

	xmlFreeDoc(doc);

	loadButtonConfig();

	return true;
}

xmlNodePtr Settings::savePropertyDouble(const gchar* key, double value, xmlNodePtr parent)
{
	XOJ_CHECK_TYPE(Settings);

	char* text = g_strdup_printf("%0.3lf", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar* key, int value, xmlNodePtr parent)
{
	XOJ_CHECK_TYPE(Settings);

	char* text = g_strdup_printf("%i", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar* key, const gchar* value, xmlNodePtr parent)
{
	XOJ_CHECK_TYPE(Settings);

	xmlNodePtr xmlNode = xmlNewChild(parent, NULL, (const xmlChar*) "property", NULL);

	xmlSetProp(xmlNode, (const xmlChar*) "name", (const xmlChar*) key);

	xmlSetProp(xmlNode, (const xmlChar*) "value", (const xmlChar*) value);

	return xmlNode;
}

void Settings::saveButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	SElement& s = getCustomElement("buttonConfig");
	s.clear();

	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		SElement& e = s.child(BUTTON_NAMES[i]);
		ButtonConfig* cfg = buttonConfig[i];

		ToolType type = cfg->action;
		e.setString("tool", toolTypeToString(type));

		if (type == TOOL_PEN || type == TOOL_HILIGHTER)
		{
			e.setString("drawingType", drawingTypeToString(cfg->drawingType));
			e.setString("size", toolSizeToString(cfg->size));
		} // end if pen or highlighter

		if (type == TOOL_PEN || type == TOOL_HILIGHTER || type == TOOL_TEXT)
		{
			e.setIntHex("color", cfg->color);
		}

		if (type == TOOL_ERASER)
		{
			e.setString("eraserMode", eraserTypeToString(cfg->eraserMode));
		}

		// Touch device
		if (i == 3)
		{
			e.setString("device", cfg->device);
			e.setBool("disableDrawing", cfg->disableDrawing);
		}
	}
}

/**
 * Do not save settings until transactionEnd() is called
 */
void Settings::transactionStart()
{
	XOJ_CHECK_TYPE(Settings);

	inTransaction = true;
}

/**
 * Stop transaction and save settings
 */
void Settings::transactionEnd()
{
	XOJ_CHECK_TYPE(Settings);

	inTransaction = false;
	save();
}

void Settings::save()
{
	XOJ_CHECK_TYPE(Settings);

	if (inTransaction)
	{
		return;
	}

	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr xmlNode;

	xmlIndentTreeOutput = TRUE;

	doc = xmlNewDoc((const xmlChar*) "1.0");
	if (doc == NULL)
	{
		return;
	}

	saveButtonConfig();

	/* Create metadata root */
	root = xmlNewDocNode(doc, NULL, (const xmlChar*) "settings", NULL);
	xmlDocSetRootElement(doc, root);
	xmlNodePtr com = xmlNewComment((const xmlChar*)
								   "The Xournal++ settings file. Do not edit this file! "
								   "The most settings are available in the Settings dialog, "
								   "the others are commented in this file, but handle with care!");
	xmlAddPrevSibling(root, com);

	WRITE_BOOL_PROP(presureSensitivity);

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
	WRITE_BOOL_PROP(presentationMode);

	WRITE_STRING_PROP(fullscreenHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)");

	WRITE_STRING_PROP(presentationHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Presentation mode, separated by a colon (,)");

	WRITE_BOOL_PROP(showBigCursor);

	if (this->scrollbarHideType == SCROLLBAR_HIDE_BOTH)
	{
		saveProperty((const char*) "scrollbarHideType", "both", root);
	}
	else if (this->scrollbarHideType == SCROLLBAR_HIDE_HORIZONTAL)
	{
		saveProperty((const char*) "scrollbarHideType", "horizontal", root);
	}
	else if (this->scrollbarHideType == SCROLLBAR_HIDE_VERTICAL)
	{
		saveProperty((const char*) "scrollbarHideType", "vertical", root);
	}
	else
	{
		saveProperty((const char*) "scrollbarHideType", "none", root);
	}


	WRITE_BOOL_PROP(autoloadPdfXoj);
	WRITE_COMMENT("Hides scroolbars in the main window, allowed values: \"none\", \"horizontal\", \"vertical\", \"both\"");

	WRITE_STRING_PROP(defaultSaveName);

	WRITE_BOOL_PROP(autosaveEnabled);
	WRITE_INT_PROP(autosaveTimeout);

	WRITE_BOOL_PROP(addHorizontalSpace);
	WRITE_BOOL_PROP(addVerticalSpace);

	WRITE_BOOL_PROP(enableLeafEnterWorkaround);
	WRITE_COMMENT("If Xournal++ crashes if you e.g. unplug your mouse set this to true. "
				  "If you have input problems, you can turn it of with false.");

	WRITE_INT_PROP(selectionColor);
	WRITE_INT_PROP(backgroundColor);

	WRITE_INT_PROP(pdfPageCacheSize);
	WRITE_COMMENT("The count of rendered PDF pages which will be cached.");

	WRITE_DOUBLE_PROP(widthMinimumMultiplier);
	WRITE_COMMENT("The multiplier for the pressure sensitivity of the pen");
	WRITE_DOUBLE_PROP(widthMaximumMultiplier);
	WRITE_COMMENT("The multiplier for the pressure sensitivity of the pen");

	WRITE_BOOL_PROP(eventCompression);

	WRITE_COMMENT("Config for new pages");
	WRITE_STRING_PROP(pageTemplate);

	WRITE_STRING_PROP(sizeUnit);

	xmlNodePtr xmlFont;
	xmlFont = xmlNewChild(root, NULL, (const xmlChar*) "property", NULL);
	xmlSetProp(xmlFont, (const xmlChar*) "name", (const xmlChar*) "font");
	xmlSetProp(xmlFont, (const xmlChar*) "font", (const xmlChar*) this->font.getName().c_str());
	gchar* sSize = g_strdup_printf("%0.1lf", this->font.getSize());
	xmlSetProp(xmlFont, (const xmlChar*) "size", (const xmlChar*) sSize);
	g_free(sSize);

	for (std::map<string, SElement>::value_type p: data)
	{
		saveData(root, p.first, p.second);
	}

	xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);
	xmlFreeDoc(doc);
}

void Settings::saveData(xmlNodePtr root, string name, SElement& elem)
{
	XOJ_CHECK_TYPE(Settings);

	xmlNodePtr xmlNode = xmlNewChild(root, NULL, (const xmlChar*) "data", NULL);

	xmlSetProp(xmlNode, (const xmlChar*) "name", (const xmlChar*) name.c_str());

	for (std::map<string, SAttribute>::value_type p : elem.attributes())
	{
		string aname = p.first;
		SAttribute& attrib = p.second;

		XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

		string type;
		string value;

		if (attrib.type == ATTRIBUTE_TYPE_BOOLEAN)
		{
			type = "boolean";

			if (attrib.iValue)
			{
				value = "true";
			}
			else
			{
				value = "false";
			}
		}
		else if (attrib.type == ATTRIBUTE_TYPE_INT)
		{
			type = "int";

			char* tmp = g_strdup_printf("%i", attrib.iValue);
			value = tmp;
			g_free(tmp);
		}
		else if (attrib.type == ATTRIBUTE_TYPE_DOUBLE)
		{
			type = "double";

			char* tmp = g_strdup_printf("%lf", attrib.dValue);
			value = tmp;
			g_free(tmp);
		}
		else if (attrib.type == ATTRIBUTE_TYPE_INT_HEX)
		{
			type = "hex";

			char* tmp = g_strdup_printf("%06x", attrib.iValue);
			value = tmp;
			g_free(tmp);
		}
		else if (attrib.type == ATTRIBUTE_TYPE_STRING)
		{
			type = "string";
			value = attrib.sValue;
		}
		else
		{
			// Unknown type or empty attribute
			continue;
		}

		xmlNodePtr at;
		at = xmlNewChild(xmlNode, NULL, (const xmlChar*) "attribute", NULL);

		xmlSetProp(at, (const xmlChar*) "name", (const xmlChar*) aname.c_str());
		xmlSetProp(at, (const xmlChar*) "type", (const xmlChar*) type.c_str());
		xmlSetProp(at, (const xmlChar*) "value", (const xmlChar*) value.c_str());

		if (!attrib.comment.empty())
		{
			xmlNodePtr com = xmlNewComment((const xmlChar*) attrib.comment.c_str());
			xmlAddPrevSibling(xmlNode, com);
		}
	}

	for (std::map<string, SElement>::value_type p : elem.children())
	{
		saveData(xmlNode, p.first, p.second);
	}
}

// Getter- / Setter
bool Settings::isPresureSensitivity()
{
	XOJ_CHECK_TYPE(Settings);

	return this->presureSensitivity;
}

bool Settings::isSidebarOnRight()
{
	XOJ_CHECK_TYPE(Settings);

	return this->sidebarOnRight;
}

void Settings::setSidebarOnRight(bool right)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->sidebarOnRight == right)
	{
		return;
	}

	this->sidebarOnRight = right;

	save();
}

bool Settings::isScrollbarOnLeft()
{
	XOJ_CHECK_TYPE(Settings);

	return this->scrollbarOnLeft;
}

void Settings::setScrollbarOnLeft(bool right)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->scrollbarOnLeft == right)
	{
		return;
	}

	this->scrollbarOnLeft = right;

	save();
}

int Settings::getAutosaveTimeout()
{
	return this->autosaveTimeout;
}

void Settings::setAutosaveTimeout(int autosave)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->autosaveTimeout == autosave)
	{
		return;
	}

	this->autosaveTimeout = autosave;

	save();
}

bool Settings::isAutosaveEnabled()
{
	XOJ_CHECK_TYPE(Settings);

	return this->autosaveEnabled;
}

void Settings::setAutosaveEnabled(bool autosave)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->autosaveEnabled == autosave)
	{
		return;
	}

	this->autosaveEnabled = autosave;

	save();
}

bool Settings::getAddVerticalSpace()
{
	XOJ_CHECK_TYPE(Settings);

	return this->addVerticalSpace;
}

void Settings::setAddVerticalSpace(bool space)
{
	XOJ_CHECK_TYPE(Settings);

	this->addVerticalSpace = space;
}

bool Settings::getAddHorizontalSpace()
{
	XOJ_CHECK_TYPE(Settings);

	return this->addHorizontalSpace;
}

void Settings::setAddHorizontalSpace(bool space)
{
	XOJ_CHECK_TYPE(Settings);

	this->addHorizontalSpace = space;
}

bool Settings::isEnableLeafEnterWorkaround()
{
	XOJ_CHECK_TYPE(Settings);

	return this->enableLeafEnterWorkaround;
}

void Settings::setEnableLeafEnterWorkaround(bool enable)
{
	XOJ_CHECK_TYPE(Settings);

	this->enableLeafEnterWorkaround = enable;

	save();
}

bool Settings::isShowBigCursor()
{
	XOJ_CHECK_TYPE(Settings);

	return this->showBigCursor;
}

void Settings::setShowBigCursor(bool b)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->showBigCursor == b)
	{
		return;
	}

	this->showBigCursor = b;
	save();
}

ScrollbarHideType Settings::getScrollbarHideType()
{
	XOJ_CHECK_TYPE(Settings);

	return this->scrollbarHideType;
}

void Settings::setScrollbarHideType(ScrollbarHideType type)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->scrollbarHideType == type)
	{
		return;
	}

	this->scrollbarHideType = type;

	save();
}

bool Settings::isAutloadPdfXoj()
{
	XOJ_CHECK_TYPE(Settings);

	return this->autoloadPdfXoj;
}

void Settings::setAutoloadPdfXoj(bool load)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->autoloadPdfXoj == load)
	{
		return;
	}
	this->autoloadPdfXoj = load;
	save();
}

string Settings::getDefaultSaveName()
{
	XOJ_CHECK_TYPE(Settings);

	return this->defaultSaveName;
}

void Settings::setDefaultSaveName(string name)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->defaultSaveName == name)
	{
		return;
	}

	this->defaultSaveName = name;

	save();
}

bool Settings::isEventCompression()
{
	XOJ_CHECK_TYPE(Settings);

	return this->eventCompression;
}

void Settings::setEventCompression(bool enabled)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->eventCompression == enabled)
	{
		return;
	}

	this->eventCompression = enabled;

	save();
}

string Settings::getPageTemplate()
{
	XOJ_CHECK_TYPE(Settings);

	return this->pageTemplate;
}

void Settings::setPageTemplate(string pageTemplate)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->pageTemplate == pageTemplate)
	{
		return;
	}

	this->pageTemplate = pageTemplate;

	save();
}

string Settings::getSizeUnit()
{
	XOJ_CHECK_TYPE(Settings);

	return sizeUnit;
}

void Settings::setSizeUnit(string sizeUnit)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->sizeUnit == sizeUnit)
	{
		return;
	}

	this->sizeUnit = sizeUnit;

	save();
}

/**
 * Get size index in XOJ_UNITS
 */
int Settings::getSizeUnitIndex()
{
	XOJ_CHECK_TYPE(Settings);

	string unit = getSizeUnit();

	for (int i = 0; i < XOJ_UNIT_COUNT; i++)
	{
		if (unit == XOJ_UNITS[i].name)
		{
			return i;
		}
	}

	return 0;
}

/**
 * Set size index in XOJ_UNITS
 */
void Settings::setSizeUnitIndex(int sizeUnitId)
{
	XOJ_CHECK_TYPE(Settings);

	if (sizeUnitId < 0 || sizeUnitId >= XOJ_UNIT_COUNT)
	{
		sizeUnitId = 0;
	}

	setSizeUnit(XOJ_UNITS[sizeUnitId].name);
}

void Settings::setShowTwoPages(bool showTwoPages)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->showTwoPages == showTwoPages)
	{
		return;
	}

	this->showTwoPages = showTwoPages;
	save();
}

bool Settings::isShowTwoPages()
{
	XOJ_CHECK_TYPE(Settings);

	return this->showTwoPages;
}

void Settings::setPresentationMode(bool presentationMode)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->presentationMode == presentationMode)
	{
		return;
	}

	this->presentationMode = presentationMode;
	save();
}

bool Settings::isPresentationMode()
{
	XOJ_CHECK_TYPE(Settings);

	return this->presentationMode;
}

void Settings::setPresureSensitivity(gboolean presureSensitivity)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->presureSensitivity == presureSensitivity)
	{
		return;
	}
	this->presureSensitivity = presureSensitivity;

	save();
}

void Settings::setLastSavePath(path p)
{
	XOJ_CHECK_TYPE(Settings);

	this->lastSavePath = p;
	save();
}

path Settings::getLastSavePath()
{
	XOJ_CHECK_TYPE(Settings);

	return this->lastSavePath;
}

void Settings::setLastImagePath(path path)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->lastImagePath == path)
	{
		return;
	}
	this->lastImagePath = path;
	save();
}

path Settings::getLastImagePath()
{
	XOJ_CHECK_TYPE(Settings);

	return this->lastImagePath;
}

void Settings::setDisplayDpi(int dpi)
{
	XOJ_CHECK_TYPE(Settings);

	this->displayDpi = dpi;

	if (this->displayDpi == dpi)
	{
		return;
	}
	this->displayDpi = dpi;
	save();
}

int Settings::getDisplayDpi()
{
	return this->displayDpi;
}

bool Settings::isSidebarVisible()
{
	XOJ_CHECK_TYPE(Settings);

	return this->showSidebar;
}

void Settings::setSidebarVisible(bool visible)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->showSidebar == visible)
	{
		return;
	}
	this->showSidebar = visible;
	save();
}

int Settings::getSidebarWidth()
{
	XOJ_CHECK_TYPE(Settings);

	if (this->sidebarWidth < 50)
	{
		setSidebarWidth(150);
	}
	return this->sidebarWidth;
}

void Settings::setSidebarWidth(int width)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->sidebarWidth == width)
	{
		return;
	}
	this->sidebarWidth = width;
	save();
}

void Settings::setMainWndSize(int width, int height)
{
	XOJ_CHECK_TYPE(Settings);

	this->mainWndWidth = width;
	this->mainWndHeight = height;

	save();
}

int Settings::getMainWndWidth()
{
	XOJ_CHECK_TYPE(Settings);

	return this->mainWndWidth;
}

int Settings::getMainWndHeight()
{
	XOJ_CHECK_TYPE(Settings);

	return this->mainWndHeight;
}

bool Settings::isMainWndMaximized()
{
	XOJ_CHECK_TYPE(Settings);

	return this->maximized;
}

void Settings::setMainWndMaximized(bool max)
{
	XOJ_CHECK_TYPE(Settings);

	this->maximized = max;
}

double Settings::getWidthMinimumMultiplier()
{
	XOJ_CHECK_TYPE(Settings);

	return this->widthMinimumMultiplier;
}

double Settings::getWidthMaximumMultiplier()
{
	XOJ_CHECK_TYPE(Settings);

	return this->widthMaximumMultiplier;
}

void Settings::setSelectedToolbar(string name)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->selectedToolbar == name)
	{
		return;
	}
	this->selectedToolbar = name;
	save();
}

string Settings::getSelectedToolbar()
{
	XOJ_CHECK_TYPE(Settings);

	return this->selectedToolbar;
}

SElement& Settings::getCustomElement(string name)
{
	XOJ_CHECK_TYPE(Settings);

	return this->data[name];
}

void Settings::customSettingsChanged()
{
	XOJ_CHECK_TYPE(Settings);

	save();
}

ButtonConfig* Settings::getButtonConfig(int id)
{
	XOJ_CHECK_TYPE(Settings);

	if (id < 0 || id >= BUTTON_COUNT)
	{
		g_error("Settings::getButtonConfig try to get id=%i out of range!", id);
		return NULL;
	}
	return this->buttonConfig[id];
}

ButtonConfig* Settings::getEraserButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[0];
}

ButtonConfig* Settings::getMiddleButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[1];
}

ButtonConfig* Settings::getRightButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[2];
}

ButtonConfig* Settings::getTouchButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[3];
}

ButtonConfig* Settings::getDefaultButtonConfig()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[4];
}

ButtonConfig* Settings::getStylusButton1Config()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[5];
}

ButtonConfig* Settings::getStylusButton2Config()
{
	XOJ_CHECK_TYPE(Settings);

	return this->buttonConfig[6];
}

string Settings::getFullscreenHideElements()
{
	XOJ_CHECK_TYPE(Settings);

	return this->fullscreenHideElements;
}

void Settings::setFullscreenHideElements(string elements)
{
	this->fullscreenHideElements = elements;
	save();
}

string Settings::getPresentationHideElements()
{
	XOJ_CHECK_TYPE(Settings);

	return this->presentationHideElements;
}

void Settings::setPresentationHideElements(string elements)
{
	XOJ_CHECK_TYPE(Settings);

	this->presentationHideElements = elements;
	save();
}

int Settings::getPdfPageCacheSize()
{
	XOJ_CHECK_TYPE(Settings);

	return this->pdfPageCacheSize;
}

void Settings::setPdfPageCacheSize(int size)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->pdfPageCacheSize == size)
	{
		return;
	}
	this->pdfPageCacheSize = size;
	save();
}

int Settings::getSelectionColor()
{
	XOJ_CHECK_TYPE(Settings);

	return this->selectionColor;
}

void Settings::setSelectionColor(int color)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->selectionColor == color)
	{
		return;
	}
	this->selectionColor = color;
	save();
}

int Settings::getBackgroundColor()
{
	XOJ_CHECK_TYPE(Settings);

	return this->backgroundColor;
}

void Settings::setBackgroundColor(int color)
{
	XOJ_CHECK_TYPE(Settings);

	if (this->backgroundColor == color)
	{
		return;
	}
	this->backgroundColor = color;
	save();
}

XojFont& Settings::getFont()
{
	XOJ_CHECK_TYPE(Settings);

	return this->font;
}

void Settings::setFont(const XojFont& font)
{
	XOJ_CHECK_TYPE(Settings);

	this->font = font;
	save();
}

//////////////////////////////////////////////////

SAttribute::SAttribute()
{
	XOJ_INIT_TYPE(SAttribute);

	this->dValue = 0;
	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;
}

SAttribute::SAttribute(const SAttribute& attrib)
{
	XOJ_INIT_TYPE(SAttribute);

	*this = attrib;
}

SAttribute::~SAttribute()
{
	XOJ_CHECK_TYPE(SAttribute);

	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;

	XOJ_RELEASE_TYPE(SAttribute);
}

//////////////////////////////////////////////////

__RefSElement::__RefSElement()
{
	XOJ_INIT_TYPE(__RefSElement);

	this->refcount = 0;
}

__RefSElement::~__RefSElement()
{
	XOJ_RELEASE_TYPE(__RefSElement);
}

void __RefSElement::ref()
{
	XOJ_CHECK_TYPE(__RefSElement);

	this->refcount++;
}

void __RefSElement::unref()
{
	XOJ_CHECK_TYPE(__RefSElement);

	this->refcount--;
	if (this->refcount == 0)
	{
		delete this;
	}
}

SElement::SElement()
{
	XOJ_INIT_TYPE(SElement);

	this->element = new __RefSElement();
	this->element->ref();
}

SElement::SElement(const SElement& elem)
{
	XOJ_INIT_TYPE(SElement);

	this->element = elem.element;
	this->element->ref();
}

SElement::~SElement()
{
	XOJ_CHECK_TYPE(SElement);

	this->element->unref();
	this->element = NULL;

	XOJ_RELEASE_TYPE(SElement);
}

void SElement::operator=(const SElement& elem)
{
	XOJ_CHECK_TYPE(SElement);

	this->element = elem.element;
	this->element->ref();
}

std::map<string, SAttribute>& SElement::attributes()
{
	XOJ_CHECK_TYPE(SElement);

	return this->element->attributes;
}

std::map<string, SElement>& SElement::children()
{
	XOJ_CHECK_TYPE(SElement);

	return this->element->children;
}

void SElement::clear()
{
	XOJ_CHECK_TYPE(SElement);

	this->element->attributes.clear();
	this->element->children.clear();
}

SElement& SElement::child(string name)
{
	XOJ_CHECK_TYPE(SElement);

	return this->element->children[name];
}

void SElement::setComment(const string name, const string comment)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.comment = comment;
}

void SElement::setIntHex(const string name, const int value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setInt(const string name, const int value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setBool(const string name, const bool value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
}

void SElement::setString(const string name, const string value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.sValue = value;
	attrib.type = ATTRIBUTE_TYPE_STRING;
}

void SElement::setDouble(const string name, const double value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	attrib.dValue = value;
	attrib.type = ATTRIBUTE_TYPE_DOUBLE;
}

bool SElement::getDouble(const string name, double& value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE)
	{
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_DOUBLE)
	{
		return false;
	}

	value = attrib.dValue;

	return true;
}

bool SElement::getInt(const string name, int& value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE)
	{
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_INT && attrib.type != ATTRIBUTE_TYPE_INT_HEX)
	{
		return false;
	}

	value = attrib.iValue;

	return true;
}

bool SElement::getBool(const string name, bool& value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE)
	{
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_BOOLEAN)
	{
		return false;
	}

	value = attrib.iValue;

	return true;
}

bool SElement::getString(const string name, string& value)
{
	XOJ_CHECK_TYPE(SElement);

	SAttribute& attrib = this->element->attributes[name];

	XOJ_CHECK_TYPE_OBJ(&attrib, SAttribute);

	if (attrib.type == ATTRIBUTE_TYPE_NONE)
	{
		this->element->attributes.erase(name);
		return false;
	}

	if (attrib.type != ATTRIBUTE_TYPE_STRING)
	{
		return false;
	}

	value = attrib.sValue;

	return true;

}

