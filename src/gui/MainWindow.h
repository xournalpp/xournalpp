/*
 * Xournal++
 *
 * The Main window
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "GladeGui.h"
#include "toolbarMenubar/ToolMenuHandler.h"

#include "XournalWidget.h"

class Control;
class MainWindow;

class MainWindow: public GladeGui {
public:
	MainWindow(GladeSearchpath * gladeSearchPath, Control * control);
	virtual ~MainWindow();

public:
	void show();

	void setRecentMenu(GtkWidget * submenu);
	void toolbarSelected(ToolbarData * d);

	void updatePageNumbers(int page, int pagecount, int pdfpage);
	int getCurrentLayer();

	void setFontButtonFont(XojFont & font);
	XojFont getFontButtonFont();

	void setMaximized(bool maximized);
	bool isMaximized();

	XournalWidget * getXournal();

	void setSidebarVisible(bool visible);

	Control * getControl();

	void updateScrollbarSidebarPosition();

	void setUndoDescription(String description);
	void setRedoDescription(String description);

	void updateLayerCombobox();

	GtkWidget * getSpinPageNo();

	void setControlTmpDisabled(bool disabled);

private:
	void initToolbar();

	static void pageNrSpinChangedCallback(GtkSpinButton * spinbutton, MainWindow * win);
	static void buttonCloseSidebarClicked(GtkButton * button, MainWindow * win);

	/**
	 * Sidebar show / hidden
	 */
	static void viewShowSidebar(GtkCheckMenuItem * checkmenuitem, MainWindow * control);

	/**
	 * Window close Button is pressed
	 */
	static bool deleteEventCallback(GtkWidget * widget, GdkEvent * event, Control * control);

	/**
	 * Callback fro window states, we ned to know if the window is fullscreen
	 */
	static bool windowStateEventCallback(GtkWidget * window, GdkEventWindowState * event, MainWindow * win);
private:
	Control * control;

	XournalWidget * xournal;

	// Toolbars
	ToolMenuHandler * toolbar;
	GSList * toolbarGroup;
	GList * toolbarMenuitem;
	ToolbarData * selectedToolbar;
	bool toolbarIntialized;

	bool maximized;

	GtkWidget * tbTop1;
	GtkWidget * tbTop2;
	GtkWidget * tbLeft1;
	GtkWidget * tbLeft2;
	GtkWidget * tbRight1;
	GtkWidget * tbRight2;
	GtkWidget * tbBottom1;
	GtkWidget * tbBottom2;
};

#endif /* __MAINWINDOW_H__ */
