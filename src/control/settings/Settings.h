/*
 * Xournal++
 *
 * Xournal Settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/Tool.h"
#include "model/Font.h"

#include <config-dev.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;
#include <libxml/xmlreader.h>

#include <map>

enum AttributeType
{
	ATTRIBUTE_TYPE_NONE,
	ATTRIBUTE_TYPE_STRING,
	ATTRIBUTE_TYPE_INT,
	ATTRIBUTE_TYPE_DOUBLE,
	ATTRIBUTE_TYPE_INT_HEX,
	ATTRIBUTE_TYPE_BOOLEAN,
};

// use this as a bit flag
enum ScrollbarHideType
{
    SCROLLBAR_HIDE_NONE = 0,
    SCROLLBAR_HIDE_HORIZONTAL = 1 << 1,
    SCROLLBAR_HIDE_VERTICAL = 1 << 2,
    SCROLLBAR_HIDE_BOTH = SCROLLBAR_HIDE_HORIZONTAL | SCROLLBAR_HIDE_VERTICAL
};

class ButtonConfig;

extern const char* BUTTON_NAMES[];
const int BUTTON_COUNT = 7;


class SAttribute
{
public:
	SAttribute();
	SAttribute(const SAttribute& attrib);
	virtual ~SAttribute();

public:
	XOJ_TYPE_ATTRIB;

	string sValue;
	int iValue;
	double dValue;

	AttributeType type;

	string comment;
};

class SElement;

class __RefSElement
{
public:
	__RefSElement();
	virtual ~__RefSElement();

public:
	void ref();
	void unref();

private:
	XOJ_TYPE_ATTRIB;

	std::map<string, SAttribute> attributes;
	std::map<string, SElement> children;

	int refcount;

	friend class SElement;
};

class SElement
{
public:
	SElement();
	SElement(const SElement& elem);
	virtual ~SElement();

public:
	void operator=(const SElement& elem);

	void clear();

	SElement& child(string name);

	void setIntHex(const string name, const int value);
	void setInt(const string name, const int value);
	void setDouble(const string name, const double value);
	void setBool(const string name, const bool value);
	void setString(const string name, const string value);

	void setComment(const string name, const string comment);

	bool getInt(const string name, int& value);
	bool getDouble(const string name, double& value);
	bool getBool(const string name, bool& value);
	bool getString(const string name, string& value);

	std::map<string, SAttribute>& attributes();
	std::map<string, SElement>& children();

private:
	XOJ_TYPE_ATTRIB;

	__RefSElement* element;
};

class Settings
{
public:
	Settings(path filename);
	virtual ~Settings();

public:
	bool load();
	void parseData(xmlNodePtr cur, SElement& elem);

	void save();

private:
	void loadDefault();
	void parseItem(xmlDocPtr doc, xmlNodePtr cur);

	xmlNodePtr savePropertyDouble(const gchar* key, double value,
								  xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar* key, int value, xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar* key, const gchar* value,
							xmlNodePtr parent);

	void saveData(xmlNodePtr root, string name, SElement& elem);

	void saveButtonConfig();
	void loadButtonConfig();
public:
	// Getter- / Setter
	bool isPresureSensitivity();
	void setPresureSensitivity(gboolean presureSensitivity);

	/**
	 * The last used font
	 */
	XojFont& getFont();
	void setFont(const XojFont& font);

	/**
	 * The selected Toolbar
	 */
	void setSelectedToolbar(string name);
	string getSelectedToolbar();

	/**
	 * Sets the screen resolution in DPI
	 */
	void setDisplayDpi(int dpi);
	int getDisplayDpi();

	/**
	 * The last saved path
	 */
	void setLastSavePath(path p);
	path getLastSavePath();

	void setLastImagePath(path p);
	path getLastImagePath();

	void setMainWndSize(int width, int height);
	void setMainWndMaximized(bool max);
	int getMainWndWidth();
	int getMainWndHeight();
	bool isMainWndMaximized();

	bool isSidebarVisible();
	void setSidebarVisible(bool visible);

	int getSidebarWidth();
	void setSidebarWidth(int width);

	bool isSidebarOnRight();
	void setSidebarOnRight(bool right);

	bool isScrollbarOnLeft();
	void setScrollbarOnLeft(bool right);

	double getWidthMinimumMultiplier();
	double getWidthMaximumMultiplier();

	void setShowTwoPages(bool showTwoPages);
	bool isShowTwoPages();

	void setPresentationMode(bool presentationMode);
	bool isPresentationMode();

	bool isAutloadPdfXoj();
	void setAutoloadPdfXoj(bool load);

	int getAutosaveTimeout();
	void setAutosaveTimeout(int autosave);
	bool isAutosaveEnabled();
	void setAutosaveEnabled(bool autosave);

	bool getAddVerticalSpace();
	void setAddVerticalSpace(bool space);

	bool getAddHorizontalSpace();
	void setAddHorizontalSpace(bool space);

	bool isEnableLeafEnterWorkaround();
	void setEnableLeafEnterWorkaround(bool enable);

	bool isShowBigCursor();
	void setShowBigCursor(bool b);

	ScrollbarHideType getScrollbarHideType();
	void setScrollbarHideType(ScrollbarHideType type);

	string getDefaultSaveName();
	void setDefaultSaveName(string name);

	ButtonConfig* getButtonConfig(int id);

	ButtonConfig* getEraserButtonConfig();
	ButtonConfig* getMiddleButtonConfig();
	ButtonConfig* getRightButtonConfig();
	ButtonConfig* getTouchButtonConfig();
	ButtonConfig* getDefaultButtonConfig();
	ButtonConfig* getStylusButtonConfig();
	ButtonConfig* getStylus2ButtonConfig();

	string getFullscreenHideElements();
	void setFullscreenHideElements(string elements);

	string getPresentationHideElements();
	void setPresentationHideElements(string elements);

	PageInsertType getPageInsertType();
	void setPageInsertType(PageInsertType type);

	int getPageBackgroundColor();
	void setPageBackgroundColor(int color);

	int getSelectionColor();
	void setSelectionColor(int color);

	int getPdfPageCacheSize();
	void setPdfPageCacheSize(int size);

	string getVisiblePageFormats();

	bool isEventCompression();
	void setEventCompression(bool enabled);
public:
	// Custom settings
	SElement& getCustomElement(string name);

	/**
	 * Call this after you have done all custom settings changes
	 */
	void customSettingsChanged();

private:

	/**
	 * Not implemented, do not use
	 * @param settings
	 */
	Settings(const Settings& settings);

	/**
	 * Not implemented, do not use
	 * @param settings
	 */
	void operator=(const Settings& settings);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 *  The config filename
	 */
	path filename;

private:
	/**
	 * The settings tree
	 */
	std::map<string, SElement> data;

	/**
	 *  Use pen pressure to control stroke width?
	 */
	bool presureSensitivity;

	/**
	 *  If the sidebar is visible
	 */
	bool showSidebar;

	/**
	 *  The Width of the Sidebar
	 */
	int sidebarWidth;

	/**
	 *  If the sidebar is on the right
	 */
	bool sidebarOnRight;

	/**
	 *  Show a better visible cursor for pen
	 */
	bool showBigCursor;

	/**
	 *  Hide the scrollbar
	 */
	ScrollbarHideType scrollbarHideType;

	/**
	 *  The selected Toolbar name
	 */
	string selectedToolbar;

	/**
	 *  The last saved folder
	 */
	path lastSavePath;

	/**
	 *  The last "insert image" folder
	 */
	path lastImagePath;

	/**
	 * The last used font
	 */
	XojFont font;

	/**
	 * The display resolution, in pixels per inch
	 */
	gint displayDpi;

	/**
	 *  If the window is maximized
	 */
	bool maximized;

	/**
	 * Width of the main window
	 */
	int mainWndWidth;

	/**
	 * Height of the main window
	 */
	int mainWndHeight;

	/**
	 * Show the scrollbar on the left side
	 */
	bool scrollbarOnLeft;

	/**
	 *  Displays two pages
	 */
	bool showTwoPages;

	/**
	 *  Sets presentation mode
	 */
	bool presentationMode;

	/**
	 * Automatically load filename.pdf.xoj instead of filename.pdf (true/false)
	 */
	bool autoloadPdfXoj;

	/**
	 *  Minimum width multiplier
	 */
	double widthMinimumMultiplier;

	/**
	 *  Maximum width multiplier
	 */
	double widthMaximumMultiplier;

	/**
	 * Automatically save documents for crash recovery each x minutes
	 */
	int autosaveTimeout;

	/**
	 *  Enable automatic save
	 */
	bool autosaveEnabled;

	/**
	 * Allow scroll outside the page
	 */
	bool addHorizontalSpace, addVerticalSpace;

	/**
	 * Enable Bugfix to prevent crash on GTK 2.18 etc
	 */
	bool enableLeafEnterWorkaround;

	/**
	 * Default name if you save a new document
	 */
	string defaultSaveName; //should be string - don't change to path

	/**
	 * The button config
	 *
	 * 0: eraser
	 * 1: middle button
	 * 2: right button
	 * 3: touch screen
	 * 4: default
	 * 5: Stylus button
	 * 6: Stylus2 button
	 */
	ButtonConfig* buttonConfig[BUTTON_COUNT];

	/**
	 * Which gui elements are hidden if you are in Fullscreen mode,
	 * separated by a colon (,)
	 */
	string fullscreenHideElements;
	string presentationHideElements;

	/**
	 * If you insert a page, which type will be selected?
	 * Plain, Lined, Copy current page...
	 */
	PageInsertType pageInsertType;

	/**
	 * The background color of a new inserted page
	 */
	int pageBackgroundColor;

	/**
	 *  The count of pages which will be cached
	 */
	int pdfPageCacheSize;

	/**
	 * The color to draw borders on selected elements
	 * (Page, insert image selection etc.)
	 */
	int selectionColor;

	/**
	 * The page format which are visible
	 */
	string visiblePageFormats;

	/**
	 * Whether event compression should be enabled
	 */
	bool eventCompression;
};
