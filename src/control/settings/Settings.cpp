#include "Settings.h"

#include "ButtonConfig.h"
#include "model/FormatDefinitions.h"

#include <config.h>
#include <i18n.h>
#include <Util.h>
#include <util/DeviceListHelper.h>
#include <utility>
#define DEFAULT_FONT "Sans"
#define DEFAULT_FONT_SIZE 12

#define WRITE_BOOL_PROP(var) xmlNode = saveProperty((const char *)#var, var ? "true" : "false", root)
#define WRITE_STRING_PROP(var) xmlNode = saveProperty((const char *)#var, var.empty() ? "" : var.c_str(), root)
#define WRITE_INT_PROP(var) xmlNode = saveProperty((const char *)#var, var, root)
#define WRITE_DOUBLE_PROP(var) xmlNode = savePropertyDouble((const char *)#var, var, root)
#define WRITE_COMMENT(var) com = xmlNewComment((const xmlChar *)var); xmlAddPrevSibling(xmlNode, com);

const char* BUTTON_NAMES[] = {"middle", "right", "eraser", "touch", "default", "stylus", "stylus2"};

Settings::Settings(Path filename)
 : filename(std::move(filename))
{
	loadDefault();
}

Settings::~Settings()
{
	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		delete this->buttonConfig[i];
		this->buttonConfig[i] = nullptr;
	}
}

void Settings::loadDefault()
{
	this->pressureSensitivity = true;
	this->zoomGesturesEnabled = true;
	this->maximized = false;
	this->showPairedPages = false;
	this->presentationMode = false;

	this->numColumns = 1;	// only one of these applies at a time
	this->numRows = 1;
	this->viewFixedRows = false;	
	
	this->layoutVertical = false;
	this->layoutRightToLeft = false;
	this->layoutBottomToTop = false;
	
	this->numPairsOffset = 1;
	
	this->zoomStep = 10.0;
	this->zoomStepScroll = 2.0;

	this->displayDpi = 72;

	this->font.setName(DEFAULT_FONT);
	this->font.setSize(DEFAULT_FONT_SIZE);

	this->mainWndWidth = 800;
	this->mainWndHeight = 600;

	this->showSidebar = true;
	this->sidebarWidth = 150;

	this->sidebarOnRight = false;

	this->scrollbarOnLeft = false;

	this->menubarVisible = true;

	this->autoloadPdfXoj = true;
	this->showBigCursor = false;
	this->highlightPosition = false;
	this->darkTheme = false;
	this->scrollbarHideType = SCROLLBAR_HIDE_NONE;
	this->disableScrollbarFadeout = false;

	//Set this for autosave frequency in minutes.
	this->autosaveTimeout = 3;
	this->autosaveEnabled = true;

	this->addHorizontalSpace = false;
	this->addHorizontalSpaceAmount = 150;
	this->addVerticalSpace = false;
	this->addVerticalSpaceAmount = 150;

	//Drawing direction emulates modifier keys
	this->drawDirModsRadius = 50;
	this->drawDirModsEnabled = false;
	
	this->snapRotation = true;
	this->snapRotationTolerance = 0.20;

	this->snapGrid = true;
	this->snapGridTolerance = 0.25;

	this->touchWorkaround = false;

	this->defaultSaveName = _("%F-Note-%H-%M");

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

	this->selectionBorderColor = 0xff0000; // red
	this->selectionMarkerColor = 0x729FCF; // light blue

	this->backgroundColor = 0xDCDAD5;

	// clang-format off
	this->pageTemplate = "xoj/template\ncopyLastPageSettings=true\nsize=595.275591x841.889764\nbackgroundType=lined\nbackgroundColor=#ffffff\n";
	// clang-format on

	this->audioSampleRate = 44100.0;
	this->audioInputDevice = -1;
	this->audioOutputDevice = -1;
	this->audioGain = 1.0;
	this->defaultSeekTime = 5;

	this->pluginEnabled = "";
	this->pluginDisabled = "";

	this->experimentalInputSystemEnabled = false;
	this->inputSystemTPCButton = false;
	this->inputSystemDrawOutsideWindow = true;

	this->strokeFilterIgnoreTime = 150;
	this->strokeFilterIgnoreLength = 1;
	this->strokeFilterSuccessiveTime = 500;
	this->strokeFilterEnabled = false;
	this->doActionOnStrokeFiltered = false;
	this->trySelectOnStrokeFiltered = false;

	this->inTransaction = false;

}

/**
 * tempg_ascii_strtod
* 	Transition to using g_ascii_strtod to minimize disruption. May, 2019. 
*  Delete this and replace calls to this function with calls to g_ascii_strtod() in 2020.
* 	See: https://developer.gnome.org/glib/stable/glib-String-Utility-Functions.html#g-strtod
*/
double tempg_ascii_strtod( const gchar* txt, gchar ** endptr )
{
	return g_strtod ( txt, endptr  );		//  makes best guess between locale formatted and C formatted numbers. See link above.
}


void Settings::parseData(xmlNodePtr cur, SElement& elem)
{
	for (xmlNodePtr x = cur->children; x != nullptr; x = x->next)
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
				double d = tempg_ascii_strtod((const char*) value, nullptr);	//g_ascii_strtod ignores locale setting.
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
	// Parse data map
	if (!xmlStrcmp(cur->name, (const xmlChar*) "data"))
	{
		xmlChar* name = xmlGetProp(cur, (const xmlChar*) "name");
		if (name == nullptr)
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
	if (name == nullptr)
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
	if (value == nullptr)
	{
		xmlFree(name);
		g_warning("No value property!\n");
		return;
	}

	// TODO: remove this typo fix in 2-3 release cycles
	if (xmlStrcmp(name, (const xmlChar*) "presureSensitivity") == 0)
	{
		setPressureSensitivity(xmlStrcmp(value, (const xmlChar*) "true") ? false : true);
	}
	if (xmlStrcmp(name, (const xmlChar*) "pressureSensitivity") == 0)
	{
		setPressureSensitivity(xmlStrcmp(value, (const xmlChar*) "true") ? false : true);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "zoomGesturesEnabled") == 0)
	{
		this->zoomGesturesEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "selectedToolbar") == 0)
	{
		this->selectedToolbar = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "lastSavePath") == 0)
	{
		this->lastSavePath = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "lastOpenPath") == 0)
	{
		this->lastOpenPath = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "lastImagePath") == 0)
	{
		this->lastImagePath = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "zoomStep") == 0)
	{
		this->zoomStep = tempg_ascii_strtod((const char*) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "zoomStepScroll") == 0)
	{
		this->zoomStepScroll = tempg_ascii_strtod((const char*) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "displayDpi") == 0)
	{
		this->displayDpi = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "mainWndWidth") == 0)
	{
		this->mainWndWidth = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "mainWndHeight") == 0)
	{
		this->mainWndHeight = g_ascii_strtoll((const char*) value, nullptr, 10);
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
		this->sidebarWidth = std::max<int>(g_ascii_strtoll((const char*) value, nullptr, 10), 50);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "sidebarOnRight") == 0)
	{
		this->sidebarOnRight = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "scrollbarOnLeft") == 0)
	{
		this->scrollbarOnLeft = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "menubarVisible") == 0)
	{
		this->menubarVisible = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "numColumns") == 0)
	{
		this->numColumns = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "numRows") == 0)
	{
		this->numRows = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "viewFixedRows") == 0)
	{
		this->viewFixedRows = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "layoutVertical") == 0)
	{
		this->layoutVertical = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "layoutRightToLeft") == 0)
	{
		this->layoutRightToLeft = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "layoutBottomToTop") == 0)
	{
		this->layoutBottomToTop = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "showPairedPages") == 0)
	{
		this->showPairedPages = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "numPairsOffset") == 0)
	{
		this->numPairsOffset = g_ascii_strtoll((const char*) value, nullptr, 10);
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
	else if (xmlStrcmp(name, (const xmlChar*) "highlightPosition") == 0)
	{
		this->highlightPosition = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "darkTheme") == 0)
	{
		this->darkTheme = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "defaultSaveName") == 0)
	{
		this->defaultSaveName = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "pluginEnabled") == 0)
	{
		this->pluginEnabled = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "pluginDisabled") == 0)
	{
		this->pluginDisabled = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "pageTemplate") == 0)
	{
		this->pageTemplate = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "sizeUnit") == 0)
	{
		this->sizeUnit = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "audioFolder") == 0)
	{
		this->audioFolder = (const char*) value;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "autosaveEnabled") == 0)
	{
		this->autosaveEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "autosaveTimeout") == 0)
	{
		this->autosaveTimeout = g_ascii_strtoll((const char*) value, nullptr, 10);
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
		this->pdfPageCacheSize = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "selectionBorderColor") == 0)
	{
		this->selectionBorderColor = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "selectionMarkerColor") == 0)
	{
		this->selectionMarkerColor = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "backgroundColor") == 0)
	{
		this->backgroundColor = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addHorizontalSpace") == 0)
	{
		this->addHorizontalSpace = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addHorizontalSpaceAmount") == 0)
	{
		this->addHorizontalSpaceAmount = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addVerticalSpace") == 0)
	{
		this->addVerticalSpace = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "addVerticalSpaceAmount") == 0)
	{
		this->addVerticalSpaceAmount = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "drawDirModsEnabled") == 0)
	{
		this->drawDirModsEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "drawDirModsRadius") == 0)
	{
		this->drawDirModsRadius = g_ascii_strtoll((const char*) value, nullptr, 10);
	}	
	else if (xmlStrcmp(name, (const xmlChar*) "snapRotation") == 0)
	{
		this->snapRotation = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "snapRotationTolerance") == 0)
	{
		this->snapRotationTolerance = tempg_ascii_strtod((const char*) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "snapGrid") == 0)
	{
		this->snapGrid = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "snapGrid") == 0)
	{
		this->snapGrid = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "snapGridTolerance") == 0)
	{
		this->snapGridTolerance = tempg_ascii_strtod((const char*) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "touchWorkaround") == 0)
	{
		this->touchWorkaround = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
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
	else if (xmlStrcmp(name, (const xmlChar*) "disableScrollbarFadeout") == 0)
	{
		this->disableScrollbarFadeout = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "audioSampleRate") == 0)
	{
		this->audioSampleRate = tempg_ascii_strtod((const char *) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "audioGain") == 0)
	{
		this->audioGain = tempg_ascii_strtod((const char *) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "defaultSeekTime") == 0)
	{
		this->defaultSeekTime = tempg_ascii_strtod((const char *) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "audioInputDevice") == 0)
	{
		this->audioInputDevice = g_ascii_strtoll((const char *) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "audioOutputDevice") == 0)
	{
		this->audioOutputDevice = g_ascii_strtoll((const char *) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "experimentalInputSystemEnabled") == 0)
	{
		this->experimentalInputSystemEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "inputSystemTPCButton") == 0)
	{
		this->inputSystemTPCButton = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "inputSystemDrawOutsideWindow") == 0)
	{
		this->inputSystemDrawOutsideWindow = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "strokeFilterIgnoreTime") == 0)
	{
		this->strokeFilterIgnoreTime = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "strokeFilterIgnoreLength") == 0)
	{
		this->strokeFilterIgnoreLength = tempg_ascii_strtod((const char*) value, nullptr);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "strokeFilterSuccessiveTime") == 0)
	{
		this->strokeFilterSuccessiveTime = g_ascii_strtoll((const char*) value, nullptr, 10);
	}
	else if (xmlStrcmp(name, (const xmlChar*) "strokeFilterEnabled") == 0)
	{
		this->strokeFilterEnabled = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "doActionOnStrokeFiltered") == 0)
	{
		this->doActionOnStrokeFiltered = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}
	else if (xmlStrcmp(name, (const xmlChar*) "trySelectOnStrokeFiltered") == 0)
	{
		this->trySelectOnStrokeFiltered = xmlStrcmp(value, (const xmlChar*) "true") ? false : true;
	}

	xmlFree(name);
	xmlFree(value);
}

void Settings::loadDeviceClasses()
{
	SElement& s = getCustomElement("deviceClasses");
	for (auto device : s.children())
	{
		SElement& deviceNode = device.second;
		int deviceClass;
		int deviceSource;
		deviceNode.getInt("deviceClass", deviceClass);
		deviceNode.getInt("deviceSource", deviceSource);
		inputDeviceClasses.insert(std::pair<string, std::pair<int, GdkInputSource>>(
		        device.first, std::pair<int, GdkInputSource>(deviceClass, (GdkInputSource) deviceSource)));
	}
}

void Settings::loadButtonConfig()
{
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
	xmlKeepBlanksDefault(0);

	if (!filename.exists())
	{
		g_warning("configfile does not exist %s\n", filename.c_str());
		return false;
	}

	xmlDocPtr doc = xmlParseFile(filename.c_str());

	if (doc == nullptr)
	{
		g_warning("Settings::load:: doc == null, could not load Settings!\n");
		return false;
	}

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (cur == nullptr)
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

	while (cur != nullptr)
	{
		parseItem(doc, cur);

		cur = cur->next;
	}

	xmlFreeDoc(doc);

	loadButtonConfig();
	loadDeviceClasses();

	return true;
}

xmlNodePtr Settings::savePropertyDouble(const gchar* key, double value, xmlNodePtr parent)
{
	char text[G_ASCII_DTOSTR_BUF_SIZE];
	//  g_ascii_ version uses C locale always.
	g_ascii_formatd(text, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar* key, int value, xmlNodePtr parent)
{
	char* text = g_strdup_printf("%i", value);
	xmlNodePtr xmlNode = saveProperty(key, text, parent);
	g_free(text);
	return xmlNode;
}

xmlNodePtr Settings::saveProperty(const gchar* key, const gchar* value, xmlNodePtr parent)
{
	xmlNodePtr xmlNode = xmlNewChild(parent, nullptr, (const xmlChar*) "property", nullptr);

	xmlSetProp(xmlNode, (const xmlChar*) "name", (const xmlChar*) key);

	xmlSetProp(xmlNode, (const xmlChar*) "value", (const xmlChar*) value);

	return xmlNode;
}

void Settings::saveDeviceClasses()
{
	SElement& s = getCustomElement("deviceClasses");

	for (const std::map<string, std::pair<int, GdkInputSource>>::value_type& device: inputDeviceClasses)
	{
		SElement& e = s.child(device.first);
		e.setInt("deviceClass", device.second.first);
		e.setInt("deviceSource", device.second.second);
	}
}

void Settings::saveButtonConfig()
{
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
			e.setString("size", toolSizeToString(cfg->size));
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
	inTransaction = true;
}

/**
 * Stop transaction and save settings
 */
void Settings::transactionEnd()
{
	inTransaction = false;
	save();
}

void Settings::save()
{
	if (inTransaction)
	{
		return;
	}

	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr xmlNode;

	xmlIndentTreeOutput = true;

	doc = xmlNewDoc((const xmlChar*) "1.0");
	if (doc == nullptr)
	{
		return;
	}

	saveButtonConfig();
	saveDeviceClasses();

	/* Create metadata root */
	root = xmlNewDocNode(doc, nullptr, (const xmlChar*) "settings", nullptr);
	xmlDocSetRootElement(doc, root);
	xmlNodePtr com = xmlNewComment((const xmlChar*)
								   "The Xournal++ settings file. Do not edit this file! "
								   "The most settings are available in the Settings dialog, "
								   "the others are commented in this file, but handle with care!");
	xmlAddPrevSibling(root, com);

	WRITE_BOOL_PROP(pressureSensitivity);
	WRITE_BOOL_PROP(zoomGesturesEnabled);

	WRITE_STRING_PROP(selectedToolbar);

	auto lastSavePath = this->lastSavePath.str();
	auto lastOpenPath = this->lastOpenPath.str();
	auto lastImagePath = this->lastImagePath.str();
	WRITE_STRING_PROP(lastSavePath);
	WRITE_STRING_PROP(lastOpenPath);
	WRITE_STRING_PROP(lastImagePath);

	WRITE_DOUBLE_PROP(zoomStep);
	WRITE_DOUBLE_PROP(zoomStepScroll);
	WRITE_INT_PROP(displayDpi);
	WRITE_INT_PROP(mainWndWidth);
	WRITE_INT_PROP(mainWndHeight);
	WRITE_BOOL_PROP(maximized);

	WRITE_BOOL_PROP(showSidebar);
	WRITE_INT_PROP(sidebarWidth);

	WRITE_BOOL_PROP(sidebarOnRight);
	WRITE_BOOL_PROP(scrollbarOnLeft);
	WRITE_BOOL_PROP(menubarVisible);
	WRITE_INT_PROP(numColumns);
	WRITE_INT_PROP(numRows);
	WRITE_BOOL_PROP(viewFixedRows);
	WRITE_BOOL_PROP(showPairedPages);
	WRITE_BOOL_PROP(layoutVertical);
	WRITE_BOOL_PROP(layoutRightToLeft);
	WRITE_BOOL_PROP(layoutBottomToTop);
	WRITE_INT_PROP(numPairsOffset);
	WRITE_BOOL_PROP(presentationMode);

	WRITE_STRING_PROP(fullscreenHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)");

	WRITE_STRING_PROP(presentationHideElements);
	WRITE_COMMENT("Which gui elements are hidden if you are in Presentation mode, separated by a colon (,)");

	WRITE_BOOL_PROP(showBigCursor);
	WRITE_BOOL_PROP(highlightPosition);
	WRITE_BOOL_PROP(darkTheme);

	WRITE_BOOL_PROP(disableScrollbarFadeout);

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
	WRITE_INT_PROP(addHorizontalSpaceAmount);	
	WRITE_BOOL_PROP(addVerticalSpace);
	WRITE_INT_PROP(addVerticalSpaceAmount);
	
	WRITE_BOOL_PROP(drawDirModsEnabled);
	WRITE_INT_PROP(drawDirModsRadius);
	

	WRITE_BOOL_PROP(snapRotation);
	WRITE_DOUBLE_PROP(snapRotationTolerance);
	WRITE_BOOL_PROP(snapGrid);
	WRITE_DOUBLE_PROP(snapGridTolerance);

	WRITE_BOOL_PROP(touchWorkaround);

	WRITE_INT_PROP(selectionBorderColor);
	WRITE_INT_PROP(backgroundColor);
	WRITE_INT_PROP(selectionMarkerColor);

	WRITE_INT_PROP(pdfPageCacheSize);
	WRITE_COMMENT("The count of rendered PDF pages which will be cached.");

	WRITE_COMMENT("Config for new pages");
	WRITE_STRING_PROP(pageTemplate);

	WRITE_STRING_PROP(sizeUnit);

	WRITE_STRING_PROP(audioFolder);
	WRITE_INT_PROP(audioInputDevice);
	WRITE_INT_PROP(audioOutputDevice);
	WRITE_DOUBLE_PROP(audioSampleRate);
	WRITE_DOUBLE_PROP(audioGain);
	WRITE_INT_PROP(defaultSeekTime);

	WRITE_STRING_PROP(pluginEnabled);
	WRITE_STRING_PROP(pluginDisabled);

	WRITE_INT_PROP(strokeFilterIgnoreTime);
	WRITE_DOUBLE_PROP(strokeFilterIgnoreLength);
	WRITE_INT_PROP(strokeFilterSuccessiveTime);

	WRITE_BOOL_PROP(strokeFilterEnabled);
	WRITE_BOOL_PROP(doActionOnStrokeFiltered);
	WRITE_BOOL_PROP(trySelectOnStrokeFiltered);

	WRITE_BOOL_PROP(experimentalInputSystemEnabled);
	WRITE_BOOL_PROP(inputSystemTPCButton);
	WRITE_BOOL_PROP(inputSystemDrawOutsideWindow);

	xmlNodePtr xmlFont;
	xmlFont = xmlNewChild(root, nullptr, (const xmlChar*) "property", nullptr);
	xmlSetProp(xmlFont, (const xmlChar*) "name", (const xmlChar*) "font");
	xmlSetProp(xmlFont, (const xmlChar*) "font", (const xmlChar*) this->font.getName().c_str());

	char sSize[G_ASCII_DTOSTR_BUF_SIZE];

	g_ascii_formatd(sSize, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, this->font.getSize());  // no locale
	xmlSetProp(xmlFont, (const xmlChar*) "size", (const xmlChar*) sSize);


	for (std::map<string, SElement>::value_type p: data)
	{
		saveData(root, p.first, p.second);
	}

	xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);
	xmlFreeDoc(doc);
}

void Settings::saveData(xmlNodePtr root, string name, SElement& elem)
{
	xmlNodePtr xmlNode = xmlNewChild(root, nullptr, (const xmlChar*) "data", nullptr);

	xmlSetProp(xmlNode, (const xmlChar*) "name", (const xmlChar*) name.c_str());

	for (std::map<string, SAttribute>::value_type p : elem.attributes())
	{
		string aname = p.first;
		SAttribute& attrib = p.second;
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

			char tmp[G_ASCII_DTOSTR_BUF_SIZE];
			g_ascii_formatd(tmp, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, attrib.dValue);
			value = tmp;
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
		at = xmlNewChild(xmlNode, nullptr, (const xmlChar*) "attribute", nullptr);

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
bool Settings::isPressureSensitivity() const
{
	return this->pressureSensitivity;
}

bool Settings::isZoomGesturesEnabled() const
{
	return this->zoomGesturesEnabled;
}

void Settings::setZoomGesturesEnabled(bool enable)
{
	if (this->zoomGesturesEnabled == enable)
	{
		return;
	}
	this->zoomGesturesEnabled = enable;
	save();
}

bool Settings::isSidebarOnRight() const
{
	return this->sidebarOnRight;
}

void Settings::setSidebarOnRight(bool right)
{
	if (this->sidebarOnRight == right)
	{
		return;
	}

	this->sidebarOnRight = right;

	save();
}

bool Settings::isScrollbarOnLeft() const
{
	return this->scrollbarOnLeft;
}

void Settings::setScrollbarOnLeft(bool right)
{
	if (this->scrollbarOnLeft == right)
	{
		return;
	}

	this->scrollbarOnLeft = right;

	save();
}

bool Settings::isMenubarVisible() const
{
	return this->menubarVisible;
}

void Settings::setMenubarVisible(bool visible)
{
	this->menubarVisible = visible;

	save();
}

int Settings::getAutosaveTimeout() const
{
	return this->autosaveTimeout;
}

void Settings::setAutosaveTimeout(int autosave)
{
	if (this->autosaveTimeout == autosave)
	{
		return;
	}

	this->autosaveTimeout = autosave;

	save();
}

bool Settings::isAutosaveEnabled() const
{
	return this->autosaveEnabled;
}

void Settings::setAutosaveEnabled(bool autosave)
{
	if (this->autosaveEnabled == autosave)
	{
		return;
	}

	this->autosaveEnabled = autosave;

	save();
}

bool Settings::getAddVerticalSpace() const
{
	return this->addVerticalSpace;
}

void Settings::setAddVerticalSpace(bool space)
{
	this->addVerticalSpace = space;
}

int Settings::getAddVerticalSpaceAmount() const
{
	return this->addVerticalSpaceAmount;
}

void Settings::setAddVerticalSpaceAmount(int pixels)
{
	if (this->addVerticalSpaceAmount == pixels)
	{
		return;
	}

	this->addVerticalSpaceAmount = pixels;
	save();
}


bool Settings::getAddHorizontalSpace() const
{
	return this->addHorizontalSpace;
}

void Settings::setAddHorizontalSpace(bool space)
{
	this->addHorizontalSpace = space;
}

int Settings::getAddHorizontalSpaceAmount() const
{
	return this->addHorizontalSpaceAmount;
}

void Settings::setAddHorizontalSpaceAmount(int pixels)
{
	if (this->addHorizontalSpaceAmount == pixels)
	{
		return;
	}

	this->addHorizontalSpaceAmount = pixels;
	save();
}


bool Settings::getDrawDirModsEnabled() const
{
	return this->drawDirModsEnabled;
}

void Settings::setDrawDirModsEnabled(bool enable)
{
	this->drawDirModsEnabled = enable;
}

int Settings::getDrawDirModsRadius() const
{
	return this->drawDirModsRadius;
}

void Settings::setDrawDirModsRadius(int pixels)
{
	if (this->drawDirModsRadius == pixels)
	{
		return;
	}

	this->drawDirModsRadius = pixels;
	save();
}


bool Settings::isShowBigCursor() const
{
	return this->showBigCursor;
}

void Settings::setShowBigCursor(bool b)
{
	if (this->showBigCursor == b)
	{
		return;
	}

	this->showBigCursor = b;
	save();
}

bool Settings::isHighlightPosition() const
{
	return this->highlightPosition;

}

void Settings::setHighlightPosition(bool highlight)
{
	if (this->highlightPosition == highlight)
	{
		return;
	}

	this->highlightPosition = highlight;
	save();
}

bool Settings::isSnapRotation() const
{
	return this->snapRotation;
}

void Settings::setSnapRotation(bool b)
{
	if (this->snapRotation == b)
	{
		return;
	}

	this->snapRotation = b;
	save();
}

double Settings::getSnapRotationTolerance() const
{
	return this->snapRotationTolerance;
}

void Settings::setSnapRotationTolerance(double tolerance)
{
	this->snapRotationTolerance = tolerance;
	save();
}

bool Settings::isSnapGrid() const
{
	return this->snapGrid;
}

void Settings::setSnapGrid(bool b)
{
	if (this->snapGrid == b)
	{
		return;
	}

	this->snapGrid = b;
	save();
}

void Settings::setSnapGridTolerance(double tolerance)
{
	this->snapGridTolerance = tolerance;
	save();
}

double Settings::getSnapGridTolerance() const
{
	return this->snapGridTolerance;
}

bool Settings::isTouchWorkaround() const
{
	return this->touchWorkaround;
}

void Settings::setTouchWorkaround(bool b)
{
	if (this->touchWorkaround == b)
	{
		return;
	}

	this->touchWorkaround = b;
	save();
}

ScrollbarHideType Settings::getScrollbarHideType() const
{
	return this->scrollbarHideType;
}

void Settings::setScrollbarHideType(ScrollbarHideType type)
{
	if (this->scrollbarHideType == type)
	{
		return;
	}

	this->scrollbarHideType = type;

	save();
}

bool Settings::isAutloadPdfXoj() const
{
	return this->autoloadPdfXoj;
}

void Settings::setAutoloadPdfXoj(bool load)
{
	if (this->autoloadPdfXoj == load)
	{
		return;
	}
	this->autoloadPdfXoj = load;
	save();
}

string const& Settings::getDefaultSaveName() const
{
	return this->defaultSaveName;
}

void Settings::setDefaultSaveName(string name)
{
	if (this->defaultSaveName == name)
	{
		return;
	}

	this->defaultSaveName = name;

	save();
}

string const& Settings::getPageTemplate() const
{
	return this->pageTemplate;
}

void Settings::setPageTemplate(string pageTemplate)
{
	if (this->pageTemplate == pageTemplate)
	{
		return;
	}

	this->pageTemplate = pageTemplate;

	save();
}

string const& Settings::getAudioFolder() const
{
	return this->audioFolder;
}

void Settings::setAudioFolder(string audioFolder)
{
	if (this->audioFolder == audioFolder)
	{
		return;
	}

	this->audioFolder = audioFolder;

	save();
}

string const& Settings::getSizeUnit() const
{
	return sizeUnit;
}

void Settings::setSizeUnit(string sizeUnit)
{
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
int Settings::getSizeUnitIndex() const
{
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
	if (sizeUnitId < 0 || sizeUnitId >= XOJ_UNIT_COUNT)
	{
		sizeUnitId = 0;
	}

	setSizeUnit(XOJ_UNITS[sizeUnitId].name);
}

void Settings::setShowPairedPages(bool showPairedPages)
{
	if (this->showPairedPages == showPairedPages)
	{
		return;
	}

	this->showPairedPages = showPairedPages;
	save();
}

bool Settings::isShowPairedPages() const
{
	return this->showPairedPages;
}

void Settings::setPresentationMode(bool presentationMode)
{
	if (this->presentationMode == presentationMode)
	{
		return;
	}

	this->presentationMode = presentationMode;
	save();
}

bool Settings::isPresentationMode() const
{
	return this->presentationMode;
}

void Settings::setPressureSensitivity(gboolean presureSensitivity)
{
	if (this->pressureSensitivity == presureSensitivity)
	{
		return;
	}
	this->pressureSensitivity = presureSensitivity;

	save();
}

void Settings::setPairsOffset(int numOffset)
{
	if (this->numPairsOffset == numOffset)
	{
		return;
	}

	this->numPairsOffset = numOffset;
	save();
}

int Settings::getPairsOffset() const
{
	return this->numPairsOffset;
}

void Settings::setViewColumns(int numColumns)
{
	if (this->numColumns == numColumns)
	{
		return;
	}

	this->numColumns = numColumns;
	save();
}

int Settings::getViewColumns() const
{
	return this->numColumns;
}


void Settings::setViewRows(int numRows)
{
	if (this->numRows == numRows)
	{
		return;
	}

	this->numRows = numRows;
	save();
}

int Settings::getViewRows() const
{
	return this->numRows;
}

void Settings::setViewFixedRows(bool viewFixedRows)
{
	if (this->viewFixedRows == viewFixedRows)
	{
		return;
	}

	this->viewFixedRows = viewFixedRows;
	save();
}

bool Settings::isViewFixedRows() const
{
	return this->viewFixedRows;
}

void Settings::setViewLayoutVert(bool vert)
{
	if (this->layoutVertical == vert)
	{
		return;
	}

	this->layoutVertical = vert;
	save();
}

bool Settings::getViewLayoutVert() const
{
	return this->layoutVertical;
}

void Settings::setViewLayoutR2L(bool r2l)
{
	if (this->layoutRightToLeft == r2l)
	{
		return;
	}

	this->layoutRightToLeft = r2l;
	save();
}

bool Settings::getViewLayoutR2L() const
{
	return this->layoutRightToLeft;
}

void Settings::setViewLayoutB2T(bool b2t)
{
	if (this->layoutBottomToTop == b2t)
	{
		return;
	}

	this->layoutBottomToTop = b2t;
	save();
}

bool Settings::getViewLayoutB2T() const
{
	return this->layoutBottomToTop;
}

void Settings::setLastSavePath(Path p)
{
	this->lastSavePath = p;
	save();
}

Path const& Settings::getLastSavePath() const
{
	return this->lastSavePath;
}

void Settings::setLastOpenPath(Path p)
{
	this->lastOpenPath = p;
	save();
}

Path const& Settings::getLastOpenPath() const
{
	return this->lastOpenPath;
}

void Settings::setLastImagePath(Path path)
{
	if (this->lastImagePath == path)
	{
		return;
	}
	this->lastImagePath = path;
	save();
}

Path const& Settings::getLastImagePath() const
{
	return this->lastImagePath;
}

void Settings::setZoomStep(double zoomStep)
{
	if (this->zoomStep == zoomStep)
	{
		return;
	}
	this->zoomStep = zoomStep;
	save();
}

double Settings::getZoomStep() const
{
	return this->zoomStep;
}

void Settings::setZoomStepScroll(double zoomStepScroll)
{
	if (this->zoomStepScroll == zoomStepScroll)
	{
		return;
	}
	this->zoomStepScroll = zoomStepScroll;
	save();
}

double Settings::getZoomStepScroll() const
{
	return this->zoomStepScroll;
}

void Settings::setDisplayDpi(int dpi)
{
	if (this->displayDpi == dpi)
	{
		return;
	}
	this->displayDpi = dpi;
	save();
}

int Settings::getDisplayDpi() const
{
	return this->displayDpi;
}

void Settings::setDarkTheme(bool dark)
{
	if (this->darkTheme == dark)
	{
		return;
	}
	this->darkTheme = dark;
	save();
}

bool Settings::isDarkTheme() const
{
	return this->darkTheme;
}

bool Settings::isSidebarVisible() const
{
	return this->showSidebar;
}

void Settings::setSidebarVisible(bool visible)
{
	if (this->showSidebar == visible)
	{
		return;
	}
	this->showSidebar = visible;
	save();
}

int Settings::getSidebarWidth() const
{
	return this->sidebarWidth;
}

void Settings::setSidebarWidth(int width)
{
	width = std::max(width, 50);

	if (this->sidebarWidth == width)
	{
		return;
	}
	this->sidebarWidth = width;
	save();
}

void Settings::setMainWndSize(int width, int height)
{
	this->mainWndWidth = width;
	this->mainWndHeight = height;

	save();
}

int Settings::getMainWndWidth() const
{
	return this->mainWndWidth;
}

int Settings::getMainWndHeight() const
{
	return this->mainWndHeight;
}

bool Settings::isMainWndMaximized() const
{
	return this->maximized;
}

void Settings::setMainWndMaximized(bool max)
{
	this->maximized = max;
}

void Settings::setSelectedToolbar(string name)
{
	if (this->selectedToolbar == name)
	{
		return;
	}
	this->selectedToolbar = name;
	save();
}

string const& Settings::getSelectedToolbar() const
{
	return this->selectedToolbar;
}

SElement& Settings::getCustomElement(string name)
{
	return this->data[name];
}

void Settings::customSettingsChanged()
{
	save();
}

ButtonConfig* Settings::getButtonConfig(int id)
{
	if (id < 0 || id >= BUTTON_COUNT)
	{
		g_error("Settings::getButtonConfig try to get id=%i out of range!", id);
		return nullptr;
	}
	return this->buttonConfig[id];
}

ButtonConfig* Settings::getEraserButtonConfig()
{
	return this->buttonConfig[0];
}

ButtonConfig* Settings::getMiddleButtonConfig()
{
	return this->buttonConfig[1];
}

ButtonConfig* Settings::getRightButtonConfig()
{
	return this->buttonConfig[2];
}

ButtonConfig* Settings::getTouchButtonConfig()
{
	return this->buttonConfig[3];
}

ButtonConfig* Settings::getDefaultButtonConfig()
{
	return this->buttonConfig[4];
}

ButtonConfig* Settings::getStylusButton1Config()
{
	return this->buttonConfig[5];
}

ButtonConfig* Settings::getStylusButton2Config()
{
	return this->buttonConfig[6];
}

string const& Settings::getFullscreenHideElements() const
{
	return this->fullscreenHideElements;
}

void Settings::setFullscreenHideElements(string elements)
{
	this->fullscreenHideElements = elements;
	save();
}

string const& Settings::getPresentationHideElements() const
{
	return this->presentationHideElements;
}

void Settings::setPresentationHideElements(string elements)
{
	this->presentationHideElements = elements;
	save();
}

int Settings::getPdfPageCacheSize() const
{
	return this->pdfPageCacheSize;
}

void Settings::setPdfPageCacheSize(int size)
{
	if (this->pdfPageCacheSize == size)
	{
		return;
	}
	this->pdfPageCacheSize = size;
	save();
}

int Settings::getBorderColor() const
{
	return this->selectionBorderColor;
}

void Settings::setBorderColor(int color)
{
	if (this->selectionBorderColor == color)
	{
		return;
	}
	this->selectionBorderColor = color;
	save();
}

int Settings::getSelectionColor() const
{
	return this->selectionMarkerColor;
}

void Settings::setSelectionColor(int color)
{
	if (this->selectionMarkerColor == color)
	{
		return;
	}
	this->selectionMarkerColor = color;
	save();
}

int Settings::getBackgroundColor() const
{
	return this->backgroundColor;
}

void Settings::setBackgroundColor(int color)
{
	if (this->backgroundColor == color)
	{
		return;
	}
	this->backgroundColor = color;
	save();
}

XojFont& Settings::getFont()
{
	return this->font;
}

void Settings::setFont(const XojFont& font)
{
	this->font = font;
	save();
}


PaDeviceIndex Settings::getAudioInputDevice() const
{
	return this->audioInputDevice;
}

void Settings::setAudioInputDevice(PaDeviceIndex deviceIndex)
{
	if (this->audioInputDevice == deviceIndex)
	{
		return;
	}
	this->audioInputDevice = deviceIndex;
	save();
}

PaDeviceIndex Settings::getAudioOutputDevice() const
{
	return this->audioOutputDevice;
}

void Settings::setAudioOutputDevice(PaDeviceIndex deviceIndex)
{
	if (this->audioOutputDevice == deviceIndex)
	{
		return;
	}
	this->audioOutputDevice = deviceIndex;
	save();
}

double Settings::getAudioSampleRate() const
{
	return this->audioSampleRate;
}

void Settings::setAudioSampleRate(double sampleRate)
{
	if (this->audioSampleRate == sampleRate)
	{
		return;
	}
	this->audioSampleRate = sampleRate;
	save();
}

double Settings::getAudioGain() const
{
	return this->audioGain;
}

void Settings::setAudioGain(double gain)
{
	if (this->audioGain == gain)
	{
		return;
	}
	this->audioGain = gain;
	save();
}

unsigned int Settings::getDefaultSeekTime() const
{
	return this->defaultSeekTime;
}

void Settings::setDefaultSeekTime(unsigned int t)
{
	if (this->defaultSeekTime == t)
	{
		return;
	}
	this->defaultSeekTime = t;
	save();
}

string const& Settings::getPluginEnabled() const
{
	return this->pluginEnabled;
}

void Settings::setPluginEnabled(string pluginEnabled)
{
	if (this->pluginEnabled == pluginEnabled)
	{
		return;
	}
	this->pluginEnabled = pluginEnabled;
	save();
}

string const& Settings::getPluginDisabled() const
{
	return this->pluginDisabled;
}

void Settings::setPluginDisabled(string pluginDisabled)
{
	if (this->pluginDisabled == pluginDisabled)
	{
		return;
	}
	this->pluginDisabled = pluginDisabled;
	save();
}


void Settings::getStrokeFilter(int* ignoreTime, double* ignoreLength, int* successiveTime) const
{
	*ignoreTime = this->strokeFilterIgnoreTime;
	*ignoreLength = this->strokeFilterIgnoreLength;
	*successiveTime = this->strokeFilterSuccessiveTime;

}

void Settings::setStrokeFilter( int ignoreTime, double ignoreLength, int successiveTime)
{
	this->strokeFilterIgnoreTime = ignoreTime;
	this->strokeFilterIgnoreLength = ignoreLength;
	this->strokeFilterSuccessiveTime = successiveTime;

}

void Settings::setStrokeFilterEnabled(bool enabled)
{
	this->strokeFilterEnabled = enabled;
}

bool Settings::getStrokeFilterEnabled() const
{
	return this->strokeFilterEnabled;
}

void Settings::setDoActionOnStrokeFiltered(bool enabled)
{
	this->doActionOnStrokeFiltered = enabled;
}

bool Settings::getDoActionOnStrokeFiltered() const
{
	return this->doActionOnStrokeFiltered;
}

void Settings::setTrySelectOnStrokeFiltered(bool enabled)
{
	this->trySelectOnStrokeFiltered = enabled;
}

bool Settings::getTrySelectOnStrokeFiltered() const
{
	return this->trySelectOnStrokeFiltered;
}


void Settings::setExperimentalInputSystemEnabled(bool systemEnabled)
{
	if (this->experimentalInputSystemEnabled == systemEnabled)
	{
		return;
	}
	this->experimentalInputSystemEnabled = systemEnabled;
	save();
}

bool Settings::getExperimentalInputSystemEnabled() const
{
	return this->experimentalInputSystemEnabled;
}

void Settings::setInputSystemTPCButtonEnabled(bool tpcButtonEnabled)
{
	if (this->inputSystemTPCButton == tpcButtonEnabled)
	{
		return;
	}
	this->inputSystemTPCButton = tpcButtonEnabled;
	save();
}

bool Settings::getInputSystemTPCButtonEnabled() const
{
	return this->inputSystemTPCButton;
}

void Settings::setInputSystemDrawOutsideWindowEnabled(bool drawOutsideWindowEnabled)
{
	if (this->inputSystemDrawOutsideWindow == drawOutsideWindowEnabled)
	{
		return;
	}
	this->inputSystemDrawOutsideWindow = drawOutsideWindowEnabled;
	save();
}

bool Settings::getInputSystemDrawOutsideWindowEnabled() const
{
	return this->inputSystemDrawOutsideWindow;
}

void Settings::setDeviceClassForDevice(GdkDevice* device, int deviceClass)
{
	this->setDeviceClassForDevice(gdk_device_get_name(device), gdk_device_get_source(device), deviceClass);
}

void Settings::setDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource, int deviceClass)
{
	auto it = inputDeviceClasses.find(deviceName);
	if (it != inputDeviceClasses.end())
	{
		it->second.first = deviceClass;
		it->second.second = deviceSource;
	}
	else
	{
		inputDeviceClasses.insert(std::pair<string, std::pair<int, GdkInputSource>>(
		        deviceName, std::pair<int, GdkInputSource>(deviceClass, deviceSource)));
	}
}

std::vector<InputDevice> Settings::getKnownInputDevices() const
{
	std::vector<InputDevice> inputDevices;
	for (std::pair<string, std::pair<int, GdkInputSource>> device: inputDeviceClasses)
	{
		inputDevices.emplace_back(device.first, device.second.second);
	}
	return inputDevices;
}

int Settings::getDeviceClassForDevice(GdkDevice* device) const
{
	return this->getDeviceClassForDevice(gdk_device_get_name(device), gdk_device_get_source(device));
}

int Settings::getDeviceClassForDevice(const string& deviceName, GdkInputSource deviceSource) const
{
	auto search = inputDeviceClasses.find(deviceName);
	if (search != inputDeviceClasses.end())
	{
		return search->second.first;
	}
	else
	{
		guint deviceType = 0;
		switch (deviceSource)
		{
			case GDK_SOURCE_CURSOR:
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
			case GDK_SOURCE_TABLET_PAD:
#endif
			case GDK_SOURCE_KEYBOARD:
				deviceType = 0;
				break;
			case GDK_SOURCE_MOUSE:
			case GDK_SOURCE_TOUCHPAD:
#if (GDK_MAJOR_VERSION >= 3 && GDK_MINOR_VERSION >= 22)
			case GDK_SOURCE_TRACKPOINT:
#endif
				deviceType = 1;
				break;
			case GDK_SOURCE_PEN:
				deviceType = 2;
				break;
			case GDK_SOURCE_ERASER:
				deviceType = 3;
				break;
			case GDK_SOURCE_TOUCHSCREEN:
				deviceType = 4;
				break;
		    default:
			    deviceType = 0;
		}
		return deviceType;
	}
}

bool Settings::isScrollbarFadeoutDisabled() const
{
	return disableScrollbarFadeout;
}

void Settings::setScrollbarFadeoutDisabled(bool disable)
{
	if (disableScrollbarFadeout == disable) return;
	disableScrollbarFadeout = disable;
	save();
}

//////////////////////////////////////////////////

SAttribute::SAttribute()
{
	this->dValue = 0;
	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;
}

SAttribute::SAttribute(const SAttribute& attrib)
{
	*this = attrib;
}

SAttribute::~SAttribute()
{
	this->iValue = 0;
	this->type = ATTRIBUTE_TYPE_NONE;
}

//////////////////////////////////////////////////

__RefSElement::__RefSElement()
{
	this->refcount = 0;
}

__RefSElement::~__RefSElement()
{
}

void __RefSElement::ref()
{
	this->refcount++;
}

void __RefSElement::unref()
{
	this->refcount--;
	if (this->refcount == 0)
	{
		delete this;
	}
}

SElement::SElement()
{
	this->element = new __RefSElement();
	this->element->ref();
}

SElement::SElement(const SElement& elem)
{
	this->element = elem.element;
	this->element->ref();
}

SElement::~SElement()
{
	this->element->unref();
	this->element = nullptr;
}

void SElement::operator=(const SElement& elem)
{
	this->element = elem.element;
	this->element->ref();
}

std::map<string, SAttribute>& SElement::attributes()
{
	return this->element->attributes;
}

std::map<string, SElement>& SElement::children()
{
	return this->element->children;
}

void SElement::clear()
{
	this->element->attributes.clear();
	this->element->children.clear();
}

SElement& SElement::child(string name)
{
	return this->element->children[name];
}

void SElement::setComment(const string name, const string comment)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.comment = comment;
}

void SElement::setIntHex(const string name, const int value)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT_HEX;
}

void SElement::setInt(const string name, const int value)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_INT;
}

void SElement::setBool(const string name, const bool value)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.iValue = value;
	attrib.type = ATTRIBUTE_TYPE_BOOLEAN;
}

void SElement::setString(const string name, const string value)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.sValue = value;
	attrib.type = ATTRIBUTE_TYPE_STRING;
}

void SElement::setDouble(const string name, const double value)
{
	SAttribute& attrib = this->element->attributes[name];
	attrib.dValue = value;
	attrib.type = ATTRIBUTE_TYPE_DOUBLE;
}

bool SElement::getDouble(const string name, double& value)
{
	SAttribute& attrib = this->element->attributes[name];
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
	SAttribute& attrib = this->element->attributes[name];
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
	SAttribute& attrib = this->element->attributes[name];
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
	SAttribute& attrib = this->element->attributes[name];
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

