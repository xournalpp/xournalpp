#include "Control.h"
#include <gtk/gtk.h>
#include "../gui/dialog/AboutDialog.h"
#include "../gui/dialog/SettingsDialog.h"
#include "../gui/dialog/PdfPagesDialog.h"
#include "../gui/dialog/ImagesDialog.h"
#include "../gui/dialog/FormatDialog.h"
#include "../gui/dialog/SelectBackgroundColorDialog.h"
#include "../cfg.h"
#include "LoadHandler.h"
#include "SaveHandler.h"
#include "PrintHandler.h"
#include "ExportHandler.h"
#include "settings/ev-metadata-manager.h"
#include "../util/CrashHandler.h"
#include "../util/ObjectStream.h"
#include "../util/Stacktrace.h"
#include "../model/FormatDefinitions.h"
#include "../undo/InsertDeletePageUndoAction.h"
#include "../undo/InsertLayerUndoAction.h"
#include "../undo/PageBackgroundChangedUndoAction.h"
#include "../undo/PageBackgroundChangedUndoAction.h"
#include "../undo/InsertUndoAction.h"
#include "../undo/RemoveLayerUndoAction.h"
#include "jobs/BlockingJob.h"
#include "jobs/PdfExportJob.h"
#include "../view/DocumentView.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include <stdio.h>
#include <string.h>

// TODO: Check for error log on startup, also check for emergency save document!


Control::Control() {
	this->win = NULL;
	this->recent = new RecentManager();
	this->undoRedo = new UndoRedoHandler(this);
	this->recent->addListener(this);
	this->undoRedo->addUndoRedoListener(this);

	this->lastAction = ACTION_NONE;
	this->lastGroup = GROUP_NOGROUP;
	this->lastEnabled = false;
	this->fullscreen = false;

	gchar * filename = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, SETTINGS_XML_FILE, NULL);
	String name(filename, true);
	this->settings = new Settings(name);
	this->settings->load();

	this->sidebar = NULL;
	this->searchBar = NULL;

	this->scheduler = new XournalScheduler();

	this->hiddenFullscreenWidgets = NULL;
	this->sidebarHidden = false;
	this->selection = NULL;
	this->autosaveTimeout = 0;
	this->autosaveHandler = NULL;

	this->defaultWidth = -1;
	this->defaultHeight = -1;

	this->statusbar = NULL;
	this->lbState = NULL;
	this->pgState = NULL;
	this->maxState = 0;

	// Init Metadata Manager
	ev_metadata_manager_init();

	doc = new Document(this);

	// for crashhandling
	setEmergencyDocument(doc);

	this->zoom = new ZoomControl();
	this->zoom->setZoom100(settings->getDisplayDpi() / 72.0);

	this->toolHandler = new ToolHandler(this, settings);
	this->toolHandler->loadSettings();

	this->cursor = new Cursor(this);

	/**
	 * This is needed to update the previews
	 */
	this->changeTimout = g_timeout_add_seconds(10, (GSourceFunc) checkChangedDocument, this);
	this->changedPages = NULL;

	this->clipboardHandler = NULL;
}

Control::~Control() {
	g_source_remove(this->changeTimout);
	this->enableAutosave(false);

	if (!this->lastAutosaveFilename.isEmpty()) {
		// delete old autosave file
		GFile * file = g_file_new_for_path(this->lastAutosaveFilename.c_str());
		g_file_delete(file, NULL, NULL);
	}

	delete this->clipboardHandler;
	this->clipboardHandler = NULL;
	delete this->recent;
	this->recent = NULL;
	delete this->undoRedo;
	this->undoRedo = NULL;
	delete this->settings;
	this->settings = NULL;
	delete this->toolHandler;
	this->toolHandler = NULL;
	delete this->doc;
	this->doc = NULL;
	delete this->sidebar;
	this->sidebar = NULL;
	delete this->searchBar;
	this->searchBar = NULL;
}

UndoRedoHandler * Control::getUndoRedoHandler() {
	return undoRedo;
}

bool Control::checkChangedDocument(Control * control) {
	for (GList * l = control->changedPages; l != NULL; l = l->next) {
		int p = control->doc->indexOf((XojPage *) l->data);
		if (p != -1) {
			control->firePageChanged(p);
		}
	}
	g_list_free(control->changedPages);
	control->changedPages = NULL;

	return true;
}

void Control::saveSettings() {
	this->toolHandler->saveSettings();

	gint width = 0;
	gint height = 0;
	gtk_window_get_size((GtkWindow*) *win, &width, &height);

	if (!win->isMaximized()) {
		settings->setMainWndSize(width, height);
	}
	settings->setMainWndMaximized(win->isMaximized());
}

void Control::initWindow(MainWindow * win) {
	win->setRecentMenu(recent->getMenu());
	selectTool(toolHandler->getToolType());
	this->win = win;
	this->zoom->initZoomHandler(win);
	this->sidebar = new Sidebar(win, this);

	updatePageNumbers(0, -1);

	eraserTypeChanged();

	searchBar = new SearchBar(this);

	// Disable undo buttons
	undoRedoChanged();

	setViewTwoPages(settings->isShowTwoPages());

	setPageInsertType(settings->getPageInsertType());

	penSizeChanged();
	eraserSizeChanged();
	hilighterSizeChanged();
	updateDeletePageButton();

	this->clipboardHandler = new ClipboardHandler(this, win->getXournal()->getWidget());

	this->enableAutosave(settings->isAutosaveEnabled());

	win->setFontButtonFont(settings->getFont());
}

ZoomControl * Control::getZoomControl() {
	return zoom;
}

Cursor * Control::getCursor() {
	return cursor;
}

RecentManager * Control::getRecentManager() {
	return this->recent;
}

gpointer Control::autosaveThread(Control * control) {
	String filename = control->doc->getFilename();
	if (filename.isEmpty()) {
		filename = Util::getAutosaveFilename();
	} else {
		if (filename.length() > 5 && filename.substring(-4) == ".xoj") {
			filename = filename.substring(0, -4);
		}
		filename += ".autosave.xoj";
	}

	if (!control->lastAutosaveFilename.isEmpty() && control->lastAutosaveFilename != filename) {
		GFile * file = g_file_new_for_path(control->lastAutosaveFilename.c_str());
		g_file_delete(file, NULL, NULL);
	}
	control->lastAutosaveFilename = filename;

	GzOutputStream * out = new GzOutputStream(filename);
	control->autosaveHandler->saveTo(out, filename);
	if (!control->autosaveHandler->getErrorMessage().isEmpty()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) control->getWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Autosave: %s"),
				control->autosaveHandler->getErrorMessage().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	delete control->autosaveHandler;
	control->autosaveHandler = NULL;
	delete out;

	return 0;
}

bool Control::autosaveCallback(Control * control) {
	if (!control->undoRedo->isChangedAutosave()) {
		printf(_("Info: autosave not necessary, nothing changed...\n"));
		// do nothing, nothing changed
		return true;
	} else {
		printf(_("Info: autosave document...\n"));
	}

	if (control->autosaveHandler) {
		delete control->autosaveHandler;
	}

	control->undoRedo->documentAutosaved();
	control->autosaveHandler = new SaveHandler();
	control->autosaveHandler->prepareSave(control->doc);

	g_thread_create((GThreadFunc) autosaveThread, control, false, NULL);

	return true;
}

void Control::enableAutosave(bool enable) {
	if (this->autosaveTimeout) {
		g_source_remove(this->autosaveTimeout);
		this->autosaveTimeout = 0;
	}

	if (enable) {
		int timeout = settings->getAutosaveTimeout() * 60;
		this->autosaveTimeout = g_timeout_add_seconds(timeout, (GSourceFunc) autosaveCallback, this);
	}
}

void Control::updatePageNumbers(int page, int pdfPage) {
	this->win->updatePageNumbers(page, doc->getPageCount(), pdfPage);
	this->sidebar->selectPageNr(page, pdfPage);

	const char * file = doc->getEvMetadataFilename();
	if (file) {
		ev_metadata_manager_set_int(file, "page", page);
	}

	int current = win->getXournal()->getCurrentPage();
	int count = doc->getPageCount();

	fireEnableAction(ACTION_GOTO_FIRST, current != 0);
	fireEnableAction(ACTION_GOTO_BACK, current != 0);
	fireEnableAction(ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE, current != 0);

	fireEnableAction(ACTION_GOTO_NEXT, current < count - 1);
	fireEnableAction(ACTION_GOTO_LAST, current < count - 1);
	fireEnableAction(ACTION_GOTO_NEXT_ANNOTATED_PAGE, current < count - 1);
}

Document * Control::getDocument() {
	return doc;
}

ToolHandler * Control::getToolHandler() {
	return toolHandler;
}

/**
 * If we change the state of a toggle button it will send an event,
 * to prevent our application to get in an endless loop we need to catch this events
 */
bool Control::shouldIgnorAction(ActionType action, ActionGroup group, bool enabled) {
	// No selection events to catch
	if (group == GROUP_NOGROUP) {
		lastGroup = group;
		return false;
	}

	// Different group, different action
	if (lastGroup != group) {
		lastAction = action;
		lastGroup = group;
		lastEnabled = enabled;
		return false;
	}

	if (GROUP_TOGGLE_GROUP < group) {
		if (lastEnabled == enabled) {
			// same action
			return true;
		} else {
			lastAction = action;
			lastGroup = group;
			lastEnabled = enabled;
			return false;
		}
	}

	lastAction = action;
	lastGroup = group;
	lastEnabled = enabled;

	if (!enabled) {
		return true;
	}

	return false;
}

void Control::actionPerformed(ActionType type, ActionGroup group, GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton, bool enabled) {

	// Because GTK sends events if we change radio items etc.
	if (shouldIgnorAction(type, group, enabled)) {
		return;
	}

	switch (type) {
	// Menu File
	case ACTION_NEW:
		clearSelectionEndText();
		newFile();
		break;
	case ACTION_OPEN:
		clearSelectionEndText();
		openFile();
		break;
	case ACTION_ANNOTATE_PDF:
		clearSelectionEndText();
		annotatePdf(NULL, false, false);
		break;
	case ACTION_SAVE:
		save();
		break;
	case ACTION_SAVE_AS:
		saveAs();
		break;
	case ACTION_EXPORT_AS_PDF:
		exportAsPdf();
		break;
	case ACTION_EXPORT_AS:
		exportAs();
		break;
	case ACTION_DOCUMENT_PROPERTIES:
		// TODO LOW PRIO: not implemented, but menupoint is hidden...
		break;
	case ACTION_PRINT:
		print();
		break;
	case ACTION_QUIT:
		quit();
		break;

		// Menu Edit
	case ACTION_UNDO:
		this->clearSelection();
		undoRedo->undo();
		this->resetShapeRecognizer();
		break;
	case ACTION_REDO:
		this->clearSelection();
		undoRedo->redo();
		this->resetShapeRecognizer();
		break;
	case ACTION_CUT:
		if (!win->getXournal()->cut()) {
			clipboardHandler->cut();
		}
		break;
	case ACTION_COPY:
		if (!win->getXournal()->copy()) {
			clipboardHandler->copy();
		}
		break;
	case ACTION_PASTE:
		if (!win->getXournal()->paste()) {
			clipboardHandler->paste();
		}
		break;
	case ACTION_SEARCH:
		clearSelectionEndText();
		searchBar->showSearchBar(true);
		break;
	case ACTION_DELETE:
		if (!win->getXournal()->actionDelete()) {
			deleteSelection();
		}
		break;
	case ACTION_SETTINGS:
		showSettings();
		break;

		// Menu Navigation
	case ACTION_GOTO_FIRST:
		scrollToPage(0);
		break;
	case ACTION_GOTO_BACK:
		goToPreviousPage();
		break;
	case ACTION_GOTO_NEXT:
		goToNextPage();
		break;
	case ACTION_GOTO_LAST:
		scrollToPage(doc->getPageCount() - 1);
		break;
	case ACTION_GOTO_NEXT_ANNOTATED_PAGE:
		gotoAnnotatedPage(true);
		break;
	case ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE:
		gotoAnnotatedPage(false);
		break;

		// Menu Journal
	case ACTION_NEW_PAGE_BEFORE:
		insertNewPage(getCurrentPageNo());
		break;
	case ACTION_NEW_PAGE_AFTER:
		insertNewPage(getCurrentPageNo() + 1);
		break;
	case ACTION_NEW_PAGE_AT_END:
		insertNewPage(doc->getPageCount());
		break;
	case ACTION_DELETE_PAGE:
		deletePage();
		break;
	case ACTION_NEW_LAYER:
		addNewLayer();
		break;
	case ACTION_DELETE_LAYER:
		deleteCurrentLayer();
		break;
	case ACTION_PAPER_FORMAT:
		paperFormat();
		break;
	case ACTION_PAPER_BACKGROUND_COLOR:
		changePageBackgroundColor();
		break;

	case ACTION_SET_PAPER_BACKGROUND_PLAIN:
	case ACTION_SET_PAPER_BACKGROUND_LINED:
	case ACTION_SET_PAPER_BACKGROUND_RULED:
	case ACTION_SET_PAPER_BACKGROUND_GRAPH:
	case ACTION_SET_PAPER_BACKGROUND_IMAGE:
	case ACTION_SET_PAPER_BACKGROUND_PDF:
		clearSelectionEndText();
		setPageBackground(type);
		break;

	case ACTION_NEW_PAGE_PLAIN:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_PLAIN);
		}
		break;
	case ACTION_NEW_PAGE_LINED:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_LINED);
		}
		break;
	case ACTION_NEW_PAGE_RULED:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_RULED);
		}
		break;
	case ACTION_NEW_PAGE_GRAPH:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_GRAPH);
		}
		break;
	case ACTION_NEW_PAGE_COPY:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_COPY);
		}
		break;
	case ACTION_NEW_PAGE_PDF_BACKGROUND:
		clearSelectionEndText();
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_PDF_BACKGROUND);
		}
		break;

		// Menu Tools
	case ACTION_TOOL_PEN:
		if (enabled) {
			selectTool(TOOL_PEN);
		}
		break;
	case ACTION_TOOL_ERASER:
		if (enabled) {
			selectTool(TOOL_ERASER);
		}
		break;

	case ACTION_TOOL_ERASER_STANDARD:
		if (enabled) {
			setEraserType(ERASER_TYPE_DEFAULT);
		}
		break;
	case ACTION_TOOL_ERASER_DELETE_STROKE:
		if (enabled) {
			setEraserType(ERASER_TYPE_DELETE_STROKE);
		}
		break;
	case ACTION_TOOL_ERASER_WHITEOUT:
		if (enabled) {
			setEraserType(ERASER_TYPE_WHITEOUT);
		}
		break;

	case ACTION_TOOL_HILIGHTER:
		if (enabled) {
			selectTool(TOOL_HILIGHTER);
		}
		break;
	case ACTION_TOOL_TEXT:
		if (enabled) {
			selectTool(TOOL_TEXT);
		}
		break;
	case ACTION_TOOL_IMAGE:
		if (enabled) {
			selectTool(TOOL_IMAGE);
		}
		break;
	case ACTION_TOOL_SELECT_RECT:
		if (enabled) {
			selectTool(TOOL_SELECT_RECT);
		}
		break;
	case ACTION_TOOL_SELECT_REGION:
		if (enabled) {
			selectTool(TOOL_SELECT_REGION);
		}
		break;
	case ACTION_TOOL_SELECT_OBJECT:
		if (enabled) {
			selectTool(TOOL_SELECT_OBJECT);
		}
		break;
	case ACTION_TOOL_VERTICAL_SPACE:
		if (enabled) {
			selectTool(TOOL_VERTICAL_SPACE);
		}
		break;
	case ACTION_TOOL_HAND:
		if (enabled) {
			selectTool(TOOL_HAND);
		}
		break;

	case ACTION_RULER:
		this->toolHandler->setRuler(enabled);
		fireActionSelected(GROUP_RULER, enabled ? ACTION_RULER : ACTION_NONE);
		if (enabled) {
			this->toolHandler->setShapeRecognizer(false);
			fireActionSelected(GROUP_SHAPE_RECOGNIZER, ACTION_NONE);
		}
		break;
	case ACTION_SHAPE_RECOGNIZER:
		this->toolHandler->setShapeRecognizer(enabled);
		fireActionSelected(GROUP_SHAPE_RECOGNIZER, enabled ? ACTION_SHAPE_RECOGNIZER : ACTION_NONE);
		if (enabled) {
			this->resetShapeRecognizer();
			this->toolHandler->setRuler(false);
			fireActionSelected(GROUP_RULER, ACTION_NONE);
		}
		break;
	case ACTION_SIZE_VERY_THIN:
		if (enabled) {
			setToolSize(TOOL_SIZE_VERY_FINE);
		}
		break;
	case ACTION_SIZE_FINE:
		if (enabled) {
			setToolSize(TOOL_SIZE_FINE);
		}
		break;
	case ACTION_SIZE_MEDIUM:
		if (enabled) {
			setToolSize(TOOL_SIZE_MEDIUM);
		}
		break;
	case ACTION_SIZE_THICK:
		if (enabled) {
			setToolSize(TOOL_SIZE_THICK);
		}
		break;
	case ACTION_SIZE_VERY_THICK:
		if (enabled) {
			setToolSize(TOOL_SIZE_VERY_THICK);
		}
		break;

	case ACTION_TOOL_ERASER_SIZE_FINE:
		if (enabled) {
			this->toolHandler->setEraserSize(TOOL_SIZE_FINE);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_ERASER_SIZE_MEDIUM:
		if (enabled) {
			this->toolHandler->setEraserSize(TOOL_SIZE_MEDIUM);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_ERASER_SIZE_THICK:
		if (enabled) {
			this->toolHandler->setEraserSize(TOOL_SIZE_THICK);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_VERY_THIN:
		if (enabled) {
			this->toolHandler->setPenSize(TOOL_SIZE_VERY_FINE);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_FINE:
		if (enabled) {
			this->toolHandler->setPenSize(TOOL_SIZE_FINE);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_MEDIUM:
		if (enabled) {
			this->toolHandler->setPenSize(TOOL_SIZE_MEDIUM);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_THICK:
		if (enabled) {
			this->toolHandler->setPenSize(TOOL_SIZE_THICK);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_VERY_THICK:
		if (enabled) {
			this->toolHandler->setPenSize(TOOL_SIZE_VERY_THICK);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_FINE:
		if (enabled) {
			this->toolHandler->setHilighterSize(TOOL_SIZE_FINE);
			hilighterSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_MEDIUM:
		if (enabled) {
			this->toolHandler->setHilighterSize(TOOL_SIZE_MEDIUM);
			hilighterSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_THICK:
		if (enabled) {
			this->toolHandler->setHilighterSize(TOOL_SIZE_THICK);
			hilighterSizeChanged();
		}
		break;

	case ACTION_SELECT_FONT:
		fontChanged();
		break;

		// Used for all colors
	case ACTION_SELECT_COLOR:
	case ACTION_SELECT_COLOR_CUSTOM:
		// nothing to do here, the color toolbar item handles the color
		break;

		// Menu View
	case ACTION_ZOOM_100:
	case ACTION_ZOOM_FIT:
	case ACTION_ZOOM_IN:
	case ACTION_ZOOM_OUT:
		invokeLater(type);
		break;

	case ACTION_VIEW_TWO_PAGES:
		setViewTwoPages(enabled);
		break;

	case ACTION_FOOTER_LAYER: {
		clearSelectionEndText();
		XojPage * p = getCurrentPage();
		if (p) {
			p->setSelectedLayerId(win->getCurrentLayer());
			win->getXournal()->layerChanged(getCurrentPageNo());
			win->updateLayerCombobox();

			if (p) {
				int layer = p->getSelectedLayerId();
				fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
			}
		}
	}
		break;

	case ACTION_FULLSCREEN:
		enableFullscreen(enabled);
		break;

		// Footer, not really an action, but need an identifier to
	case ACTION_FOOTER_PAGESPIN:
	case ACTION_FOOTER_ZOOM_SLIDER:
		// nothing to do here
		break;

		// Menu Help
	case ACTION_HELP:
		// TODO LOW PRIO: implement help
		break;
	case ACTION_ABOUT:
		showAbout();
		break;

	default:
		g_warning("Unhandled action event: %i / %i", type, group);
		Stacktrace::printStracktrace();
	}

	if (type >= ACTION_TOOL_PEN && type <= ACTION_TOOL_HAND) {
		ActionType at = (ActionType) (toolHandler->getToolType() - TOOL_PEN + ACTION_TOOL_PEN);
		if (type == at && !enabled) {
			fireActionSelected(GROUP_TOOL, at);
		}
	}
}

void Control::clearSelectionEndText() {
	clearSelection();
	if (win) {
		win->getXournal()->endTextSelection();
	}
}

void Control::invokeLater(ActionType type) {
	g_idle_add((GSourceFunc) &invokeCallback, new CallbackData(this, type));
}

void Control::goToPreviousPage() {
	scrollToPage(win->getXournal()->getCurrentPage() - 1);
}

void Control::goToNextPage() {
	scrollToPage(win->getXournal()->getCurrentPage() + 1);
}

/**
 * Fire page selected, but first check if the page Number is valid
 */
void Control::firePageSelected(XojPage * page) {
	int p = doc->indexOf(page);
	if (p == -1) {
		return;
	}

	DocumentHandler::firePageSelected(p);
}

void Control::firePageSelected(int page) {
	DocumentHandler::firePageSelected(page);
}

void Control::enableFullscreen(bool enabled, bool presentation) {
	if (enabled) {
		gtk_window_fullscreen((GtkWindow*) *win);

		String e;
		if (presentation) {
			e = settings->getPresentationHideElements();
		} else {
			e = settings->getFullscreenHideElements();
		}
		char * str = g_strdup(e.c_str());

		char * part = strtok(str, ",");

		while (part) {
			if (!strcmp("sidebarContents", part) && settings->isSidebarVisible()) {
				this->sidebarHidden = true;
				win->setSidebarVisible(false);
			} else {
				GtkWidget * w = win->get(part);
				if (w == NULL) {
					g_warning("Fullscreen: Try to hide \"%s\", but coulden't find it. Wrong entry in ~/" CONFIG_DIR "/" SETTINGS_XML_FILE "?", part);
				} else {
					if (gtk_widget_get_visible(w)) {
						gtk_widget_hide(w);
						this->hiddenFullscreenWidgets = g_list_append(this->hiddenFullscreenWidgets, w);
					}
				}
			}

			part = strtok(NULL, ",");
		}

		g_free(str);
	} else {
		gtk_window_unfullscreen((GtkWindow*) *win);

		for (GList * l = this->hiddenFullscreenWidgets; l != NULL; l = l->next) {
			gtk_widget_show(GTK_WIDGET(l->data));
		}

		if (this->sidebarHidden) {
			this->sidebarHidden = false;
			win->setSidebarVisible(true);
		}

		g_list_free(this->hiddenFullscreenWidgets);
		this->hiddenFullscreenWidgets = NULL;
	}

	fireActionSelected(GROUP_FULLSCREEN, enabled ? ACTION_FULLSCREEN : ACTION_NONE);
	this->fullscreen = enabled;
}

void Control::setSidebarTmpDisabled(bool disabled) {
	this->sidebar->setTmpDisabled(disabled);
}

XournalScheduler * Control::getScheduler() {
	return this->scheduler;
}

void Control::addDefaultPage() {
	PageInsertType type = settings->getPageInsertType();

	double width = 0;
	double heigth = 0;

	getDefaultPagesize(width, heigth);

	XojPage * page = new XojPage(width, heigth);
	page->setBackgroundColor(settings->getPageBackgroundColor());

	if (PAGE_INSERT_TYPE_PLAIN == type) {
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (PAGE_INSERT_TYPE_RULED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (PAGE_INSERT_TYPE_GRAPH == type) {
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else {//PAGE_INSERT_TYPE_LINED or PDF or COPY
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	}

	doc->addPage(page);

	updateDeletePageButton();
}

void Control::getDefaultPagesize(double & width, double & height) {
	if (this->defaultHeight < 0) {
		SElement & format = settings->getCustomElement("format");
		format.setComment("paperformat", "Available values are: system, A4, Letter, Custom: For custom you have to create the tags width and height.");
		String settingsPaperFormat;

		double w = 0;
		double h = 0;

		String paper;

		if (format.getString("paperformat", settingsPaperFormat)) {
			if (settingsPaperFormat == "system") {
				// nothing to do
			} else if (settingsPaperFormat == "Custom") {
				if (format.getDouble("width", w) && format.getDouble("height", h)) {
					width = w;
					height = h;
					this->defaultHeight = h;
					this->defaultWidth = w;
					return;
				}
			} else {
				paper = settingsPaperFormat;
			}
		} else {
			format.setString("paperformat", "system");
		}

		GtkPaperSize * size = NULL;

		if (paper != NULL) {
			GList * list = gtk_paper_size_get_paper_sizes(false);
			for (GList * l = list; l != NULL; l = l->next) {
				GtkPaperSize * s = (GtkPaperSize *) l->data;

				if (paper.equalsIgnorCase(gtk_paper_size_get_display_name(s))) {
					size = s;
				} else {
					gtk_paper_size_free(s);
				}
			}

			g_list_free(list);
		}

		if (size == NULL) {
			size = gtk_paper_size_new(NULL);
		}

		this->defaultWidth = gtk_paper_size_get_width(size, GTK_UNIT_POINTS);
		this->defaultHeight = gtk_paper_size_get_height(size, GTK_UNIT_POINTS);

		gtk_paper_size_free(size);
	}

	width = this->defaultWidth;
	height = this->defaultHeight;
}

void Control::updateDeletePageButton() {
	if (this->win) {
		GtkWidget * w = this->win->get("menuDeletePage");
		gtk_widget_set_sensitive(w, doc->getPageCount() > 1);
	}
}

void Control::deletePage() {
	clearSelectionEndText();
	// don't allow delete pages if we have less than 2 pages,
	// so we can be (more or less) sure there is at least one page.
	if (doc->getPageCount() < 2) {
		return;
	}

	int pNr = getCurrentPageNo();
	if (pNr < 0 || pNr > doc->getPageCount()) {
		// something went wrong...
		return;
	}

	XojPage * page = doc->getPage(pNr);

	// first send event, then delete page...
	firePageDeleted(pNr);
	doc->deletePage(pNr);

	updateDeletePageButton();
	undoRedo->addUndoAction(new InsertDeletePageUndoAction(page, pNr, false));
}

void Control::insertNewPage(int position) {
	clearSelectionEndText();
	PageInsertType type = settings->getPageInsertType();

	if (position > doc->getPageCount()) {
		position = doc->getPageCount();
	}

	double width = 0;
	double height = 0;
	getDefaultPagesize(width, height);

	XojPage * page = new XojPage(width, height);
	page->setBackgroundColor(settings->getPageBackgroundColor());

	if (PAGE_INSERT_TYPE_PLAIN == type) {
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (PAGE_INSERT_TYPE_LINED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (PAGE_INSERT_TYPE_RULED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (PAGE_INSERT_TYPE_GRAPH == type) {
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else if (PAGE_INSERT_TYPE_COPY == type) {
		XojPage * current = getCurrentPage();
		if (current == NULL) { // should not happen, but if you open an ivalid file or something like this...
			page->setBackgroundType(BACKGROUND_TYPE_LINED);
		} else {
			BackgroundType bg = current->getBackgroundType();
			page->setBackgroundType(bg);
			if (bg == BACKGROUND_TYPE_PDF) {
				page->setBackgroundPdfPageNr(current->getPdfPageNr());
			} else if (bg == BACKGROUND_TYPE_IMAGE) {
				page->backgroundImage = current->backgroundImage;
			} else {
				page->setBackgroundColor(current->getBackgroundColor());
			}
		}
	} else if (PAGE_INSERT_TYPE_PDF_BACKGROUND == type) {
		if (doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("You don't have any PDF pages to select from. Cancel operation,\n"
							"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			// delete page
			page->unreference();
			return;
		} else {
			PdfPagesDialog * dlg = new PdfPagesDialog(this->doc, this->settings);
			for (int i = 0; i < doc->getPageCount(); i++) {
				XojPage * p = doc->getPage(i);
				if (p->getBackgroundType() == BACKGROUND_TYPE_PDF && p->getPdfPageNr() >= 0) {
					dlg->setPageUsed(i);
				}
			}

			dlg->show();

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected < 0 || selected >= doc->getPdfPageCount()) {
				// delete page
				page->unreference();
				return;
			}

			// no need to set a type, if we set the page number the type is also set
			page->setBackgroundPdfPageNr(selected);

			XojPopplerPage * p = doc->getPdfPage(selected);
			page->setSize(p->getWidth(), p->getHeight());
		}
	}

	insertPage(page, position);
}

void Control::insertPage(XojPage * page, int position) {
	doc->insertPage(page, position);
	firePageInserted(position);

	cursor->updateCursor();

	scrollToPage(position);

	updateDeletePageButton();

	undoRedo->addUndoAction(new InsertDeletePageUndoAction(page, position, true));
}

bool Control::isFullscreen() {
	return fullscreen;
}

void Control::addNewLayer() {
	clearSelectionEndText();
	XojPage * p = getCurrentPage();
	if (p == NULL) {
		return;
	}

	Layer * l = new Layer();
	p->insertLayer(l, p->getSelectedLayerId());
	if (win) {
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new InsertLayerUndoAction(p, l));
}

void Control::setPageBackground(ActionType type) {
	XojPage * page = getCurrentPage();

	if (page == NULL) {
		return;
	}

	int pageNr = doc->indexOf(page);
	if (pageNr == -1) {
		return; // should not happen...
	}

	int origPdfPage = page->getPdfPageNr();
	BackgroundType origType = page->getBackgroundType();
	BackgroundImage origBackgroundImage = page->backgroundImage;
	double origW = page->getWidth();
	double origH = page->getHeight();

	if (ACTION_SET_PAPER_BACKGROUND_PLAIN == type) {
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (ACTION_SET_PAPER_BACKGROUND_LINED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (ACTION_SET_PAPER_BACKGROUND_RULED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (ACTION_SET_PAPER_BACKGROUND_GRAPH == type) {
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else if (ACTION_SET_PAPER_BACKGROUND_IMAGE == type) {
		ImagesDialog * dlg = new ImagesDialog(this->doc, this->settings);
		dlg->show();
		BackgroundImage * img = dlg->getSelectedImage();
		if (img) {
			printf("image selected\n");
			page->backgroundImage = *img;
			page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
		} else if (dlg->shouldShowFilechooser()) {
			GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open Image"), (GtkWindow*) *win, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
			gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

			GtkFileFilter *filterSupported = gtk_file_filter_new();
			gtk_file_filter_set_name(filterSupported, _("Images"));
			gtk_file_filter_add_pixbuf_formats(filterSupported);
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

			if (!settings->getLastSavePath().isEmpty()) {
				gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
			}

			GtkWidget * attach_opt = gtk_check_button_new_with_label(_("Attach file to the journal"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attach_opt), false);
			gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attach_opt);

			if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
				gtk_widget_destroy(dialog);
				return;
			}
			char *name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			bool attach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
					attach_opt));

			String filename = name;
			char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
			settings->setLastSavePath(folder);
			g_free(folder);
			g_free(name);

			gtk_widget_destroy(dialog);

			BackgroundImage newImg;
			GError * err = NULL;
			newImg.loadFile(filename, &err);
			newImg.setAttach(attach);
			if (err) {
				GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
						_("This image could not be loaded. Error message: %s"), err->message);
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				g_error_free(err);
				return;
			} else {
				page->backgroundImage = newImg;
				page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
			}
		}

		GdkPixbuf * pixbuf = page->backgroundImage.getPixbuf();
		if (pixbuf) {
			page->setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
			firePageSizeChanged(pageNr);
		}

		/**
		 // TODO LOW PRIO: create preview for images, also previews for all other filechoosers!

		 image = gtk_image_new ();
		 gtk_file_chooser_set_preview_widget (chooser_dialog, image);
		 g_signal_connect (chooser_dialog, "update-preview",
		 G_CALLBACK (avatar_chooser_update_preview_cb),
		 chooser);
		 static void
		 avatar_chooser_update_preview_cb (GtkFileChooser       *file_chooser,
		 EmpathyAvatarChooser *chooser)
		 {
		 gchar *filename;

		 filename = gtk_file_chooser_get_preview_filename (file_chooser);

		 if (filename) {
		 GtkWidget *image;
		 GdkPixbuf *pixbuf = NULL;
		 GdkPixbuf *scaled_pixbuf;

		 pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

		 image = gtk_file_chooser_get_preview_widget (file_chooser);

		 if (pixbuf) {
		 scaled_pixbuf = empathy_pixbuf_scale_down_if_necessary (pixbuf, AVATAR_SIZE_SAVE);
		 gtk_image_set_from_pixbuf (GTK_IMAGE (image), scaled_pixbuf);
		 g_object_unref (scaled_pixbuf);
		 g_object_unref (pixbuf);
		 } else {
		 gtk_image_set_from_stock (GTK_IMAGE (image),
		 "gtk-dialog-question",
		 GTK_ICON_SIZE_DIALOG);
		 }

		 g_free (filename);
		 }

		 gtk_file_chooser_set_preview_widget_active (file_chooser, TRUE);
		 }




		 */

		delete dlg;
	} else if (ACTION_SET_PAPER_BACKGROUND_PDF == type) {
		if (doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("You don't have any PDF pages to select from. Cancel operation,\n"
							"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		} else {
			PdfPagesDialog * dlg = new PdfPagesDialog(this->doc, this->settings);
			for (int i = 0; i < doc->getPageCount(); i++) {
				XojPage * p = doc->getPage(i);
				if (p->getBackgroundType() == BACKGROUND_TYPE_PDF && p->getPdfPageNr() >= 0) {
					dlg->setPageUsed(i);
				}
			}

			dlg->show();

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected < 0 || selected >= doc->getPdfPageCount()) {
				return;
			}

			// no need to set a type, if we set the page number the type is also set
			page->setBackgroundPdfPageNr(selected);
		}
	}

	firePageChanged(pageNr);
	updateBackgroundSizeButton();

	undoRedo->addUndoAction(new PageBackgroundChangedUndoAction(page, origType, origPdfPage, origBackgroundImage, origW, origH));
}

void Control::updateBackgroundSizeButton() {
	// Update paper color button
	XojPage * p = getCurrentPage();
	if (p == NULL || win == NULL) {
		return;
	}
	BackgroundType bg = p->getBackgroundType();
	GtkWidget * paperColor = win->get("menuJournalPaperColor");
	GtkWidget * pageSize = win->get("menuJournalPaperFormat");

	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg && BACKGROUND_TYPE_RULED != bg && BACKGROUND_TYPE_GRAPH != bg) {
		gtk_widget_set_sensitive(paperColor, false);
	} else {
		gtk_widget_set_sensitive(paperColor, true);
	}

	// PDF page size is defined, you cannot change it
	gtk_widget_set_sensitive(pageSize, bg != BACKGROUND_TYPE_PDF);
}

void Control::paperFormat() {
	XojPage * page = getCurrentPage();
	if (!page || page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
		return;
	}
	clearSelectionEndText();

	FormatDialog * dlg = new FormatDialog(settings, page->getWidth(), page->getHeight());
	dlg->show();

	double width = dlg->getWidth();
	double height = dlg->getHeight();

	if (width > 0) {
		doc->setPageSize(page, width, height);
	}

	delete dlg;
}

void Control::changePageBackgroundColor() {
	int pNr = getCurrentPageNo();
	XojPage * p = doc->getPage(pNr);

	if (p == NULL) {
		return;
	}

	clearSelectionEndText();

	BackgroundType bg = p->getBackgroundType();
	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg && BACKGROUND_TYPE_RULED != bg && BACKGROUND_TYPE_GRAPH != bg) {
		return;
	}

	SelectBackgroundColorDialog * dlg = new SelectBackgroundColorDialog(this);
	dlg->show();
	int color = dlg->getSelectedColor();

	if (color == -2) {
		dlg->showColorchooser();
		color = dlg->getSelectedColor();
	}

	if (color != -1) {
		p->setBackgroundColor(color);
		firePageChanged(pNr);
		settings->setPageBackgroundColor(color);
	}

	delete dlg;
}

void Control::deleteCurrentLayer() {
	clearSelectionEndText();
	XojPage * p = getCurrentPage();
	int pId = getCurrentPageNo();
	if (p == NULL) {
		return;
	}

	int lId = p->getSelectedLayerId();
	if (lId < 1) {
		return;
	}
	Layer * l = p->getSelectedLayer();

	p->removeLayer(l);
	if (win) {
		win->getXournal()->layerChanged(pId);
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new RemoveLayerUndoAction(p, l, lId - 1));
	this->resetShapeRecognizer();
}

void Control::calcZoomFitSize() {
	if (doc && win) {
		double width = 0;
		//		for (int i = 0; i < doc->getPageCount(); i++) {
		//			Page * p = doc->getPage(i);
		//			width = MAX(p->getWidth(), width);
		//		}


		XojPage * p = getCurrentPage();
		if (p == NULL) {
			return;
		}
		width = p->getWidth() + 20; // 20: show a shadow

		GtkAllocation allocation = { 0 };
		GtkWidget * w = win->get("scrolledwindowMain");

		gtk_widget_get_allocation(w, &allocation);
		double factor = ((double) allocation.width - 20) / width;
		zoom->setZoomFit(factor);
	}
}

void Control::zoomFit() {
	calcZoomFitSize();
	zoom->zoomFit();
}

void Control::setViewTwoPages(bool twoPages) {
	settings->setShowTwoPages(twoPages);
	fireActionSelected(GROUP_TWOPAGES, twoPages ? ACTION_VIEW_TWO_PAGES : ACTION_NOT_SELECTED);

	int currentPage = getCurrentPageNo();
	win->getXournal()->layoutPages();
	scrollToPage(currentPage);
}

void Control::setPageInsertType(PageInsertType type) {
	settings->setPageInsertType(type);
	fireActionSelected(GROUP_PAGE_INSERT_TYPE, (ActionType) (type - PAGE_INSERT_TYPE_PLAIN + ACTION_NEW_PAGE_PLAIN));
}

bool Control::invokeCallback(CallbackData * cb) {
	gdk_threads_enter();

	ZoomControl * zoom = cb->control->getZoomControl();

	switch (cb->type) {
	case ACTION_ZOOM_100:
		zoom->zoom100();
		break;
	case ACTION_ZOOM_FIT:
		cb->control->zoomFit();
		break;
	case ACTION_ZOOM_IN:
		zoom->zoomIn();
		break;
	case ACTION_ZOOM_OUT:
		zoom->zoomOut();
		break;
	}

	delete cb;

	gdk_threads_leave();

	return false;
}

void Control::scrollToSpinPange() {
	GtkWidget * spinPageNo = win->getSpinPageNo();
	int page = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinPageNo));
	if (page == 0) {
		return;
	}
	scrollToPage(page - 1);
}

int Control::getCurrentPageNo() {
	return win->getXournal()->getCurrentPage();
}

bool Control::searchTextOnPage(const char * text, int p, int * occures, double * top) {
	return getWindow()->getXournal()->searchTextOnPage(text, p, occures, top);
}

XojPage * Control::getCurrentPage() {
	int page = win->getXournal()->getCurrentPage();
	return doc->getPage(page);
}

void Control::fileOpened(const char * uri) {
	openFile(uri);
	win->updateLayerCombobox();
}

void Control::undoRedoChanged() {
	fireEnableAction(ACTION_UNDO, undoRedo->canUndo());
	fireEnableAction(ACTION_REDO, undoRedo->canRedo());

	win->setUndoDescription(undoRedo->undoDescription());
	win->setRedoDescription(undoRedo->redoDescription());

	updateWindowTitle();
}

void Control::undoRedoPageChanged(XojPage * page) {
	for (GList * l = this->changedPages; l != NULL; l = l->next) {
		if (l->data == page) {
			return;
		}
	}
	this->changedPages = g_list_append(this->changedPages, page);
}

void Control::gotoAnnotatedPage(bool next) {
	int step;
	if (next) {
		step = 1;
	} else {
		step = -1;
	}

	for (int i = win->getXournal()->getCurrentPage() + step; i >= 0 && i < doc->getPageCount(); i += step) {
		if (doc->getPage(i)->isAnnotated()) {
			scrollToPage(i);
			return;
		}
	}
}

void Control::selectTool(ToolType type) {
	toolHandler->selectTool(type);
}

void Control::setEraserType(EraserType eraserType) {
	toolHandler->_setEraserType(eraserType);
	eraserTypeChanged();
}

void Control::eraserTypeChanged() {
	switch (toolHandler->getEraserType()) {
	case ERASER_TYPE_DELETE_STROKE:
		fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_DELETE_STROKE);
		break;

	case ERASER_TYPE_WHITEOUT:
		fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_WHITEOUT);
		break;

	case ERASER_TYPE_DEFAULT:
	default:
		fireActionSelected(GROUP_ERASER_MODE, ACTION_TOOL_ERASER_STANDARD);
		break;
	}
}

void Control::toolChanged() {
	ToolType type = toolHandler->getToolType();

	// Convert enum values, enums has to be in the same order!
	ActionType at = (ActionType) (type - TOOL_PEN + ACTION_TOOL_PEN);

	fireActionSelected(GROUP_TOOL, at);

	fireEnableAction(ACTION_SELECT_COLOR, toolHandler->isEnableColor());
	fireEnableAction(ACTION_SELECT_COLOR_CUSTOM, toolHandler->isEnableColor());

	fireEnableAction(ACTION_RULER, toolHandler->isEnableRuler());
	fireEnableAction(ACTION_SHAPE_RECOGNIZER, toolHandler->isEnableShapreRecognizer());

	bool enableSize = toolHandler->isEnableSize();

	fireEnableAction(ACTION_SIZE_MEDIUM, enableSize);
	fireEnableAction(ACTION_SIZE_THICK, enableSize);
	fireEnableAction(ACTION_SIZE_FINE, enableSize);
	fireEnableAction(ACTION_SIZE_VERY_THICK, enableSize);
	fireEnableAction(ACTION_SIZE_VERY_THIN, enableSize);

	if (enableSize) {
		toolSizeChanged();
	}

	// Update color
	if (toolHandler->isEnableColor()) {
		toolColorChanged();
	}

	fireActionSelected(GROUP_SHAPE_RECOGNIZER, toolHandler->isShapeRecognizer() ? ACTION_SHAPE_RECOGNIZER : ACTION_NOT_SELECTED);
	fireActionSelected(GROUP_RULER, toolHandler->isRuler() ? ACTION_RULER : ACTION_NOT_SELECTED);

	cursor->updateCursor();

	if (type != TOOL_SELECT_RECT && type != TOOL_SELECT_REGION && type != TOOL_SELECT_OBJECT) {
		clearSelection();
	}
	if (type != TOOL_TEXT) {
		if (win) {
			win->getXournal()->endTextSelection();
		}
	}
}

void Control::eraserSizeChanged() {
	switch (toolHandler->getEraserSize()) {
	case TOOL_SIZE_FINE:
		fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_FINE);
		break;
	case TOOL_SIZE_MEDIUM:
		fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_MEDIUM);
		break;
	case TOOL_SIZE_THICK:
		fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_THICK);
		break;
	}
}

void Control::penSizeChanged() {
	switch (toolHandler->getPenSize()) {
	case TOOL_SIZE_VERY_FINE:
		fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_VERY_THIN);
		break;
	case TOOL_SIZE_FINE:
		fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_FINE);
		break;
	case TOOL_SIZE_MEDIUM:
		fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_MEDIUM);
		break;
	case TOOL_SIZE_THICK:
		fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_THICK);
		break;
	case TOOL_SIZE_VERY_THICK:
		fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_VERY_THICK);
		break;
	}
}

void Control::hilighterSizeChanged() {
	switch (toolHandler->getHilighterSize()) {
	case TOOL_SIZE_FINE:
		fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_FINE);
		break;
	case TOOL_SIZE_MEDIUM:
		fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_MEDIUM);
		break;
	case TOOL_SIZE_THICK:
		fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_THICK);
		break;
	}
}

void Control::toolSizeChanged() {
	if (toolHandler->getToolType() == TOOL_PEN) {
		penSizeChanged();
	} else if (toolHandler->getToolType() == TOOL_ERASER) {
		eraserSizeChanged();
	} else if (toolHandler->getToolType() == TOOL_HILIGHTER) {
		hilighterSizeChanged();
	}

	switch (toolHandler->getSize()) {
	case TOOL_SIZE_NONE:
		fireActionSelected(GROUP_SIZE, ACTION_NONE);
		break;
	case TOOL_SIZE_VERY_FINE:
		fireActionSelected(GROUP_SIZE, ACTION_SIZE_VERY_THICK);
		break;
	case TOOL_SIZE_FINE:
		fireActionSelected(GROUP_SIZE, ACTION_SIZE_FINE);
		break;
	case TOOL_SIZE_MEDIUM:
		fireActionSelected(GROUP_SIZE, ACTION_SIZE_MEDIUM);
		break;
	case TOOL_SIZE_THICK:
		fireActionSelected(GROUP_SIZE, ACTION_SIZE_THICK);
		break;
	case TOOL_SIZE_VERY_THICK:
		fireActionSelected(GROUP_SIZE, ACTION_SIZE_VERY_THIN);
		break;
	}
}

void Control::toolColorChanged() {
	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR);
	cursor->updateCursor();

	if (this->selection && toolHandler->getColor() != -1) {
		UndoAction * undo = this->selection->setColor(toolHandler->getColor());
		if (undo) {
			undoRedo->addUndoAction(undo);
		}
	}
}

void Control::setCustomColorSelected() {
	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR_CUSTOM);
}

void Control::showSettings() {
	bool xeventEnabled = settings->isUseXInput();
	int selectionColor = settings->getSelectionColor();
	bool allowScrollOutside = settings->isAllowScrollOutsideThePage();
	bool bigCursor = settings->isShowBigCursor();

	SettingsDialog * dlg = new SettingsDialog(settings);
	dlg->show();

	if (xeventEnabled != settings->isUseXInput()) {
		win->getXournal()->updateXEvents();
	}

	if (selectionColor != settings->getSelectionColor()) {
		win->getXournal()->forceUpdatePagenumbers();
	}

	if (allowScrollOutside != settings->isAllowScrollOutsideThePage()) {
		win->getXournal()->layoutPages();
	}

	if (bigCursor != settings->isShowBigCursor()) {
		cursor->updateCursor();
	}

	win->updateScrollbarSidebarPosition();

	enableAutosave(settings->isAutosaveEnabled());

	this->zoom->setZoom100(settings->getDisplayDpi() / 72.0);
	delete dlg;
}

void Control::newFile() {
	if (!this->close()) {
		return;
	}

	Document newDoc(this);

	*doc = newDoc;

	PageInsertType type = settings->getPageInsertType();
	if (type != PAGE_INSERT_TYPE_PLAIN && type != PAGE_INSERT_TYPE_LINED && type != PAGE_INSERT_TYPE_RULED && type != PAGE_INSERT_TYPE_GRAPH) {
		type = PAGE_INSERT_TYPE_LINED;
	}

	addDefaultPage();

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

	fileLoaded();
}

MainWindow * Control::getWindow() {
	return this->win;
}

String Control::showOpenDialog(bool pdf, bool & attachPdf) {
	GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open file"), (GtkWindow*) *win, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter *filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");

	GtkFileFilter *filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");

	GtkFileFilter *filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");

	GtkFileFilter *filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Supported files"));
	gtk_file_filter_add_pattern(filterSupported, "*.xoj");
	gtk_file_filter_add_pattern(filterSupported, "*.pdf");
	gtk_file_filter_add_pattern(filterSupported, "*.PDF");

	if (!pdf) {
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	GtkWidget * attachOpt = NULL;
	if (pdf) {
		attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), FALSE);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
	}

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return NULL;
	}
	char *name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	if (attachOpt) {
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
	}

	String filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(
			dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	return filename;
}

void Control::scrollToPage(XojPage * page, double top) {
	int p = doc->indexOf(page);
	if (p != -1) {
		scrollToPage(p, top);
	}
}

void Control::scrollToPage(int page, double top) {
	win->getXournal()->scrollTo(page, top);
}

void adjustmentScroll(GtkAdjustment * adj, double scroll, int size) {
	double v = gtk_adjustment_get_value(adj);
	double max = gtk_adjustment_get_upper(adj) - size;

	double newPos = v + scroll;
	if (newPos < 0) {
		newPos = 0;
	}

	if (newPos > max) {
		newPos = max;
	}

	gtk_adjustment_set_value(adj, newPos);
}

void Control::scrollRelative(double x, double y) {
	GtkWidget * scroll = win->get("scrolledwindowMain");

	GtkAdjustment * hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scroll));
	GtkAdjustment * vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));

	GtkAllocation alloc = { 0 };
	gtk_widget_get_allocation(scroll, &alloc);

	adjustmentScroll(hadj, x, alloc.width);
	adjustmentScroll(vadj, y, alloc.height);
}

bool Control::openFile(String filename) {
	if (!this->close()) {
		return false;
	}

	if (filename.isEmpty()) {
		bool attachPdf = false;
		filename = showOpenDialog(false, attachPdf);
		if (filename.isEmpty()) {
			return false;
		}
	}

	LoadHandler h;

	String lower = filename.toLowerCase();
	if (filename.toLowerCase().endsWith(".pdf")) {
		if (settings->isAutloadPdfXoj()) {
			String f = filename;
			f += ".xoj";
			Document * tmp = h.loadDocument(f);
			if (tmp) {
				doc->clearDocument();
				*doc = *tmp;
				fileLoaded();
				return true;
			}
		}

		bool an = annotatePdf(filename, false, false);
		fileLoaded();
		return an;
	}

	Document * tmp = h.loadDocument(filename);
	if (!tmp) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Error opening file '%s'\n%s"), filename.c_str(), h.getLastError().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		fileLoaded();
		return false;
	} else {
		doc->clearDocument();
		*doc = *tmp;
	}

	GValue value = { 0 };

	const char * file = doc->getEvMetadataFilename();
	if (file) {
		if (ev_metadata_manager_get(file, "zoom", &value, TRUE) && G_VALUE_TYPE(&value) == G_TYPE_DOUBLE) {
			zoom->setZoom(g_value_get_double(&value));
		}

		if (ev_metadata_manager_get(file, "page", &value, TRUE) && G_VALUE_TYPE(&value) == G_TYPE_INT) {
			scrollToPage(g_value_get_int(&value));
		}
		recent->addRecentFileFilename(file);
	}

	fileLoaded();
	return true;
	// TODO: handle if PDF not found!
}

void Control::fileLoaded() {
	updateWindowTitle();
	win->updateLayerCombobox();
	win->getXournal()->forceUpdatePagenumbers();
	cursor->updateCursor();
	updateDeletePageButton();
}

bool Control::annotatePdf(String filename, bool attachPdf, bool attachToDocument) {
	if (!this->close()) {
		return false;
	}

	if (filename.isEmpty()) {
		filename = showOpenDialog(true, attachToDocument);
		if (filename.isEmpty()) {
			return false;
		}
	}

	cursor->setCursorBusy(true);
	if (doc->readPdf(filename, true, attachToDocument)) {
		int page = 0;
		GValue value = { 0 };

		recent->addRecentFileFilename(filename.c_str());

		const char * file = doc->getEvMetadataFilename();
		if (file && ev_metadata_manager_get(file, "page", &value, TRUE) && G_VALUE_TYPE(&value) == G_TYPE_INT) {
			page = g_value_get_int(&value);
		}
		scrollToPage(page);
	} else {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Error annotate PDF file '%s'\n%s"), filename.c_str(), doc->getLastErrorMsg().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	cursor->setCursorBusy(false);

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	cursor->updateCursor();

	return true;
}

void Control::print() {
	PrintHandler print;
	print.print(this->doc, getCurrentPageNo());
}

void Control::copyProgressCallback(goffset current_num_bytes, goffset total_num_bytes, Control * control) {
	printf("copyProgressCallback: %i, %i\n", (int) current_num_bytes, (int) total_num_bytes);
}

bool Control::copyFile(String source, String target) {

	// we need to build the GFile from a path.
	// But if future versions support URIs, this has to be changed
	GFile * src = g_file_new_for_path(source.c_str());
	GFile * trg = g_file_new_for_path(target.c_str());
	GError * err = NULL;

	bool ok = g_file_copy(src, trg, G_FILE_COPY_OVERWRITE, NULL, (GFileProgressCallback) &copyProgressCallback, this, &err);

	if (!err && !ok) {
		copyError = "Copy error: return false, but didn't set error message";
	}
	if (err) {
		ok = false;
		copyError = err->message;
		g_error_free(err);
	}
	return ok;
}

void Control::updatePreview() {
	const int previewSize = 128;

	if (doc->getPageCount() > 0) {
		XojPage * page = doc->getPage(0);

		double width = page->getWidth();
		double height = page->getHeight();

		double zoom = 1;

		if (width < height) {
			zoom = previewSize / height;
		} else {
			zoom = previewSize / width;
		}
		width *= zoom;
		height *= zoom;

		cairo_surface_t * crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

		cairo_t * cr = cairo_create(crBuffer);
		cairo_scale(cr, zoom, zoom);
		XojPopplerPage * popplerPage = NULL;

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF) {
			int pgNo = page->getPdfPageNr();
			popplerPage = sidebar->getDocument()->getPdfPage(pgNo);
			if (popplerPage) {
				popplerPage->render(cr, false);
			}
		}

		DocumentView view;
		view.drawPage(page, cr);
		cairo_destroy(cr);
		doc->setPreview(crBuffer);
		cairo_surface_destroy(crBuffer);
	} else {
		doc->setPreview(NULL);
	}
}

void Control::block(const char * name) {
	// Disable all gui Control, to get full control over the application
	win->setControlTmpDisabled(true);
	cursor->setCursorBusy(true);
	setSidebarTmpDisabled(true);

	this->statusbar = this->win->get("statusbar");
	this->lbState = GTK_LABEL(this->win->get("lbState"));
	this->pgState = GTK_PROGRESS_BAR(this->win->get("pgState"));

	gtk_label_set_text(this->lbState, name);
	gtk_widget_show(this->statusbar);

	this->maxState = 100;

}

void Control::unblock() {
	this->win->setControlTmpDisabled(false);
	cursor->setCursorBusy(false);
	setSidebarTmpDisabled(false);

	gtk_widget_hide(this->statusbar);
}

void Control::setMaximumState(int max) {
	this->maxState = max;
}

void Control::setCurrentState(int state) {
	gtk_progress_bar_set_fraction(this->pgState, (double) state / this->maxState);
}

bool Control::save() {
	if (doc->getFilename().isEmpty()) {
		if (!showSaveDialog()) {
			return false;
		}
	}

	cursor->setCursorBusy(true);
	block(_("Save"));

	updatePreview();

	SaveHandler h;
	h.prepareSave(this->doc);

	if (this->doc->shouldCreateBackupOnSave()) {
		String backup = doc->getFilename();
		backup += ".bak";
		if (!copyFile(doc->getFilename(), backup)) {
			g_warning("Could not create backup! (The file was created from an older Xournal version)\n%s\n", this->copyError.c_str());
			cursor->setCursorBusy(false);
			return false;
		}

		this->doc->setCreateBackupOnSave(false);
	}

	GzOutputStream * out = new GzOutputStream(this->doc->getFilename());

	if (!out->getLastError().isEmpty()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) getWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Open file error: %s"),
				out->getLastError().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		delete out;
		cursor->setCursorBusy(false);
		return false;
	}

	h.saveTo(out, doc->getFilename());
	out->close();
	getUndoRedoHandler()->documentSaved();
	updateWindowTitle();

	if (!out->getLastError().isEmpty()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *win, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _(
				"Output stream error: %s"), out->getLastError().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		delete out;
		return false;
	}

	delete out;

	recent->addRecentFileFilename(doc->getFilename().c_str());

	unblock();

	return true;
}

String Control::getFilename(String uri) {
	int pos = uri.lastIndexOf(G_DIR_SEPARATOR_S);
	if (pos == -1) {
		return uri;
	}

	String s = uri.substring(pos + 1);

	return s;
}

bool Control::showSaveDialog() {
	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Save File"), (GtkWindow*) *win, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter * filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	String saveFilename = "";

	if (!doc->getFilename().isEmpty()) {
		saveFilename = getFilename(doc->getFilename());
	} else if (!doc->getPdfFilename().isEmpty()) {
		saveFilename = getFilename(doc->getPdfFilename());
		saveFilename += ".xoj";
	} else {
		time_t curtime = time(NULL);
		char stime[128];
		strftime(stime, sizeof(stime), settings->getDefaultSaveName().c_str(), localtime(&curtime));

		saveFilename = stime;
	}

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return false;
	}

	char * name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	String filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(
			dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	doc->setFilename(filename);

	return true;
}

void Control::updateWindowTitle() {
	String title = "";

	if (doc->getFilename().isEmpty()) {
		if (doc->getPdfFilename().isEmpty()) {
			title = _("Unsaved Document");
		} else {
			if (undoRedo->isChanged()) {
				title += "*";
			}
			title += doc->getPdfFilename();
		}
	} else {
		if (undoRedo->isChanged()) {
			title += "*";
		}

		title += getFilename(doc->getFilename());
	}

	title += " - Xournal";

	gtk_window_set_title((GtkWindow*) *win, title.c_str());
}

void Control::exportAsPdf() {
	this->scheduler->addJob(new PdfExportJob(this), JOB_PRIORITY_NONE);
}

void Control::exportAs() {
	ExportHandler handler;
	handler.runExportWithDialog(this->doc, getCurrentPageNo());
}

void Control::saveAs() {
	if (!showSaveDialog()) {
		return;
	}
	if (doc->getFilename().isEmpty()) {
		return;
	}
	doc->setCreateBackupOnSave(false);
	save();
}

void Control::quit() {
	if (!this->close()) {
		return;
	}

	this->scheduler->finishTask();

	settings->save();
	gtk_main_quit();
}

bool Control::close() {
	if (undoRedo->isChanged()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
				_("This document is not saved yet."));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Save"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Discard"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 3);
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		// cancel
		if (res == 3) {
			return false;
		}

		// save
		if (res == 1) {
			if (!this->save()) {
				// if not saved cancel, else close
				return false;
			}
		}
	}

	undoRedo->clearContents();
	return true;
}

void Control::resetShapeRecognizer() {
	if (this->win) {
		this->win->getXournal()->resetShapeRecognizer();
	}
}

void Control::showAbout() {
	AboutDialog * dlg = new AboutDialog();
	dlg->show();
	delete dlg;
}

Settings * Control::getSettings() {
	return settings;
}

void Control::clipboardCutCopyEnabled(bool enabled) {
	fireEnableAction(ACTION_CUT, enabled);
	fireEnableAction(ACTION_COPY, enabled);
}

void Control::clipboardPasteEnabled(bool enabled) {
	fireEnableAction(ACTION_PASTE, enabled);
}

void Control::clipboardPasteText(String text) {
	double x = 0;
	double y = 0;
	int pageNr = getCurrentPageNo();
	if (pageNr == -1) {
		return;
	}
	PageView * view = win->getXournal()->getViewFor(pageNr);
	if (view == NULL) {
		return;
	}

	XojPage * page = doc->getPage(pageNr);
	Layer * layer = page->getSelectedLayer();
	win->getXournal()->getPasteTarget(x, y);

	Text * t = new Text();
	t->setText(text);
	t->setFont(settings->getFont());
	t->setColor(toolHandler->getColor());

	double width = t->getElementWidth();
	double height = t->getElementHeight();

	t->setX(x - width / 2);
	t->setY(y - height / 2);
	layer->addElement(t);
	undoRedo->addUndoAction(new InsertUndoAction(page, layer, t, view));

	EditSelection * selection = new EditSelection(this->undoRedo, t, view, page);
	setSelection(selection);
	view->repaint(t);
}

void Control::clipboardPasteXournal(ObjectInputStream & in) {
	int pNr = getCurrentPageNo();
	if (pNr == -1 && win != NULL) {
		return;
	}

	XojPage * page = doc->getPage(pNr);
	PageView * view = win->getXournal()->getViewFor(pNr);

	if (!view || !page) {
		return;
	}

	EditSelection * selection = NULL;
	Element * element = NULL;
	try {
		String version = in.readString();
		if (version != PACKAGE_STRING) {
			g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PACKAGE_STRING);
		}

		in.readObject("Selection");
		double x = in.readDouble();
		double y = in.readDouble();
		double width = in.readDouble();
		double height = in.readDouble();

		selection = new EditSelection(this->undoRedo, x, y, width, height, page, view);

		int count = in.readInt();

		in.endObject();

		for (int i = 0; i < count; i++) {
			String name = in.getNextObjectName();
			element = NULL;

			if (name == "Stroke") {
				element = new Stroke();
			} else if (name == "Image") {
				element = new Image();
			} else if (name == "Text") {
				element = new Text();
			} else {
				throw INPUT_STREAM_EXCEPTION("Get unknown object %s", name.c_str());
			}

			in >> element;

			selection->addElement(element);
			element = NULL;
		}

		setSelection(selection);
		view->redrawDocumentRegion(x, y, x + width, y + height);

	} catch (std::exception & e) {
		g_warning("could not paste, Exception occurred: %s", e.what());
		Stacktrace::printStracktrace();

		// cleanup
		if (element) {
			delete element;
		}

		if (selection) {
			for (GList * l = selection->getElements(); l != NULL; l = l->next) {
				delete (Element *) l->data;
			}

			delete selection;
		}
	}
}

void Control::deleteSelection() {
	if (this->selection) {
		PageView * view = (PageView *) this->selection->getView();
		DeleteUndoAction * undo = new DeleteUndoAction(this->selection->getPage(), view, false);
		this->selection->fillUndoItemAndDelete(undo);
		this->undoRedo->addUndoAction(undo);

		clearSelection();

		view->repaint();
	}
}

void Control::clearSelection() {
	delete this->selection;
	this->selection = NULL;

	if (this->clipboardHandler) {
		this->clipboardHandler->setSelection(this->selection);
	}

	cursor->setMouseSelectionType(CURSOR_SELECTION_NONE);
	toolHandler->setSelectionEditTools(false, false);
}

void Control::setSelection(EditSelection * selection) {
	clearSelection();
	this->selection = selection;

	if (this->clipboardHandler) {
		this->clipboardHandler->setSelection(this->selection);
	}

	bool canChangeSize = false;
	bool canChangeColor = false;

	for (GList * l = this->selection->getElements(); l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		if (e->getType() == ELEMENT_TEXT) {
			canChangeColor = true;
		} else if (e->getType() == ELEMENT_STROKE) {
			Stroke * s = (Stroke *) e;
			if (s->getToolType() != STROKE_TOOL_ERASER) {
				canChangeColor = true;
			}
			canChangeSize = true;
		}

		if (canChangeColor && canChangeSize) {
			break;
		}
	}

	toolHandler->setSelectionEditTools(canChangeColor, canChangeSize);
}

EditSelection * Control::getSelection() {
	return this->selection;
}

EditSelection * Control::getSelectionFor(PageView * view) {
	if (this->selection && this->selection->getInputView() == view) {
		return this->selection;
	}
	return NULL;
}

void Control::paintSelection(cairo_t * cr, GdkEventExpose *event, double zoom, PageView * view) {
	if (this->selection && this->selection->getView() == view) {
		this->selection->paint(cr, event, zoom);
	}
}

void Control::setCopyPasteEnabled(bool enabled) {
	this->clipboardHandler->setCopyPasteEnabled(enabled);
}

void Control::setToolSize(ToolSize size) {
	if (this->selection) {
		UndoAction * undo = this->selection->setSize(size, toolHandler->getToolThikness(TOOL_PEN), toolHandler->getToolThikness(TOOL_HILIGHTER),
				toolHandler->getToolThikness(TOOL_ERASER));
		if (undo) {
			undoRedo->addUndoAction(undo);
		}
	}
	this->toolHandler->setSize(size);
}

TextEditor * Control::getTextEditor() {
	if (win) {
		return win->getXournal()->getTextEditor();
	}
	return NULL;
}

void Control::fontChanged() {
	XojFont font = win->getFontButtonFont();
	settings->setFont(font);

	if (this->selection) {
		UndoAction * undo = this->selection->setFont(font);
		if (undo) {
			undoRedo->addUndoAction(undo);
		}
	}

	TextEditor * editor = getTextEditor();
	if (editor) {
		editor->setFont(font);
	}
}
