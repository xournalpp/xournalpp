#include "Control.h"
#include <gtk/gtk.h>
#include "../gui/XournalView.h"
#include "../gui/dialog/AboutDialog.h"
#include "../gui/dialog/SettingsDialog.h"
#include "../gui/dialog/PdfPagesDialog.h"
#include "../gui/dialog/ImagesDialog.h"
#include "../gui/dialog/FormatDialog.h"
#include "../gui/dialog/GotoDialog.h"
#include "../gui/dialog/SelectBackgroundColorDialog.h"
#include "../gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"
#include "../gui/dialog/ToolbarManageDialog.h"
#include "../cfg.h"
#include "LoadHandler.h"
#include "PrintHandler.h"
#include "ExportHandler.h"
#include "settings/MetadataManager.h"
#include <CrashHandler.h>
#include <serializing/ObjectInputStream.h>
#include <Stacktrace.h>
#include <XInputUtils.h>
#include "../model/FormatDefinitions.h"
#include "../model/XojPage.h"
#include "../model/BackgroundImage.h"
#include "../undo/InsertDeletePageUndoAction.h"
#include "../undo/InsertLayerUndoAction.h"
#include "../undo/PageBackgroundChangedUndoAction.h"
#include "../undo/PageBackgroundChangedUndoAction.h"
#include "../undo/InsertUndoAction.h"
#include "../undo/RemoveLayerUndoAction.h"
#include "jobs/BlockingJob.h"
#include "jobs/PdfExportJob.h"
#include "jobs/SaveJob.h"
#include "jobs/AutosaveJob.h"
#include "../view/DocumentView.h"
#include "stockdlg/ImageOpenDlg.h"
#include "stockdlg/XojOpenDlg.h"
#include "../gui/TextEditor.h"
#include "../undo/DeleteUndoAction.h"
#include "settings/ButtonConfig.h"
#include "../gui/Cursor.h"
#include "../gui/toolbarMenubar/model/ToolbarModel.h"
#include "../collab/Collaboration.h"
#include "../gui/toolbarMenubar/model/ToolbarData.h"
#include "../gui/toolbarMenubar/ToolMenuHandler.h"

#include <config.h>
#include <glib/gi18n-lib.h>

#include <stdio.h>
#include <string.h>

Control::Control(GladeSearchpath * gladeSearchPath) {
	XOJ_INIT_TYPE(Control);

	this->win = NULL;
	this->recent = new RecentManager();
	this->undoRedo = new UndoRedoHandler(this);
	this->recent->addListener(this);
	this->undoRedo->addUndoRedoListener(this);
	this->isBlocking = false;

	this->gladeSearchPath = gladeSearchPath;

	this->metadata = new MetadataManager();
	this->cursor = new Cursor(this);

	this->lastAction = ACTION_NONE;
	this->lastGroup = GROUP_NOGROUP;
	this->lastEnabled = false;
	this->fullscreen = false;

	String name = String::format("%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, CONFIG_DIR, G_DIR_SEPARATOR,
			SETTINGS_XML_FILE);
	this->settings = new Settings(name);
	this->settings->load();

	this->sidebar = NULL;
	this->searchBar = NULL;

	this->scrollHandler = new ScrollHandler(this);

	this->scheduler = new XournalScheduler();

	this->hiddenFullscreenWidgets = NULL;
	this->sidebarHidden = false;
	this->autosaveTimeout = 0;

	this->defaultWidth = -1;
	this->defaultHeight = -1;

	this->statusbar = NULL;
	this->lbState = NULL;
	this->pgState = NULL;
	this->maxState = 0;

	this->doc = new Document(this);

	// for crashhandling
	setEmergencyDocument(this->doc);

	this->zoom = new ZoomControl();
	this->zoom->setZoom100(this->settings->getDisplayDpi() / 72.0);

	this->toolHandler = new ToolHandler(this, this, this->settings);
	this->toolHandler->loadSettings();

	/**
	 * This is needed to update the previews
	 */
	this->changeTimout = g_timeout_add_seconds(10, (GSourceFunc) checkChangedDocument, this);
	this->changedPages = NULL;

	this->clipboardHandler = NULL;

	this->collaboration = NULL;
	this->dragDropHandler = NULL;
}

Control::~Control() {
	XOJ_CHECK_TYPE(Control);

	if (this->collaboration) {
		delete this->collaboration;
		this->collaboration = NULL;
	}

	g_source_remove(this->changeTimout);
	this->enableAutosave(false);

	deleteLastAutosaveFile("");

	this->scheduler->stop();

	for (GList * l = this->changedPages; l != NULL; l = l->next) {
		XojPage * page = (XojPage *) l->data;
		page->unreference();
	}
	g_list_free(this->changedPages);
	this->changedPages = NULL;

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
	delete this->sidebar;
	this->sidebar = NULL;
	delete this->doc;
	this->doc = NULL;
	delete this->searchBar;
	this->searchBar = NULL;
	delete this->scrollHandler;
	this->scrollHandler = NULL;
	delete this->metadata;
	this->metadata = NULL;
	delete this->cursor;
	this->cursor = NULL;
	delete this->zoom;
	this->zoom = NULL;
	delete this->scheduler;
	this->scheduler = NULL;
	delete this->dragDropHandler;
	this->dragDropHandler = NULL;

	XOJ_RELEASE_TYPE(Control);
}

void Control::renameLastAutosaveFile() {
	XOJ_CHECK_TYPE(Control);

	if (!this->lastAutosaveFilename.isEmpty()) {
		String filename = this->lastAutosaveFilename;
		int pos = filename.lastIndexOf("/") + 1;
		String folder = filename.substring(0, pos);
		String file = filename.substring(pos);
		String renamed = folder + "_tmp_" + file;

		GFile * file1 = g_file_new_for_path(filename.c_str());
		GFile * file2 = g_file_new_for_path(renamed.c_str());
		g_file_move(file1, file2, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
	}
}

void Control::deleteLastAutosaveFile(String newAutosaveFile) {
	XOJ_CHECK_TYPE(Control);

	if (!this->lastAutosaveFilename.isEmpty()) {
		// delete old autosave file
		GFile * file = g_file_new_for_path(this->lastAutosaveFilename.c_str());
		g_file_delete(file, NULL, NULL);
	}
	this->lastAutosaveFilename = newAutosaveFile;
}

bool Control::checkChangedDocument(Control * control) {
	XOJ_CHECK_TYPE_OBJ(control, Control);

	if (!control->doc->tryLock()) {
		// call again later
		return true;
	}
	for (GList * l = control->changedPages; l != NULL; l = l->next) {
		XojPage * page = (XojPage *) l->data;
		int p = control->doc->indexOf(page);
		if (p != -1) {
			control->firePageChanged(p);
		}

		page->unreference();
	}
	g_list_free(control->changedPages);
	control->changedPages = NULL;

	control->doc->unlock();

	// Call again
	return true;
}

void Control::saveSettings() {
	XOJ_CHECK_TYPE(Control);

	this->toolHandler->saveSettings();

	gint width = 0;
	gint height = 0;
	gtk_window_get_size((GtkWindow*) *this->win, &width, &height);

	if (!this->win->isMaximized()) {
		this->settings->setMainWndSize(width, height);
	}
	this->settings->setMainWndMaximized(this->win->isMaximized());

	GtkWidget * sidebar = this->win->get("sidebarContents");
	GtkAllocation alloc;
	gtk_widget_get_allocation(sidebar, &alloc);

	this->settings->setSidebarWidth(alloc.width);
}

void Control::initWindow(MainWindow * win) {
	XOJ_CHECK_TYPE(Control);

	win->setRecentMenu(recent->getMenu());
	selectTool(toolHandler->getToolType());
	this->win = win;
	this->zoom->initZoomHandler(win->getXournal()->getWidget());
	this->sidebar = new Sidebar(win, this);

	updatePageNumbers(0, -1);

	toolHandler->eraserTypeChanged();

	this->searchBar = new SearchBar(this);

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

	XInputUtils::initUtils(win->getWindow());
	XInputUtils::setLeafEnterWorkaroundEnabled(settings->isEnableLeafEnterWorkaround());
}

bool Control::autosaveCallback(Control * control) {
	XOJ_CHECK_TYPE_OBJ(control, Control);

	if (!control->undoRedo->isChangedAutosave()) {
		printf(_("Info: autosave not necessary, nothing changed...\n"));
		// do nothing, nothing changed
		return true;
	} else {
		printf(_("Info: autosave document...\n"));
	}

	AutosaveJob * job = new AutosaveJob(control);
	control->scheduler->addJob(job, JOB_PRIORITY_NONE);
	job->unref();

	return true;
}

void Control::enableAutosave(bool enable) {
	XOJ_CHECK_TYPE(Control);

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
	XOJ_CHECK_TYPE(Control);

	if (this->win == NULL) {
		return;
	}

	this->win->updatePageNumbers(page, this->doc->getPageCount(), pdfPage);
	this->sidebar->selectPageNr(page, pdfPage);

	String file = this->doc->getEvMetadataFilename();
	this->metadata->setInt(file, "page", page);

	int current = this->win->getXournal()->getCurrentPage();
	int count = this->doc->getPageCount();

	fireEnableAction(ACTION_GOTO_FIRST, current != 0);
	fireEnableAction(ACTION_GOTO_BACK, current != 0);
	fireEnableAction(ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE, current != 0);

	fireEnableAction(ACTION_GOTO_PAGE, count > 1);

	fireEnableAction(ACTION_GOTO_NEXT, current < count - 1);
	fireEnableAction(ACTION_GOTO_LAST, current < count - 1);
	fireEnableAction(ACTION_GOTO_NEXT_ANNOTATED_PAGE, current < count - 1);
}

void Control::actionPerformed(ActionType type, ActionGroup group, GdkEvent *event, GtkMenuItem *menuitem,
		GtkToolButton *toolbutton, bool enabled) {
	XOJ_CHECK_TYPE(Control);

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
		cut();
		break;
	case ACTION_COPY:
		copy();
		break;
	case ACTION_PASTE:
		paste();
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
		scrollHandler->scrollToPage(0);
		break;
	case ACTION_GOTO_BACK:
		scrollHandler->goToPreviousPage();
		break;
	case ACTION_GOTO_PAGE:
		gotoPage();
		break;
	case ACTION_GOTO_NEXT:
		scrollHandler->goToNextPage();
		break;
	case ACTION_GOTO_LAST:
		scrollHandler->scrollToPage(this->doc->getPageCount() - 1);
		break;
	case ACTION_GOTO_NEXT_ANNOTATED_PAGE:
		scrollHandler->scrollToAnnotatedPage(true);
		break;
	case ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE:
		scrollHandler->scrollToAnnotatedPage(false);
		break;

		// Menu Journal
	case ACTION_NEW_PAGE_BEFORE:
		insertNewPage(getCurrentPageNo());
		break;
	case ACTION_NEW_PAGE_AFTER:
		insertNewPage(getCurrentPageNo() + 1);
		break;
	case ACTION_NEW_PAGE_AT_END:
		insertNewPage(this->doc->getPageCount());
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
		clearSelection();
		if (enabled) {
			selectTool(TOOL_PEN);
		}
		break;
	case ACTION_TOOL_ERASER:
		clearSelection();
		if (enabled) {
			selectTool(TOOL_ERASER);
		}
		break;

	case ACTION_TOOL_ERASER_STANDARD:
		if (enabled) {
			toolHandler->setEraserType(ERASER_TYPE_DEFAULT);
		}
		break;
	case ACTION_TOOL_ERASER_DELETE_STROKE:
		if (enabled) {
			toolHandler->setEraserType(ERASER_TYPE_DELETE_STROKE);
		}
		break;
	case ACTION_TOOL_ERASER_WHITEOUT:
		if (enabled) {
			toolHandler->setEraserType(ERASER_TYPE_WHITEOUT);
		}
		break;

	case ACTION_TOOL_HILIGHTER:
		clearSelection();
		if (enabled) {
			selectTool(TOOL_HILIGHTER);
		}
		break;
	case ACTION_TOOL_TEXT:
		clearSelection();
		if (enabled) {
			selectTool(TOOL_TEXT);
		}
		break;
	case ACTION_TOOL_IMAGE:
		clearSelection();
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
		clearSelection();
		if (enabled) {
			selectTool(TOOL_VERTICAL_SPACE);
		}
		break;

	case ACTION_TOOL_HAND:
		if (enabled) {
			selectTool(TOOL_HAND);
		}
		break;

	case ACTION_TOOL_DEFAULT:
		if (enabled) {
			selectDefaultTool();
		}
		break;

	case ACTION_RULER:
		setRulerEnabled(enabled);
		break;

	case ACTION_SHAPE_RECOGNIZER:
		setShapeRecognizerEnabled(enabled);
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

	case ACTION_FONT_BUTTON_CHANGED:
		fontChanged();
		break;

	case ACTION_SELECT_FONT:
		if (win) {
			win->getToolMenuHandler()->showFontSelectionDlg();
		}
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
		PageRef p = getCurrentPage();
		if (p.isValid()) {
			p.setSelectedLayerId(this->win->getCurrentLayer());
			this->win->getXournal()->layerChanged(getCurrentPageNo());
			this->win->updateLayerCombobox();

			if (p.isValid()) {
				int layer = p.getSelectedLayerId();
				fireEnableAction(ACTION_DELETE_LAYER, layer > 0);
			}
		}
	}
		break;

	case ACTION_MANAGE_TOOLBAR:
		manageToolbars();
		break;
	case ACTION_CUSTOMIZE_TOOLBAR:
		customizeToolbars();
		break;

	case ACTION_FULLSCREEN:
		enableFullscreen(enabled);
		break;

		// Footer, not really an action, but need an identifier to
	case ACTION_FOOTER_PAGESPIN:
	case ACTION_FOOTER_ZOOM_SLIDER:
		// nothing to do here
		break;

		// Menu collab
	case ACTION_COLLAB_START:
		if (this->collaboration == NULL) {
			this->collaboration = new Collaboration(this);
			this->collaboration->start();
		}
		break;

		// Menu Help
	case ACTION_HELP:
		this->help();
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

// TODO: create help for Xournal++
#if OFFLINE_HELP_ENABLED
#define XOJ_HELP "ghelp:evince"
#else
#define XOJ_HELP "http://library.gnome.org/users/evince/stable/"
#endif

void Control::help() {
	GError * error = NULL;

	gtk_show_uri(gtk_window_get_screen(GTK_WINDOW(this->win->getWindow())), XOJ_HELP, gtk_get_current_event_time(),
			&error);
	if (error) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) getWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK, _("There was an error displaying help: %s"), error->message);
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		g_error_free(error);
	}
}

bool Control::copy() {
	if (this->win && this->win->getXournal()->copy()) {
		return true;
	}
	return this->clipboardHandler->copy();
}

bool Control::cut() {
	if (this->win && this->win->getXournal()->cut()) {
		return true;
	}
	return this->clipboardHandler->cut();
}

bool Control::paste() {
	if (this->win && this->win->getXournal()->paste()) {
		return true;
	}
	return this->clipboardHandler->paste();
}

void Control::clearSelectionEndText() {
	XOJ_CHECK_TYPE(Control);

	clearSelection();
	if (win) {
		win->getXournal()->endTextAllPages();
	}
}

void Control::invokeLater(ActionType type) {
	XOJ_CHECK_TYPE(Control);

	g_idle_add((GSourceFunc) & invokeCallback, new CallbackData(this, type));
}

/**
 * Fire page selected, but first check if the page Number is valid
 *
 * @return the page ID or -1 if the page is not found
 */
int Control::firePageSelected(PageRef page) {
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	int pageId = this->doc->indexOf(page);
	this->doc->unlock();
	if (pageId == -1) {
		return -1;
	}

	DocumentHandler::firePageSelected(pageId);
	return pageId;
}

void Control::firePageSelected(int page) {
	XOJ_CHECK_TYPE(Control);

	DocumentHandler::firePageSelected(page);
}

void Control::manageToolbars() {
	XOJ_CHECK_TYPE(Control);

	ToolbarManageDialog dlg(this->gladeSearchPath, this->win->getToolbarModel());
	dlg.show(GTK_WINDOW(this->win->getWindow()));

	this->win->updateToolbarMenu();

	char * file = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, TOOLBAR_CONFIG,
			NULL);
	this->win->getToolbarModel()->save(file);
	g_free(file);
}

void Control::customizeToolbars() {
	XOJ_CHECK_TYPE(Control);

	g_return_if_fail(this->win != NULL);

	if (this->win->getSelectedToolbar()->isPredefined()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *this->win, GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _(
						"The Toolbarconfiguration \"%s\" is predefined, would you create a copy to edit?"),
				this->win->getSelectedToolbar()->getName().c_str());

		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (res == -8) { // Yes
			ToolbarData * data = new ToolbarData(*this->win->getSelectedToolbar());

			ToolbarModel * model = this->win->getToolbarModel();

			for (int i = 0; i < 100; i++) {
				String id = data->getId() + " Copy";

				if (i != 0) {
					id += " ";
					id += i;
				}

				if (!model->existsId(id)) {
					if (i != 0) {
						data->setName(data->getName() + " " + _("Copy") + " " + i);
					} else {
						data->setName(data->getName() + " " + _("Copy"));
					}
					data->setId(id);
					break;
				}
			}

			model->add(data);
			this->win->toolbarSelected(data);
			this->win->updateToolbarMenu();
		} else {
			return;
		}
	}

	if (!this->dragDropHandler) {
		this->dragDropHandler = new ToolbarDragDropHandler(this);
	}
	this->dragDropHandler->configure();
}

void Control::endDragDropToolbar() {
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler) {
		return;
	}

	this->dragDropHandler->clearToolbarsFromDragAndDrop();
}

void Control::startDragDropToolbar() {
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler) {
		return;
	}

	this->dragDropHandler->prepareToolbarsForDragAndDrop();
}

bool Control::isInDragAndDropToolbar() {
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler) {
		return false;
	}

	return this->dragDropHandler->isInDragAndDrop();
}

void Control::setRulerEnabled(bool enabled) {
	XOJ_CHECK_TYPE(Control);

	if (this->toolHandler->isRuler() == enabled) {
		return;
	}

	this->toolHandler->setRuler(enabled);
	fireActionSelected(GROUP_RULER, enabled ? ACTION_RULER : ACTION_NONE);
	if (enabled) {
		setShapeRecognizerEnabled(false);
	}
}

void Control::setShapeRecognizerEnabled(bool enabled) {
	XOJ_CHECK_TYPE(Control);

	if (this->toolHandler->isShapeRecognizer() == enabled) {
		return;
	}

	this->toolHandler->setShapeRecognizer(enabled);
	fireActionSelected(GROUP_SHAPE_RECOGNIZER, enabled ? ACTION_SHAPE_RECOGNIZER : ACTION_NONE);

	if (enabled) {
		this->resetShapeRecognizer();
		setRulerEnabled(false);
	}
}

void Control::enableFullscreen(bool enabled, bool presentation) {
	XOJ_CHECK_TYPE(Control);

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
					g_warning(
							"Fullscreen: Try to hide \"%s\", but coulden't find it. Wrong entry in ~/" CONFIG_DIR "/" SETTINGS_XML_FILE "?",
							part);
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

void Control::disableSidebarTmp(bool disabled) {
	XOJ_CHECK_TYPE(Control);

	this->sidebar->setTmpDisabled(disabled);
}

void Control::addDefaultPage() {
	XOJ_CHECK_TYPE(Control);

	PageInsertType type = settings->getPageInsertType();

	double width = 0;
	double heigth = 0;

	getDefaultPagesize(width, heigth);

	PageRef page = new XojPage(width, heigth);
	page.setBackgroundColor(settings->getPageBackgroundColor());

	if (PAGE_INSERT_TYPE_PLAIN == type) {
		page.setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (PAGE_INSERT_TYPE_RULED == type) {
		page.setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (PAGE_INSERT_TYPE_GRAPH == type) {
		page.setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else {//PAGE_INSERT_TYPE_LINED or PDF or COPY
		page.setBackgroundType(BACKGROUND_TYPE_LINED);
	}

	this->doc->lock();
	this->doc->addPage(page);
	this->doc->unlock();

	updateDeletePageButton();
}

void Control::getDefaultPagesize(double & width, double & height) {
	XOJ_CHECK_TYPE(Control);

	if (this->defaultHeight < 0) {
		SElement & format = settings->getCustomElement("format");
		format.setComment("paperformat",
				"Available values are: system, A4, Letter, Custom: For custom you have to create the tags width and height.");
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
	XOJ_CHECK_TYPE(Control);

	if (this->win) {
		GtkWidget * w = this->win->get("menuDeletePage");
		gtk_widget_set_sensitive(w, this->doc->getPageCount() > 1);
	}
}

void Control::deletePage() {
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	// don't allow delete pages if we have less than 2 pages,
	// so we can be (more or less) sure there is at least one page.
	if (this->doc->getPageCount() < 2) {
		return;
	}

	int pNr = getCurrentPageNo();
	if (pNr < 0 || pNr > this->doc->getPageCount()) {
		// something went wrong...
		return;
	}

	this->doc->lock();
	PageRef page = doc->getPage(pNr);
	this->doc->unlock();

	// first send event, then delete page...
	firePageDeleted(pNr);

	this->doc->lock();
	doc->deletePage(pNr);
	this->doc->unlock();

	updateDeletePageButton();
	this->undoRedo->addUndoAction(new InsertDeletePageUndoAction(page, pNr, false));

	if (pNr >= this->doc->getPageCount()) {
		pNr = this->doc->getPageCount() - 1;
	}

	scrollHandler->scrollToPage(pNr, 0);
}

void Control::insertNewPage(int position) {
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	PageInsertType type = settings->getPageInsertType();

	if (position > doc->getPageCount()) {
		position = doc->getPageCount();
	}

	double width = 0;
	double height = 0;

	int lastPage = position - 1;
	if (lastPage < 0) {
		getDefaultPagesize(width, height);
	} else {
		PageRef page = doc->getPage(lastPage);

		if (page.isValid()) {
			width = page.getWidth();
			height = page.getHeight();
		} else {
			getDefaultPagesize(width, height);
		}
	}

	PageRef page = new XojPage(width, height);
	page.setBackgroundColor(settings->getPageBackgroundColor());

	if (PAGE_INSERT_TYPE_PLAIN == type) {
		page.setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (PAGE_INSERT_TYPE_LINED == type) {
		page.setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (PAGE_INSERT_TYPE_RULED == type) {
		page.setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (PAGE_INSERT_TYPE_GRAPH == type) {
		page.setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else if (PAGE_INSERT_TYPE_COPY == type) {
		PageRef current = getCurrentPage();
		if (!current.isValid()) { // should not happen, but if you open an invalid file or something like this...
			page.setBackgroundType(BACKGROUND_TYPE_LINED);
		} else {
			BackgroundType bg = current.getBackgroundType();
			page.setBackgroundType(bg);
			if (bg == BACKGROUND_TYPE_PDF) {
				page.setBackgroundPdfPageNr(current.getPdfPageNr());
			} else if (bg == BACKGROUND_TYPE_IMAGE) {
				page.setBackgroundImage(current.getBackgroundImage());
			} else {
				page.setBackgroundColor(current.getBackgroundColor());
			}

			page.setSize(current.getWidth(), current.getHeight());
		}
	} else if (PAGE_INSERT_TYPE_PDF_BACKGROUND == type) {
		if (this->doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("You don't have any PDF pages to select from. Cancel operation,\n"
						"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			return;
		} else {
			this->doc->lock();
			PdfPagesDialog * dlg = new PdfPagesDialog(this->gladeSearchPath, this->doc, this->settings);
			for (int i = 0; i < doc->getPageCount(); i++) {
				PageRef p = doc->getPage(i);
				if (p.getBackgroundType() == BACKGROUND_TYPE_PDF && p.getPdfPageNr() >= 0) {
					dlg->setPageUsed(i);
				}
			}

			dlg->show(GTK_WINDOW(this->win->getWindow()));

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected < 0 || selected >= doc->getPdfPageCount()) {
				// page is automatically deleted
				return;
			}

			// no need to set a type, if we set the page number the type is also set
			page.setBackgroundPdfPageNr(selected);

			XojPopplerPage * p = doc->getPdfPage(selected);
			page.setSize(p->getWidth(), p->getHeight());
			this->doc->unlock();
		}
	}

	insertPage(page, position);
}

void Control::insertPage(PageRef page, int position) {
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	this->doc->insertPage(page, position);
	this->doc->unlock();
	firePageInserted(position);

	getCursor()->updateCursor();

	int visibleHeight = 0;
	scrollHandler->isPageVisible(position, &visibleHeight);

	if (visibleHeight < 10) {
		scrollHandler->scrollToPage(position);
	}
	firePageSelected(position);

	updateDeletePageButton();

	undoRedo->addUndoAction(new InsertDeletePageUndoAction(page, position, true));
}

void Control::addNewLayer() {
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	PageRef p = getCurrentPage();
	if (!p.isValid()) {
		return;
	}

	Layer * l = new Layer();
	p.insertLayer(l, p.getSelectedLayerId());
	if (win) {
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new InsertLayerUndoAction(p, l));
}

void Control::setPageBackground(ActionType type) {
	XOJ_CHECK_TYPE(Control);

	PageRef page = getCurrentPage();

	if (!page.isValid()) {
		return;
	}

	this->doc->lock();
	int pageNr = this->doc->indexOf(page);
	this->doc->unlock();
	if (pageNr == -1) {
		return; // should not happen...
	}

	int origPdfPage = page.getPdfPageNr();
	BackgroundType origType = page.getBackgroundType();
	BackgroundImage origBackgroundImage = page.getBackgroundImage();
	double origW = page.getWidth();
	double origH = page.getHeight();

	if (ACTION_SET_PAPER_BACKGROUND_PLAIN == type) {
		page.setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (ACTION_SET_PAPER_BACKGROUND_LINED == type) {
		page.setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (ACTION_SET_PAPER_BACKGROUND_RULED == type) {
		page.setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (ACTION_SET_PAPER_BACKGROUND_GRAPH == type) {
		page.setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else if (ACTION_SET_PAPER_BACKGROUND_IMAGE == type) {
		this->doc->lock();
		ImagesDialog * dlg = new ImagesDialog(this->gladeSearchPath, this->doc, this->settings);
		this->doc->unlock();

		dlg->show(GTK_WINDOW(this->win->getWindow()));
		BackgroundImage img = dlg->getSelectedImage();
		if (!img.isEmpty()) {
			page.setBackgroundImage(img);
			page.setBackgroundType(BACKGROUND_TYPE_IMAGE);
		} else if (dlg->shouldShowFilechooser()) {

			bool attach = false;
			GFile * file = ImageOpenDlg::show((GtkWindow*) *win, settings, true, &attach);
			if (file == NULL) {
				return;
			}
			char * name = g_file_get_path(file);
			String filename = name;
			g_free(name);
			g_object_unref(file);

			BackgroundImage newImg;
			GError * err = NULL;
			newImg.loadFile(filename, &err);
			newImg.setAttach(attach);
			if (err) {
				GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("This image could not be loaded. Error message: %s"),
						err->message);
				gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				g_error_free(err);
				return;
			} else {
				page.setBackgroundImage(newImg);
				page.setBackgroundType(BACKGROUND_TYPE_IMAGE);
			}
		}

		GdkPixbuf * pixbuf = page.getBackgroundImage().getPixbuf();
		if (pixbuf) {
			page.setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
			firePageSizeChanged(pageNr);
		}

		delete dlg;
	} else if (ACTION_SET_PAPER_BACKGROUND_PDF == type) {
		if (doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("You don't have any PDF pages to select from. Cancel operation,\n"
						"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		} else {
			this->doc->lock();

			PdfPagesDialog * dlg = new PdfPagesDialog(this->gladeSearchPath, this->doc, this->settings);
			for (int i = 0; i < doc->getPageCount(); i++) {
				PageRef p = doc->getPage(i);
				if (p.getBackgroundType() == BACKGROUND_TYPE_PDF && p.getPdfPageNr() >= 0) {
					dlg->setPageUsed(i);
				}
			}

			this->doc->unlock();

			dlg->show(GTK_WINDOW(this->win->getWindow()));

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected < 0 || selected >= doc->getPdfPageCount()) {
				return;
			}

			// no need to set a type, if we set the page number the type is also set
			page.setBackgroundPdfPageNr(selected);
		}
	}

	firePageChanged(pageNr);
	updateBackgroundSizeButton();

	this->undoRedo->addUndoAction(new PageBackgroundChangedUndoAction(page, origType, origPdfPage, origBackgroundImage,
			origW, origH));
}

void Control::gotoPage() {
	GotoDialog * dlg = new GotoDialog(this->gladeSearchPath, this->doc->getPageCount());

	dlg->show(GTK_WINDOW(this->win->getWindow()));
	int page = dlg->getSelectedPage();

	if(page != -1) {
		this->scrollHandler->scrollToPage(page - 1, 0);
	}

	delete dlg;
}

void Control::updateBackgroundSizeButton() {
	XOJ_CHECK_TYPE(Control);

	if (this->win == NULL) {
		return;
	}

	// Update paper color button
	PageRef p = getCurrentPage();
	if (!p.isValid() || this->win == NULL) {
		return;
	}
	BackgroundType bg = p.getBackgroundType();
	GtkWidget * paperColor = win->get("menuJournalPaperColor");
	GtkWidget * pageSize = win->get("menuJournalPaperFormat");

	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg && BACKGROUND_TYPE_RULED != bg
			&& BACKGROUND_TYPE_GRAPH != bg) {
		gtk_widget_set_sensitive(paperColor, false);
	} else {
		gtk_widget_set_sensitive(paperColor, true);
	}

	// PDF page size is defined, you cannot change it
	gtk_widget_set_sensitive(pageSize, bg != BACKGROUND_TYPE_PDF);
}

void Control::paperFormat() {
	XOJ_CHECK_TYPE(Control);

	PageRef page = getCurrentPage();
	if (!page.isValid() || page.getBackgroundType() == BACKGROUND_TYPE_PDF) {
		return;
	}
	clearSelectionEndText();

	FormatDialog * dlg = new FormatDialog(this->gladeSearchPath, settings, page.getWidth(), page.getHeight());
	dlg->show(GTK_WINDOW(this->win->getWindow()));

	double width = dlg->getWidth();
	double height = dlg->getHeight();

	if (width > 0) {
		this->doc->lock();
		this->doc->setPageSize(page, width, height);
		this->doc->unlock();
	}

	delete dlg;
}

void Control::changePageBackgroundColor() {
	XOJ_CHECK_TYPE(Control);

	int pNr = getCurrentPageNo();
	this->doc->lock();
	PageRef p = this->doc->getPage(pNr);
	this->doc->unlock();

	if (!p.isValid()) {
		return;
	}

	clearSelectionEndText();

	BackgroundType bg = p.getBackgroundType();
	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg && BACKGROUND_TYPE_RULED != bg
			&& BACKGROUND_TYPE_GRAPH != bg) {
		return;
	}

	SelectBackgroundColorDialog * dlg = new SelectBackgroundColorDialog(this->gladeSearchPath, this);
	dlg->show(GTK_WINDOW(this->win->getWindow()));
	int color = dlg->getSelectedColor();

	if (color == -2) {
		dlg->showColorchooser();
		color = dlg->getSelectedColor();
	}

	if (color != -1) {
		p.setBackgroundColor(color);
		firePageChanged(pNr);
		settings->setPageBackgroundColor(color);
	}

	delete dlg;
}

void Control::deleteCurrentLayer() {
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	PageRef p = getCurrentPage();
	int pId = getCurrentPageNo();
	if (!p.isValid()) {
		return;
	}

	int lId = p.getSelectedLayerId();
	if (lId < 1) {
		return;
	}
	Layer * l = p.getSelectedLayer();

	p.removeLayer(l);
	if (win) {
		win->getXournal()->layerChanged(pId);
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new RemoveLayerUndoAction(p, l, lId - 1));
	this->resetShapeRecognizer();
}

void Control::calcZoomFitSize() {
	XOJ_CHECK_TYPE(Control);

	if (this->doc && this->win) {
		PageRef p = getCurrentPage();
		if (!p.isValid()) {
			return;
		}
		double width = p.getWidth() + 20;

		GtkAllocation allocation = { 0 };

		gtk_widget_get_allocation(win->getXournal()->getWidget(), &allocation);
		double factor = ((double) allocation.width) / width;
		zoom->setZoomFit(factor);
	}
}

void Control::zoomFit() {
	XOJ_CHECK_TYPE(Control);

	calcZoomFitSize();
	zoom->zoomFit();
}

void Control::setViewTwoPages(bool twoPages) {
	XOJ_CHECK_TYPE(Control);

	settings->setShowTwoPages(twoPages);
	fireActionSelected(GROUP_TWOPAGES, twoPages ? ACTION_VIEW_TWO_PAGES : ACTION_NOT_SELECTED);

	int currentPage = getCurrentPageNo();
	win->getXournal()->layoutPages();
	scrollHandler->scrollToPage(currentPage);
}

void Control::setPageInsertType(PageInsertType type) {
	XOJ_CHECK_TYPE(Control);

	settings->setPageInsertType(type);
	fireActionSelected(GROUP_PAGE_INSERT_TYPE, (ActionType) (type - PAGE_INSERT_TYPE_PLAIN + ACTION_NEW_PAGE_PLAIN));
}

bool Control::invokeCallback(CallbackData * cb) {
	gdk_threads_enter();

	XOJ_CHECK_TYPE_OBJ(cb->control, Control);

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

int Control::getCurrentPageNo() {
	XOJ_CHECK_TYPE(Control);

	if (this->win) {
		return this->win->getXournal()->getCurrentPage();
	}
	return 0;
}

bool Control::searchTextOnPage(const char * text, int p, int * occures, double * top) {
	XOJ_CHECK_TYPE(Control);

	return getWindow()->getXournal()->searchTextOnPage(text, p, occures, top);
}

PageRef Control::getCurrentPage() {
	XOJ_CHECK_TYPE(Control);

	int page = this->win->getXournal()->getCurrentPage();

	this->doc->lock();
	PageRef p = this->doc->getPage(page);
	this->doc->unlock();

	return p;
}

void Control::fileOpened(const char * uri) {
	XOJ_CHECK_TYPE(Control);

	openFile(uri);
	win->updateLayerCombobox();
}

void Control::undoRedoChanged() {
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_UNDO, undoRedo->canUndo());
	fireEnableAction(ACTION_REDO, undoRedo->canRedo());

	win->setUndoDescription(undoRedo->undoDescription());
	win->setRedoDescription(undoRedo->redoDescription());

	updateWindowTitle();
}

void Control::undoRedoPageChanged(PageRef page) {
	XOJ_CHECK_TYPE(Control);

	for (GList * l = this->changedPages; l != NULL; l = l->next) {
		if (l->data == (XojPage *) page) {
			return;
		}
	}

	XojPage * p = (XojPage *) page;
	this->changedPages = g_list_append(this->changedPages, p);
	p->reference();
}

void Control::selectTool(ToolType type) {
	XOJ_CHECK_TYPE(Control);

	toolHandler->selectTool(type);
}

void Control::selectDefaultTool() {
	XOJ_CHECK_TYPE(Control);

	ButtonConfig * cfg = settings->getDefaultButtonConfig();
	cfg->acceptActions(toolHandler);
}

void Control::toolChanged() {
	XOJ_CHECK_TYPE(Control);

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

	fireActionSelected(GROUP_SHAPE_RECOGNIZER, toolHandler->isShapeRecognizer() ? ACTION_SHAPE_RECOGNIZER
			: ACTION_NOT_SELECTED);
	fireActionSelected(GROUP_RULER, toolHandler->isRuler() ? ACTION_RULER : ACTION_NOT_SELECTED);

	getCursor()->updateCursor();

	if (type != TOOL_TEXT) {
		if (win) {
			win->getXournal()->endTextAllPages();
		}
	}
}

void Control::eraserSizeChanged() {
	XOJ_CHECK_TYPE(Control);

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
	XOJ_CHECK_TYPE(Control);

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
	XOJ_CHECK_TYPE(Control);

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
	XOJ_CHECK_TYPE(Control);

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
	XOJ_CHECK_TYPE(Control);

	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR);
	getCursor()->updateCursor();

	if (this->win && toolHandler->getColor() != -1) {
		EditSelection * sel = this->win->getXournal()->getSelection();
		if (sel) {
			UndoAction * undo = sel->setColor(toolHandler->getColor());
			undoRedo->addUndoAction(undo);
		}

		TextEditor * edit = getTextEditor();


		if(this->toolHandler->getToolType() == TOOL_TEXT && edit != NULL) {
			UndoAction * undo = edit->setColor(toolHandler->getColor());
			undoRedo->addUndoAction(undo);
		}
	}
}

void Control::setCustomColorSelected() {
	XOJ_CHECK_TYPE(Control);

	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR_CUSTOM);
}

void Control::showSettings() {
	XOJ_CHECK_TYPE(Control);

	bool xeventEnabled = settings->isUseXInput();
	int selectionColor = settings->getSelectionColor();
	bool allowScrollOutside = settings->isAllowScrollOutsideThePage();
	bool bigCursor = settings->isShowBigCursor();

	SettingsDialog * dlg = new SettingsDialog(this->gladeSearchPath, settings);
	dlg->show(GTK_WINDOW(this->win->getWindow()));

	if (xeventEnabled != settings->isUseXInput()) {
		win->getXournal()->updateXEvents();
	}

	if (selectionColor != settings->getSelectionColor()) {
		win->getXournal()->forceUpdatePagenumbers();
	}

	if (allowScrollOutside != settings->isAllowScrollOutsideThePage()) {
		int currentPage = getCurrentPageNo();
		win->getXournal()->layoutPages();
		scrollHandler->scrollToPage(currentPage);
	}

	if (bigCursor != settings->isShowBigCursor()) {
		getCursor()->updateCursor();
	}

	win->updateScrollbarSidebarPosition();

	enableAutosave(settings->isAutosaveEnabled());

	this->zoom->setZoom100(settings->getDisplayDpi() / 72.0);
	delete dlg;
}

bool Control::newFile() {
	XOJ_CHECK_TYPE(Control);

	if (!this->close()) {
		return false;
	}

	Document newDoc(this);

	this->doc->lock();
	*doc = newDoc;
	this->doc->unlock();

	PageInsertType type = settings->getPageInsertType();
	if (type != PAGE_INSERT_TYPE_PLAIN && type != PAGE_INSERT_TYPE_LINED && type != PAGE_INSERT_TYPE_RULED && type
			!= PAGE_INSERT_TYPE_GRAPH) {
		type = PAGE_INSERT_TYPE_LINED;
	}

	addDefaultPage();

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

	fileLoaded();

	return true;
}

bool Control::openFile(String filename, int scrollToPage) {
	XOJ_CHECK_TYPE(Control);

	if (!this->close()) {
		return false;
	}

	if (filename.isEmpty()) {
		bool attachPdf = false;
		filename = XojOpenDlg::showOpenDialog((GtkWindow *) *win, this->settings, false, attachPdf);

		printf("Filename: %s\n", filename.c_str());

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

				this->doc->lock();
				this->doc->clearDocument();
				*this->doc = *tmp;
				this->doc->unlock();

				fileLoaded(scrollToPage);
				return true;
			}
		}

		bool an = annotatePdf(filename, false, false);
		fileLoaded(scrollToPage);
		return an;
	}

	Document * tmp = h.loadDocument(filename);
	if (!tmp && h.isAttachedPdfMissing() || !h.getMissingPdfFilename().isEmpty()) {
		// give the user a second chance to select a new PDF file, or to discard the PDF


		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, h.isAttachedPdfMissing() ? _(
						"The attached background PDF could not be found.") : _("The background PDF could not be found."));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another PDF"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Remove PDF Background"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 3);
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (res == 2) { // remove PDF background
			h.removePdfBackground();
			tmp = h.loadDocument(filename);
		} else if (res == 1) { // select another PDF background
			bool attachToDocument = false;
			String pdfFilename = XojOpenDlg::showOpenDialog((GtkWindow*) *win, this->settings, true, attachToDocument);
			if (!pdfFilename.isEmpty()) {
				h.setPdfReplacement(pdfFilename, attachToDocument);
				tmp = h.loadDocument(filename);
			}
		}
	}

	if (!tmp) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK, _("Error opening file '%s'\n%s"), filename.c_str(), h.getLastError().c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		fileLoaded(scrollToPage);
		return false;
	} else {
		this->doc->lock();
		this->doc->clearDocument();
		*this->doc = *tmp;
		this->doc->unlock();
	}

	fileLoaded(scrollToPage);
	return true;
}

void Control::fileLoaded(int scrollToPage) {
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	String file = this->doc->getEvMetadataFilename();
	this->doc->unlock();

	if (!file.isEmpty()) {
		double zoom = 1;
		if (this->metadata->getDouble(file, "zoom", zoom)) {
			this->zoom->setZoom(zoom);
		} else {
			this->zoom->zoomFit();
		}

		int scrollPageMetadata = 0;
		if (scrollToPage >= 0) {
			scrollHandler->scrollToPage(scrollToPage);
		} else if (this->metadata->getInt(file, "page", scrollPageMetadata)) {
			scrollHandler->scrollToPage(scrollPageMetadata);
		}
		recent->addRecentFileFilename(file.c_str());
	} else {
		this->zoom->zoomFit();
	}

	updateWindowTitle();
	win->updateLayerCombobox();
	win->getXournal()->forceUpdatePagenumbers();
	getCursor()->updateCursor();
	updateDeletePageButton();
}

bool Control::annotatePdf(String filename, bool attachPdf, bool attachToDocument) {
	XOJ_CHECK_TYPE(Control);

	if (!this->close()) {
		return false;
	}

	if (filename.isEmpty()) {
		filename = XojOpenDlg::showOpenDialog((GtkWindow*) *win, this->settings, true, attachToDocument);
		if (filename.isEmpty()) {
			return false;
		}
	}

	getCursor()->setCursorBusy(true);

	bool res = this->doc->readPdf(filename, true, attachToDocument);

	if (res) {
		int page = 0;

		this->recent->addRecentFileFilename(filename.c_str());

		this->doc->lock();
		String file = this->doc->getEvMetadataFilename();
		this->doc->unlock();

		this->metadata->getInt(file, "page", page);
		this->scrollHandler->scrollToPage(page);
	} else {
		this->doc->lock();
		String errMsg = doc->getLastErrorMsg();
		this->doc->unlock();

		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *win, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK, _("Error annotate PDF file '%s'\n%s"), filename.c_str(), errMsg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	getCursor()->setCursorBusy(false);

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	getCursor()->updateCursor();

	return true;
}

void Control::print() {
	XOJ_CHECK_TYPE(Control);

	PrintHandler print;
	this->doc->lock();
	print.print(this->doc, getCurrentPageNo());
	this->doc->unlock();
}

void Control::block(const char * name) {
	XOJ_CHECK_TYPE(Control);

	if (this->isBlocking) {
		return;
	}

	// Disable all gui Control, to get full control over the application
	win->setControlTmpDisabled(true);
	getCursor()->setCursorBusy(true);
	disableSidebarTmp(true);

	this->statusbar = this->win->get("statusbar");
	this->lbState = GTK_LABEL(this->win->get("lbState"));
	this->pgState = GTK_PROGRESS_BAR(this->win->get("pgState"));

	gtk_label_set_text(this->lbState, name);
	gtk_widget_show(this->statusbar);

	this->maxState = 100;
	this->isBlocking = true;

}

void Control::unblock() {
	XOJ_CHECK_TYPE(Control);

	if (!this->isBlocking) {
		return;
	}

	this->win->setControlTmpDisabled(false);
	getCursor()->setCursorBusy(false);
	disableSidebarTmp(false);

	gtk_widget_hide(this->statusbar);

	this->isBlocking = false;
}

void Control::setMaximumState(int max) {
	XOJ_CHECK_TYPE(Control);

	this->maxState = max;
}

void Control::setCurrentState(int state) {
	XOJ_CHECK_TYPE(Control);

	gtk_progress_bar_set_fraction(this->pgState, (double) state / this->maxState);
}

bool Control::save(bool synchron) {
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	String filename = this->doc->getFilename();
	this->doc->unlock();

	if (filename.isEmpty()) {
		if (!showSaveDialog()) {
			return false;
		}
	}

	SaveJob * job = new SaveJob(this);
	bool result = true;
	if (synchron) {
		result = job->save();
		unblock();
	} else {
		this->scheduler->addJob(job, JOB_PRIORITY_URGENT);
	}
	job->unref();

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
	XOJ_CHECK_TYPE(Control);

	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Save File"), (GtkWindow*) *win, GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter * filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	String saveFilename = "";

	this->doc->lock();
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
	this->doc->unlock();

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return false;
	}

	char * name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	String filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	this->doc->lock();

	this->metadata->move(this->doc->getFilename(), filename);
	this->doc->setFilename(filename);
	this->doc->unlock();

	return true;
}

void Control::updateWindowTitle() {
	XOJ_CHECK_TYPE(Control);

	String title = "";

	this->doc->lock();
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
	this->doc->unlock();

	title += " - Xournal";

	gtk_window_set_title((GtkWindow*) *win, title.c_str());
}

void Control::exportAsPdf() {
	XOJ_CHECK_TYPE(Control);

	PdfExportJob * job = new PdfExportJob(this);
	if (job->showFilechooser()) {
		this->scheduler->addJob(job, JOB_PRIORITY_NONE);
	} else {
		// The job blocked, so we have to unblock, because the job unblocks only after run
		unblock();
	}
	job->unref();
}

void Control::exportAs() {
	XOJ_CHECK_TYPE(Control);

	ExportHandler handler;

	handler.runExportWithDialog(this->gladeSearchPath, this->settings, this->doc, this, getCurrentPageNo());
}

void Control::saveAs() {
	XOJ_CHECK_TYPE(Control);

	if (!showSaveDialog()) {
		return;
	}
	this->doc->lock();
	String filename = doc->getFilename();
	this->doc->unlock();

	if (filename.isEmpty()) {
		return;
	}

	// no lock needed, this is an uncritical operation
	this->doc->setCreateBackupOnSave(false);
	save();
}

void Control::quit() {
	XOJ_CHECK_TYPE(Control);

	if (!this->close(true)) {
		return;
	}

	this->scheduler->lock();

	settings->save();

	this->scheduler->removeAllJobs();
	this->scheduler->unlock();
	gtk_main_quit();
}

bool Control::close(bool destroy) {
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();

	if (undoRedo->isChanged()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, _("This document is not saved yet."));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Save"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Discard"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 3);
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		// save
		if (res == 1) {
			if (this->save(true)) {
				return true;
			} else {
				// if not saved cancel, else close
				return false;
			}
		}

		// cancel or closed
		if (res != 2) {// 2 = discard
			return false;
		}
	}

	undoRedo->clearContents();

	this->doc->lock();
	this->doc->clearDocument(destroy);
	this->doc->unlock();

	updateWindowTitle();
	return true;
}

void Control::resetShapeRecognizer() {
	XOJ_CHECK_TYPE(Control);

	if (this->win) {
		this->win->getXournal()->resetShapeRecognizer();
	}
}

void Control::showAbout() {
	XOJ_CHECK_TYPE(Control);

	AboutDialog dlg(this->gladeSearchPath);
	dlg.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::clipboardCutCopyEnabled(bool enabled) {
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_CUT, enabled);
	fireEnableAction(ACTION_COPY, enabled);
}

void Control::clipboardPasteEnabled(bool enabled) {
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_PASTE, enabled);
}

void Control::clipboardPasteText(String text) {
	XOJ_CHECK_TYPE(Control);

	Text * t = new Text();
	t->setText(text);
	t->setFont(settings->getFont());
	t->setColor(toolHandler->getColor());

	clipboardPaste(t);
}

void Control::clipboardPasteImage(GdkPixbuf * img) {
	XOJ_CHECK_TYPE(Control);

	Image * image = new Image();
	image->setImage(img);

	int width = gdk_pixbuf_get_width(img);
	int height = gdk_pixbuf_get_height(img);

	image->setWidth(width);
	image->setHeight(height);

	clipboardPaste(image);
}

void Control::clipboardPaste(Element * e) {
	XOJ_CHECK_TYPE(Control);

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

	this->doc->lock();
	PageRef page = this->doc->getPage(pageNr);
	Layer * layer = page.getSelectedLayer();
	win->getXournal()->getPasteTarget(x, y);

	double width = e->getElementWidth();
	double height = e->getElementHeight();

	e->setX(x - width / 2);
	e->setY(y - height / 2);
	layer->addElement(e);

	this->doc->unlock();

	undoRedo->addUndoAction(new InsertUndoAction(page, layer, e, view));

	EditSelection * selection = new EditSelection(this->undoRedo, e, view, page);

	win->getXournal()->setSelection(selection);
}

void Control::clipboardPasteXournal(ObjectInputStream & in) {
	XOJ_CHECK_TYPE(Control);

	int pNr = getCurrentPageNo();
	if (pNr == -1 && win != NULL) {
		return;
	}

	this->doc->lock();
	PageRef page = this->doc->getPage(pNr);

	PageView * view = win->getXournal()->getViewFor(pNr);

	if (!view || !page) {
		this->doc->unlock();
		return;
	}

	EditSelection * selection = NULL;
	Element * element = NULL;
	try {
		String version = in.readString();
		if (version != PACKAGE_STRING) {
			g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PACKAGE_STRING);
		}

		selection = new EditSelection(this->undoRedo, page, view);
		in >> selection;

		// document lock not needed anymore, because we don't change the document, we only change the selection
		this->doc->unlock();

		int count = in.readInt();

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

		win->getXournal()->setSelection(selection);

	} catch (std::exception & e) {
		g_warning("could not paste, Exception occurred: %s", e.what());
		Stacktrace::printStracktrace();

		// cleanup
		if (element) {
			delete element;
		}

		if (selection) {
			ListIterator<Element *> it = selection->getElements();

			while (it.hasNext()) {
				delete it.next();
			}

			delete selection;
		}
	}
}

void Control::deleteSelection() {
	XOJ_CHECK_TYPE(Control);

	if (win) {
		win->getXournal()->deleteSelection();
	}
}

void Control::clearSelection() {
	XOJ_CHECK_TYPE(Control);

	if (this->win) {
		this->win->getXournal()->clearSelection();
	}
}

void Control::setClipboardHandlerSelection(EditSelection * selection) {
	XOJ_CHECK_TYPE(Control);

	if (this->clipboardHandler) {
		this->clipboardHandler->setSelection(selection);
	}
}

void Control::setCopyPasteEnabled(bool enabled) {
	XOJ_CHECK_TYPE(Control);

	this->clipboardHandler->setCopyPasteEnabled(enabled);
}

void Control::setToolSize(ToolSize size) {
	XOJ_CHECK_TYPE(Control);

	EditSelection * sel = NULL;
	if (this->win) {
		sel = this->win->getXournal()->getSelection();
	}

	if (sel) {
		UndoAction * undo = sel->setSize(size, toolHandler->getToolThickness(TOOL_PEN), toolHandler->getToolThickness(
				TOOL_HILIGHTER), toolHandler->getToolThickness(TOOL_ERASER));
		if (undo) {
			undoRedo->addUndoAction(undo);
		}
	}
	this->toolHandler->setSize(size);
}

void Control::fontChanged() {
	XOJ_CHECK_TYPE(Control);

	XojFont font = win->getFontButtonFont();
	settings->setFont(font);

	EditSelection * sel = NULL;
	if (this->win) {
		sel = this->win->getXournal()->getSelection();
	}
	if (sel) {
		UndoAction * undo = sel->setFont(font);
		if (undo) {
			undoRedo->addUndoAction(undo);
		}
	}

	TextEditor * editor = getTextEditor();
	if (editor) {
		editor->setFont(font);
	}
}

/**
 * GETTER / SETTER
 */

UndoRedoHandler * Control::getUndoRedoHandler() {
	XOJ_CHECK_TYPE(Control);

	return this->undoRedo;
}

ZoomControl * Control::getZoomControl() {
	XOJ_CHECK_TYPE(Control);

	return this->zoom;
}

Cursor * Control::getCursor() {
	XOJ_CHECK_TYPE(Control);

	return this->cursor;
}

RecentManager * Control::getRecentManager() {
	XOJ_CHECK_TYPE(Control);

	return this->recent;
}

Document * Control::getDocument() {
	XOJ_CHECK_TYPE(Control);

	return this->doc;
}

ToolHandler * Control::getToolHandler() {
	XOJ_CHECK_TYPE(Control);

	return this->toolHandler;
}

XournalScheduler * Control::getScheduler() {
	XOJ_CHECK_TYPE(Control);

	return this->scheduler;
}

MainWindow * Control::getWindow() {
	XOJ_CHECK_TYPE(Control);

	return this->win;
}

bool Control::isFullscreen() {
	XOJ_CHECK_TYPE(Control);

	return this->fullscreen;
}

TextEditor * Control::getTextEditor() {
	XOJ_CHECK_TYPE(Control);

	if (this->win) {
		return this->win->getXournal()->getTextEditor();
	}
	return NULL;
}

GladeSearchpath * Control::getGladeSearchPath() {
	XOJ_CHECK_TYPE(Control);

	return this->gladeSearchPath;
}

Settings * Control::getSettings() {
	XOJ_CHECK_TYPE(Control);

	return settings;
}

ScrollHandler * Control::getScrollHandler() {
	XOJ_CHECK_TYPE(Control);

	return this->scrollHandler;
}

MetadataManager * Control::getMetadataManager() {
	XOJ_CHECK_TYPE(Control);

	return this->metadata;
}

Sidebar * Control::getSidebar() {
	XOJ_CHECK_TYPE(Control);

	return this->sidebar;
}

