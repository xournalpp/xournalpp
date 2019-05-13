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
#include "control/layer/LayerCtrlListener.h"

class Control;
class Layout;
class MainWindow;
class SpinPageAdapter;
class ScrollHandling;
class ToolMenuHandler;
class ToolbarData;
class ToolbarModel;
class XournalView;
class MainWindowToolbarMenu;
class ZoomGesture;

class MainWindow : public GladeGui, public LayerCtrlListener
{
public:
	MainWindow(GladeSearchpath* gladeSearchPath, Control* control);
	virtual ~MainWindow();

	// LayerCtrlListener
public:
	virtual void rebuildLayerMenu();
	virtual void layerVisibilityChanged();

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

	SpinPageAdapter* getSpinPageNo();
	ToolbarModel* getToolbarModel();
	ToolMenuHandler* getToolMenuHandler();

	void disableAudioPlaybackButtons();
	void enableAudioPlaybackButtons();
	void setAudioPlaybackPaused(bool paused);

	void setControlTmpDisabled(bool disabled);

	void updateToolbarMenu();

	GtkWidget** getToolbarWidgets(int& length);
	const char* getToolbarName(GtkToolbar* toolbar);

	Layout* getLayout();

	bool isGestureActive();
	
	
	void showFloatingToolbox(int menutype, int x, int y);

private:
	void initXournalWidget();

	/**
	 * Allow to hide menubar, but only if global menu is not enabled
	 */
	void initHideMenu();
	static void toggleMenuBar(MainWindow* win);

	void createToolbarAndMenu();
	
	void initFloatingToolbar();

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

	
	/**
	 * Callback for positioning overlayed floating menu
	 */
	static gboolean  getOverlayPosition (GtkOverlay *overlay, GtkWidget *widget, GdkRectangle *allocation, MainWindow* win);
	
	
private:
	XOJ_TYPE_ATTRIB;

	Control* control;

	XournalView* xournal = NULL;
	GtkWidget* winXournal = NULL;
	ScrollHandling* scrollHandling = NULL;

	ZoomGesture* zoomGesture = NULL;

	// Toolbars
	ToolMenuHandler* toolbar;
	ToolbarData* selectedToolbar = NULL;
	bool toolbarIntialized = false;

	bool maximized = false;

	GtkWidget** toolbarWidgets;

	MainWindowToolbarMenu* toolbarSelectMenu;

	/**
	 * Workaround for double hide menubar event
	 */
	bool ignoreNextHideEvent;
	
	int overlayX = 0;
	int overlayY = 0;
	
};
