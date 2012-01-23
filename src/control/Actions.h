/*
 * Xournal++
 *
 * Handler for actions, every menu action, tool button etc. is defined here
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ACTIONDISPATCHER_H__
#define __ACTIONDISPATCHER_H__

#include <XournalType.h>

#include <gtk/gtk.h>

enum ActionType {
	ACTION_NONE = 0,

	// Menu file
	ACTION_NEW = 100,
	ACTION_OPEN,
	ACTION_ANNOTATE_PDF,
	ACTION_SAVE,
	ACTION_SAVE_AS,
	ACTION_EXPORT_AS_PDF,
	ACTION_EXPORT_AS,
	ACTION_DOCUMENT_PROPERTIES,
	ACTION_PRINT,
	ACTION_QUIT,

	// Menu edit
	ACTION_UNDO = 200,
	ACTION_REDO,
	ACTION_CUT,
	ACTION_COPY,
	ACTION_PASTE,
	ACTION_SEARCH,
	ACTION_DELETE,
	ACTION_SETTINGS,

	// Menu navigation
	ACTION_GOTO_FIRST = 300,
	ACTION_GOTO_BACK,
	ACTION_GOTO_PAGE,
	ACTION_GOTO_NEXT,
	ACTION_GOTO_LAST,
	ACTION_GOTO_NEXT_ANNOTATED_PAGE,
	ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE,

	//Menu Journal
	ACTION_NEW_PAGE_BEFORE = 400,
	ACTION_NEW_PAGE_AFTER,
	ACTION_NEW_PAGE_AT_END,

	// Has to be in the same order as in Tool.h: PageInsertType!
	ACTION_NEW_PAGE_PLAIN,
	ACTION_NEW_PAGE_LINED,
	ACTION_NEW_PAGE_RULED,
	ACTION_NEW_PAGE_GRAPH,
	ACTION_NEW_PAGE_COPY,
	ACTION_NEW_PAGE_PDF_BACKGROUND,

	ACTION_DELETE_PAGE,
	ACTION_NEW_LAYER,
	ACTION_DELETE_LAYER,
	ACTION_PAPER_FORMAT,
	ACTION_PAPER_BACKGROUND_COLOR,

	ACTION_SET_PAPER_BACKGROUND_PLAIN,
	ACTION_SET_PAPER_BACKGROUND_LINED,
	ACTION_SET_PAPER_BACKGROUND_RULED,
	ACTION_SET_PAPER_BACKGROUND_GRAPH,
	ACTION_SET_PAPER_BACKGROUND_IMAGE,
	ACTION_SET_PAPER_BACKGROUND_PDF,

	// Menu Tools
	// Has to be in the same order as in Tool.h: ToolType!
	ACTION_TOOL_PEN = 500,
	ACTION_TOOL_ERASER,
	ACTION_TOOL_HILIGHTER,
	ACTION_TOOL_TEXT,
	ACTION_TOOL_IMAGE,
	ACTION_TOOL_SELECT_RECT,
	ACTION_TOOL_SELECT_REGION,
	ACTION_TOOL_SELECT_OBJECT,
	ACTION_TOOL_VERTICAL_SPACE,
	ACTION_TOOL_HAND,
	ACTION_TOOL_DEFAULT,

	ACTION_SHAPE_RECOGNIZER,
	ACTION_RULER,

	ACTION_SIZE_VERY_THIN,
	ACTION_SIZE_FINE,
	ACTION_SIZE_MEDIUM,
	ACTION_SIZE_THICK,
	ACTION_SIZE_VERY_THICK,

	ACTION_TOOL_ERASER_STANDARD,
	ACTION_TOOL_ERASER_WHITEOUT,
	ACTION_TOOL_ERASER_DELETE_STROKE,

	ACTION_TOOL_ERASER_SIZE_FINE,
	ACTION_TOOL_ERASER_SIZE_MEDIUM,
	ACTION_TOOL_ERASER_SIZE_THICK,

	ACTION_TOOL_PEN_SIZE_VERY_THIN,
	ACTION_TOOL_PEN_SIZE_FINE,
	ACTION_TOOL_PEN_SIZE_MEDIUM,
	ACTION_TOOL_PEN_SIZE_THICK,
	ACTION_TOOL_PEN_SIZE_VERY_THICK,

	ACTION_TOOL_HILIGHTER_SIZE_FINE,
	ACTION_TOOL_HILIGHTER_SIZE_MEDIUM,
	ACTION_TOOL_HILIGHTER_SIZE_THICK,

	// Used for all colors
	ACTION_SELECT_COLOR,
	ACTION_SELECT_COLOR_CUSTOM,

	ACTION_SELECT_FONT,
	ACTION_FONT_BUTTON_CHANGED,

	// Menu View
	ACTION_ZOOM_IN = 600,
	ACTION_ZOOM_OUT,
	ACTION_ZOOM_FIT,
	ACTION_ZOOM_100,
	ACTION_FULLSCREEN,
	ACTION_VIEW_TWO_PAGES,
	ACTION_MANAGE_TOOLBAR,
	ACTION_CUSTOMIZE_TOOLBAR,

	// Menu Help
	ACTION_ABOUT = 800,
	ACTION_HELP,


	// Footer, not really an action, but need an identifier too
	ACTION_FOOTER_PAGESPIN = 900,
	ACTION_FOOTER_ZOOM_SLIDER,
	ACTION_FOOTER_LAYER,


	// Used to select no item in a group...
	ACTION_NOT_SELECTED = 1
};

enum ActionGroup {
	GROUP_NOGROUP = 0,
	GROUP_TOOL = 1,
	GROUP_COLOR,
	GROUP_SIZE,
	GROUP_ERASER_MODE,
	GROUP_ERASER_SIZE,
	GROUP_PEN_SIZE,
	GROUP_HILIGHTER_SIZE,
	GROUP_PAGE_INSERT_TYPE,

	// Need group for toggle button, this is the first Toggle Group
	GROUP_TOGGLE_GROUP,

	GROUP_TWOPAGES,

	GROUP_FULLSCREEN,

	GROUP_SHAPE_RECOGNIZER,
	GROUP_RULER,
};

class ActionHandler;

class ActionEnabledListener {
public:
	ActionEnabledListener();
	virtual ~ActionEnabledListener();

public:
	virtual void actionEnabledAction(ActionType action, bool enabled) = 0;

	void registerListener(ActionHandler * handler);
	void unregisterListener();

private:
	XOJ_TYPE_ATTRIB;


	ActionHandler * handler;
};

class ActionSelectionListener {
public:
	ActionSelectionListener();
	virtual ~ActionSelectionListener();

	virtual void actionSelected(ActionGroup group, ActionType action) = 0;

	void registerListener(ActionHandler * handler);
	void unregisterListener();

private:
	XOJ_TYPE_ATTRIB;


	ActionHandler * handler;
};

class ActionHandler {
public:
	ActionHandler();
	virtual ~ActionHandler();

public:
	virtual void actionPerformed(ActionType type, ActionGroup group, GdkEvent *event, GtkMenuItem *menuitem,
			GtkToolButton *toolbutton, bool enabled) = 0;

	void fireEnableAction(ActionType action, bool enabled);
	void addListener(ActionEnabledListener * listener);
	void removeListener(ActionEnabledListener * listener);

	void fireActionSelected(ActionGroup group, ActionType action);
	void addListener(ActionSelectionListener * listener);
	void removeListener(ActionSelectionListener * listener);

private:
	XOJ_TYPE_ATTRIB;


	GList * enabledListener;
	GList * selectionListener;
};

#endif /* __ACTIONDISPATCHER_H__ */
