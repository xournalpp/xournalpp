/*
 * Xournal++
 *
 * Xournal Settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <libxml/xmlreader.h>
#include <glib.h>
#include "../../model/Font.h"
#include "../Tool.h"
#include "../../cfg.h"

#include <map>

enum AttributeType {
	ATTRIBUTE_TYPE_NONE, ATTRIBUTE_TYPE_STRING, ATTRIBUTE_TYPE_INT, ATTRIBUTE_TYPE_DOUBLE, ATTRIBUTE_TYPE_INT_HEX, ATTRIBUTE_TYPE_BOOLEAN,
};

enum ScrollbarHideType {
	SCROLLBAR_HIDE_NONE = 0,
	SCROLLBAR_HIDE_HORIZONTAL = 1,
	SCROLLBAR_HIDE_VERTICAL = 2,
	SCROLLBAR_HIDE_BOTH = 3
};

class ButtonConfig;

class SAttribute {
public:
	SAttribute();
	SAttribute(const SAttribute & attrib);
	virtual ~SAttribute();

public:
	XOJ_TYPE_ATTRIB;

	String sValue;
	int iValue;
	double dValue;

	AttributeType type;

	String comment;
};

class SElement;

class __RefSElement {
public:
	__RefSElement();
	virtual ~__RefSElement();

public:
	void ref();
	void unref();

private:
	XOJ_TYPE_ATTRIB;

	std::map<String, SAttribute> attributes;
	std::map<String, SElement> children;

	int refcount;

	friend class SElement;
};

class SElement {
public:
	SElement();
	SElement(const SElement& elem);
	virtual ~SElement();

public:
	void operator=(const SElement& elem);

	void clear();

	SElement & child(String name);

	void setIntHex(const String name, const int value);
	void setInt(const String name, const int value);
	void setDouble(const String name, const double value);
	void setBool(const String name, const bool value);
	void setString(const String name, const String value);

	void setComment(const String name, const String comment);

	bool getInt(const String name, int & value);
	bool getDouble(const String name, double & value);
	bool getBool(const String name, bool & value);
	bool getString(const String name, String & value);

	std::map<String, SAttribute> & attributes();
	std::map<String, SElement> & children();

private:
	XOJ_TYPE_ATTRIB;

	__RefSElement * element;
};

class Settings {
public:
	Settings(String filename);
	virtual ~Settings();

public:
	bool load();
	void parseData(xmlNodePtr cur, SElement & elem);

	void save();

	/**
	 * Check if there is an XInput device
	 */
	void checkCanXInput();

	/**
	 * Enables / disables extended events
	 */
	//	void updateXEvents();
private:
	void loadDefault();
	void saveTimeout();
	static gboolean saveCallback(Settings * data);

	void parseItem(xmlDocPtr doc, xmlNodePtr cur);

	xmlNodePtr savePropertyDouble(const gchar * key, double value, xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar * key, int value, xmlNodePtr parent);
	xmlNodePtr saveProperty(const gchar * key, const gchar * value, xmlNodePtr parent);

	void saveData(xmlNodePtr root, String name, SElement & elem);

	void saveButtonConfig();
	void loadButtonConfig();
public:
	// Getter- / Setter
	bool isPresureSensitivity();
	void setPresureSensitivity(gboolean presureSensitivity);

	/**
	 * XInput is enabled by the user
	 */
	bool isXinputEnabled();
	void setXinputEnabled(gboolean useXinput);

	/**
	 * Disable Core events if XInput is enabled
	 */
	bool isIgnoreCoreEvents();
	void setIgnoreCoreEvents(bool ignor);

	/**
	 * XInput is available
	 */
	bool isXInputAvailable();

	/**
	 * XInput should be used in the application
	 */
	bool isUseXInput();

	/**
	 * The last used font
	 */
	XojFont & getFont();
	void setFont(const XojFont & font);

	/**
	 * The selected Toolbar
	 */
	void setSelectedToolbar(String name);
	String getSelectedToolbar();

	/**
	 * Sets the screen resolution in DPI
	 */
	void setDisplayDpi(int dpi);
	int getDisplayDpi();

	/**
	 * The last saved path
	 */
	void setLastSavePath(String path);
	String getLastSavePath();

	void setLastImagePath(String path);
	String getLastImagePath();

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

	bool isAutloadPdfXoj();
	void setAutoloadPdfXoj(bool load);

	int getAutosaveTimeout();
	void setAutosaveTimeout(int autosave);
	bool isAutosaveEnabled();
	void setAutosaveEnabled(bool autosave);

	bool isAllowScrollOutsideThePage();
	void setAllowScrollOutsideThePage(bool outside);

	bool isEnableLeafEnterWorkaround();
	void setEnableLeafEnterWorkaround(bool enable);

	bool isShowBigCursor();
	void setShowBigCursor(bool b);

	ScrollbarHideType getScrollbarHideType();
	void setScrollbarHideType(ScrollbarHideType type);

	String getDefaultSaveName();
	void setDefaultSaveName(String name);

	ButtonConfig * getButtonConfig(int id);

	ButtonConfig * getEraserButtonConfig();
	ButtonConfig * getMiddleButtonConfig();
	ButtonConfig * getRightButtonConfig();
	ButtonConfig * getTouchButtonConfig();
	ButtonConfig * getDefaultButtonConfig();

	String getFullscreenHideElements();
	void setFullscreenHideElements(String elements);

	String getPresentationHideElements();
	void setPresentationHideElements(String elements);

	PageInsertType getPageInsertType();
	void setPageInsertType(PageInsertType type);

	int getPageBackgroundColor();
	void setPageBackgroundColor(int color);

	int getSelectionColor();
	void setSelectionColor(int color);

	int getPdfPageCacheSize();
	void setPdfPageCacheSize(int size);

	String getVisiblePageFormats();
public:
	// Custom settings
	SElement & getCustomElement(String name);

	/**
	 * Call this after you have done all custom settings changes
	 */
	void customSettingsChanged();

private:
	Settings(const Settings& settings) {
	}
	void operator=(const Settings & settings) {
	}

private:
	XOJ_TYPE_ATTRIB;

	bool saved;
	gint timeoutId;

	/**
	 * The config filename
	 */
	String filename;

private:
	// Settings
	/**
	 * The settings tree
	 */
	std::map<String, SElement> data;

	/**
	 * Use XInput
	 */
	bool useXinput;

	/**
	 * If there is an XInput device available
	 */
	bool canXIput;

	/**
	 * Use pen pressure to control stroke width?
	 */
	bool presureSensitivity;

	/**
	 * Ignore core events if XInput is enabled
	 */
	bool ignoreCoreEvents;

	/**
	 * If the sidebar is visible
	 */
	bool showSidebar;

	/**
	 * The Width of the Sidebar
	 */
	int sidebarWidth;

	/**
	 * If the sidebar is on the right
	 */
	bool sidebarOnRight;

	/**
	 * Show a better visibel cursor for pen
	 */
	bool showBigCursor;

	/**
	 * Hide the scrollbar
	 */
	ScrollbarHideType scrollbarHideType;

	/**
	 * The selected Toolbar name
	 */
	String selectedToolbar;

	/**
	 * The last saved folder
	 */
	String lastSavePath;

	/**
	 * The last "insert image" folder
	 */
	String lastImagePath;

	/**
	 * The last used font
	 */
	XojFont font;

	/**
	 * The display resolution, in pixels per inch
	 */
	gint displayDpi;

	/**
	 * If the window is maximized
	 */
	bool maximized;

	/**
	 * The Main window size
	 */
	int mainWndWidth;
	int mainWndHeight;

	/**
	 * Show Scrollbar on left
	 */
	bool scrollbarOnLeft;

	/**
	 * Displays two pages
	 */
	bool showTwoPages;

	/**
	 * Automatically load filename.pdf.xoj instead of filename.pdf (true/false)
	 */
	bool autoloadPdfXoj;

	/**
	 * Minimum width multiplier
	 */
	double widthMinimumMultiplier;

	/**
	 * maximum width multiplier
	 */
	double widthMaximumMultiplier;

	/**
	 * automatically save documents for crash recovery each x minutes
	 */
	int autosaveTimeout;
	bool autosaveEnabled;

	/**
	 * allow scroll outside the page
	 */
	bool allowScrollOutsideThePage;

	/**
	 * Enable Bugfix to prevent crash on GTK 2.18 etc
	 */
	bool enableLeafEnterWorkaround;

	/**
	 * Default name if you save a new document
	 */
	String defaultSaveName;

	/**
	 * The button config
	 *
	 * 0: eraser
	 * 1: middle button
	 * 2: right button
	 * 3: touch screen
	 * 4: default
	 */
	ButtonConfig * buttonConfig[5];

	/**
	 * Which gui elements are hidden if you are in Fullscreen mode, separated by a colon (,)
	 */
	String fullscreenHideElements;
	String presentationHideElements;

	/**
	 * If you insert a page, which type will be selected? Plain, Lined, Copy current page...
	 */
	PageInsertType pageInsertType;

	/**
	 * The background color of a new inserted page
	 */
	int pageBackgroundColor;

	/**
	 * The count of pages which will be cached
	 */
	int pdfPageCacheSize;

	/**
	 * The color to draw borders on selected elements (Page, insert image selection etc.)
	 */
	int selectionColor;

	/**
	 * The page format which are visibel
	 */
	String visiblePageFormats;
};

#endif /* __SETTINGS_H__ */
