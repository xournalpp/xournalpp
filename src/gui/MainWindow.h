/*
 * Xournal++
 *
 * The Main window
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "GladeGui.h"
#include "model/Font.h"

#include <XournalType.h>

#include <vector>

class Control;
class Layout;
class MainWindow;
class SpinPageAdapter;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class MainWindowToolbarMenu;
class ZoomGesture;

class MainWindow : public GladeGui
{
public:
	MainWindow(GladeSearchpath* gladeSearchPath, Control* control);
	virtual ~MainWindow();

public:
	virtual void show(GtkWindow* parent);

	void setRecentMenu(GtkWidget* submenu);
	void toolbarSelected(ToolbarData* d);
	ToolbarData* getSelectedToolbar();
	void reloadToolbars();

	/**
	 * This methods are only used internally and for toolbar configuration
	 */
	ToolbarData* clearToolbar();
	void loadToolbar(ToolbarData* d);


	void updatePageNumbers(size_t page, size_t pagecount, size_t pdfpage);
	int getCurrentLayer();

	void setFontButtonFont(XojFont& font);
	XojFont getFontButtonFont();

	void saveSidebarSize();

	void setMaximized(bool maximized);
	bool isMaximized();

	XournalView* getXournal();

	void setSidebarVisible(bool visible);

	Control* getControl();

	void updateScrollbarSidebarPosition();

	void setUndoDescription(string description);
	void setRedoDescription(string description);

	void updateLayerCombobox();

	SpinPageAdapter* getSpinPageNo();
	ToolbarModel* getToolbarModel();
	ToolMenuHandler* getToolMenuHandler();

	void setControlTmpDisabled(bool disabled);

	void updateToolbarMenu();

	GtkWidget** getToolbarWidgets(int& length);
	const char* getToolbarName(GtkToolbar* toolbar);

	Layout* getLayout();

private:
	void createToolbarAndMenu();

	static void buttonCloseSidebarClicked(GtkButton* button, MainWindow* win);

	/**
	 * Sidebar show / hidden
	 */
	static void viewShowSidebar(GtkCheckMenuItem* checkmenuitem, MainWindow* control);

	/**
	 * Window close Button is pressed
	 */
	static bool deleteEventCallback(GtkWidget* widget, GdkEvent* event, Control* control);

	/**
	 * Key is pressed
	 */
	static bool onKeyPressCallback(GtkWidget* widget, GdkEventKey* event, MainWindow* win);

	/**
	 * Callback fro window states, we ned to know if the window is fullscreen
	 */
	static bool windowStateEventCallback(GtkWidget* window, GdkEventWindowState* event, MainWindow* win);

	/**
	 * Callback for drag & drop files
	 */
	static void dragDataRecived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y,
								GtkSelectionData* data, guint info, guint time, MainWindow* win);

private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	XournalView* xournal;

	ZoomGesture* zoomGesture;

	// Toolbars
	ToolMenuHandler* toolbar;
	ToolbarData* selectedToolbar;
	bool toolbarIntialized;

	bool maximized;

	GtkWidget** toolbarWidgets;

	MainWindowToolbarMenu* toolbarSelectMenu;
};
