/*
 * Xournal++
 *
 * The main Control
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "RecentManager.h"
#include "../gui/MainWindow.h"
#include "Actions.h"
#include "../undo/UndoRedoHandler.h"
#include "ClipboardHandler.h"
#include "settings/Settings.h"
#include "ToolHandler.h"
#include "../model/Document.h"
#include "ZoomControl.h"
#include "jobs/XournalScheduler.h"
#include "../util/MemoryCheck.h"

#include "../gui/sidebar/Sidebar.h"
#include "../gui/SearchBar.h"
#include "../gui/Cursor.h"
#include "jobs/ProgressListener.h"
#include "ScrollHandler.h"

class Sidebar;
class CallbackData;
class PageView;
class EditSelection;
class SaveHandler;
class GladeSearchpath;

class Control: public ActionHandler,
		public ToolListener,
		public DocumentHandler,
		public RecentManagerListener,
		public UndoRedoListener,
		public ClipboardListener,
		public MemoryCheckObject,
		public ProgressListener {
public:
	Control(GladeSearchpath * gladeSearchPath);
	virtual ~Control();

	void initWindow(MainWindow * win);
public:
	// Menu File
	void newFile();
	bool openFile(String filename = NULL, int scrollToPage = -1);
	bool annotatePdf(String filename, bool attachPdf, bool attachToDocument);
	void print();
	void exportAsPdf();
	void exportAs();
	bool save(bool synchron = false);
	void saveAs();
	void quit();
	bool close();

	void resetShapeRecognizer();

	// Menu edit
	void showSettings();

	// Menu Help
	void showAbout();

	Settings * getSettings();
	ToolHandler * getToolHandler();

	virtual void actionPerformed(ActionType type, ActionGroup group, GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton, bool enabled);

	virtual void toolColorChanged();
	virtual void setCustomColorSelected();
	virtual void toolChanged();
	virtual void toolSizeChanged();
	virtual void eraserTypeChanged();

	void setEraserType(EraserType eraserType);

	void selectTool(ToolType type);
	void selectDefaultTool();

	void updatePageNumbers(int page, int pdfPage);

	Document * getDocument();

	virtual void fileOpened(const char * uri);

	ZoomControl * getZoomControl();

	/**
	 * Save current state
	 */
	void saveSettings();

	UndoRedoHandler * getUndoRedoHandler();
	MainWindow * getWindow();
	Cursor * getCursor();
	RecentManager * getRecentManager();
	ScrollHandler * getScrollHandler();

	void updateWindowTitle();

	int getCurrentPageNo();

	XojPage * getCurrentPage();
	void calcZoomFitSize();

	void setViewTwoPages(bool continous);

	void setPageInsertType(PageInsertType type);

	void enableFullscreen(bool enabled, bool presentation = false);

	void addNewLayer();
	void deleteCurrentLayer();

	void paperFormat();
	void changePageBackgroundColor();
	void setPageBackground(ActionType type);
	void updateBackgroundSizeButton();

	bool isFullscreen();

	static String getFilename(String uri);

	bool searchTextOnPage(const char * text, int p, int * occures, double * top);

	void firePageSelected(XojPage * page);
	void firePageSelected(int page);

	void addDefaultPage();
	void insertNewPage(int position);
	void insertPage(XojPage * page, int position);
	void deletePage();

	/**
	 * Disable / enable delete page button
	 */
	void updateDeletePageButton();

	// selection handling
	void clearSelection();
	EditSelection * getSelectionFor(PageView * view);
	EditSelection * getSelection();
	void setSelection(EditSelection * selection);
	void paintSelection(cairo_t * cr, GdkEventExpose *event, double zoom, PageView * view);

	void setCopyPasteEnabled(bool enabled);

	void enableAutosave(bool enable);

	void getDefaultPagesize(double & width, double & height);

	void clearSelectionEndText();

	void setToolSize(ToolSize size);

	TextEditor * getTextEditor();

	void disableSidebarTmp(bool disabled);

	XournalScheduler * getScheduler();

	void block(const char * name);
	void unblock();

public:
	// UndoRedoListener interface
	void undoRedoChanged();
	void undoRedoPageChanged(XojPage * page);

public:
	// ProgressListener interface
	void setMaximumState(int max);
	void setCurrentState(int state);

public:
	// ClipboardListener interface
	void clipboardCutCopyEnabled(bool enabled);
	void clipboardPasteEnabled(bool enabled);
	void clipboardPasteText(String text);
	void clipboardPasteXournal(ObjectInputStream & in);
	void deleteSelection();

protected:
	static bool invokeCallback(CallbackData * cb);
	void invokeLater(ActionType type);
	void zoomFit();

	bool shouldIgnorAction(ActionType type, ActionGroup group, bool enabled);

	bool showSaveDialog();

	void fileLoaded();

	void eraserSizeChanged();
	void penSizeChanged();
	void hilighterSizeChanged();

	static bool checkChangedDocument(Control * control);
	static bool autosaveCallback(Control * control);

	void fontChanged();
private:
	RecentManager * recent;
	UndoRedoHandler * undoRedo;
	ZoomControl * zoom;
	bool fullscreen;

	Settings * settings;
	MainWindow * win;

	Document * doc;

	Sidebar * sidebar;
	SearchBar * searchBar;

	ToolHandler * toolHandler;

	Cursor * cursor;

	ActionType lastAction;
	ActionGroup lastGroup;
	bool lastEnabled;

	GList * hiddenFullscreenWidgets;
	bool sidebarHidden;

	ScrollHandler * scrollHandler;

	/**
	 * Timeout id: the timeout watches the changes and actualizes the previews from time to time
	 */
	int changeTimout;

	/**
	 * The pages wihch has changed since the last update (for preview update)
	 */
	GList * changedPages;

	/**
	 * Our clipboard abstraction
	 */
	ClipboardHandler * clipboardHandler;

	/**
	 * Selected content, if any
	 */
	EditSelection * selection;

	/**
	 * The autosave handler ID
	 */
	int autosaveTimeout;
	SaveHandler * autosaveHandler;
	String lastAutosaveFilename;

	/**
	 * Default page size
	 */
	double defaultWidth;
	double defaultHeight;

	XournalScheduler * scheduler;

	/**
	 * State / Blocking attributes
	 */
	GtkWidget * statusbar;
	GtkLabel * lbState;
	GtkProgressBar * pgState;
	int maxState;
	bool isBlocking;

	GladeSearchpath * gladeSearchPath;

};

class CallbackData {
public:
	CallbackData(Control * control, ActionType type) {
		this->control = control;
		this->type = type;
	}

	ActionType type;
	Control * control;
};

#endif /* __CONTROL_H__ */
