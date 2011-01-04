#include "Control.h"
#include <gtk/gtk.h>
#include "../gui/AboutDialog.h"
#include "../gui/ExportDialog.h"
#include "../gui/SettingsDialog.h"
#include "../gui/PdfPagesDialog.h"
#include "../gui/SelectBackgroundColorDialog.h"
#include "../cfg.h"
#include "LoadHandler.h"
#include "../gettext.h"
#include "ev-metadata-manager.h"
#include "../pdf/PdfExport.h"

#include <stdio.h>
#include <string.h>

#include "SaveHandler.h"

// TODO: Check for error log on startup


/*
 // the paper sizes dialog

 GtkWidget *papersize_dialog;
 int papersize_std, papersize_unit;
 double papersize_width, papersize_height;
 gboolean papersize_need_init, papersize_width_valid, papersize_height_valid;

 #define STD_SIZE_A4 0
 #define STD_SIZE_A4R 1
 #define STD_SIZE_LETTER 2
 #define STD_SIZE_LETTER_R 3
 #define STD_SIZE_CUSTOM 4

 double unit_sizes[4] = {28.346, 72., 72./DISPLAY_DPI_DEFAULT, 1.};
 double std_widths[STD_SIZE_CUSTOM] =  {595.27, 841.89, 612., 792.};
 double std_heights[STD_SIZE_CUSTOM] = {841.89, 595.27, 792., 612.};
 double std_units[STD_SIZE_CUSTOM] = {UNIT_CM, UNIT_CM, UNIT_IN, UNIT_IN};

 void
 on_journalPaperSize_activate           (GtkMenuItem     *menuitem,
 gpointer         user_data)
 {
 int i, response;
 struct Page *pg;
 GList *pglist;

 end_text();
 papersize_dialog = create_papersizeDialog();
 papersize_width = ui.cur_page->width;
 papersize_height = ui.cur_page->height;
 papersize_unit = ui.default_unit;
 unit_sizes[UNIT_PX] = 1./DEFAULT_ZOOM;
 //  if (ui.cur_page->bg->type == BG_PIXMAP) papersize_unit = UNIT_PX;
 papersize_std = STD_SIZE_CUSTOM;
 for (i=0;i<STD_SIZE_CUSTOM;i++)
 if (fabs(papersize_width - std_widths[i])<0.1 &&
 fabs(papersize_height - std_heights[i])<0.1)
 { papersize_std = i; papersize_unit = std_units[i]; }
 papersize_need_init = TRUE;
 papersize_width_valid = papersize_height_valid = TRUE;

 gtk_widget_show(papersize_dialog);
 on_comboStdSizes_changed(GTK_COMBO_BOX(g_object_get_data(
 G_OBJECT(papersize_dialog), "comboStdSizes")), NULL);
 gtk_dialog_set_default_response(GTK_DIALOG(papersize_dialog), GTK_RESPONSE_OK);

 response = gtk_dialog_run(GTK_DIALOG(papersize_dialog));
 gtk_widget_destroy(papersize_dialog);
 if (response != GTK_RESPONSE_OK) return;

 pg = ui.cur_page;
 for (pglist = journal.pages; pglist!=NULL; pglist = pglist->next) {
 if (ui.bg_apply_all_pages) pg = (struct Page *)pglist->data;
 prepare_new_undo();
 if (ui.bg_apply_all_pages) {
 if (pglist->next!=NULL) undo->multiop |= MULTIOP_CONT_REDO;
 if (pglist->prev!=NULL) undo->multiop |= MULTIOP_CONT_UNDO;
 }
 undo->type = ITEM_PAPER_RESIZE;
 undo->page = pg;
 undo->val_x = pg->width;
 undo->val_y = pg->height;
 if (papersize_width_valid) pg->width = papersize_width;
 if (papersize_height_valid) pg->height = papersize_height;
 make_page_clipbox(pg);
 update_canvas_bg(pg);
 if (!ui.bg_apply_all_pages) break;
 }
 do_switch_page(ui.pageno, TRUE, TRUE);
 }


 */
Control::Control() {
	this->win = NULL;
	recent = new RecentManager();
	undoRedo = new UndoRedoHandler(this);
	recent->addListener(this);
	undoRedo->addUndoRedoListener(this);

	background = new BackgroundThreadHandler(this);

	lastAction = ACTION_NONE;
	lastGroup = GROUP_NOGROUP;
	lastEnabled = false;
	fullscreen = false;

	gchar * filename = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S,
			SETTINGS_XML_FILE, NULL);
	String name(filename, true);
	settings = new Settings(name);
	settings->load();

	sidebar = NULL;
	searchBar = NULL;

	this->hiddenFullscreenWidgets = NULL;

	// Init Metadata Manager
	ev_metadata_manager_init();

	doc = new Document(this);

	this->zoom = new ZoomControl();
	this->zoom->setZoom100(settings->getDisplayDpi() / 72.0);

	this->toolHandler = new ToolHandler(this, settings);
	this->toolHandler->loadSettings();

	this->cursor = new Cursor(this);

	this->changeTimout = g_timeout_add_seconds(10, (GSourceFunc) checkChangedDocument, this);
	this->changedPages = NULL;
}

Control::~Control() {
	g_source_remove(this->changeTimout);

	delete recent;
	recent = NULL;
	delete undoRedo;
	undoRedo = NULL;
	delete settings;
	settings = NULL;
	delete toolHandler;
	toolHandler = NULL;
	delete doc;
	doc = NULL;
	delete sidebar;
	sidebar = NULL;
	delete searchBar;
	searchBar = NULL;
	delete background;
	background = NULL;
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
	gtk_window_get_size(GTK_WINDOW(win->getWindow()), &width, &height);

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
}

ZoomControl * Control::getZoomControl() {
	return zoom;
}

Cursor * Control::getCursor() {
	return cursor;
}

void Control::updatePageNumbers(int page, int pdfPage) {
	this->win->updatePageNumbers(page, doc->getPageCount(), pdfPage);
	this->sidebar->selectPageNr(page);

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

void xxxxx() {
	printf("This action is not yet implemented\n");
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

void Control::actionPerformed(ActionType type, ActionGroup group, GdkEvent *event, GtkMenuItem *menuitem,
		GtkToolButton *toolbutton, bool enabled) {

	// Because GTK sends events if I change radio items etc.
	if (shouldIgnorAction(type, group, enabled)) {
		return;
	}

	switch (type) {
	// Menu File
	case ACTION_NEW:
		newFile();
		break;
	case ACTION_OPEN:
		openFile();
		break;
	case ACTION_ANNOTATE_PDF:
		annotatePdf(NULL, false);
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
		// TODO: not implemented, but menupoint is hidden...
		break;
	case ACTION_PRINT:
		xxxxx();
		break;
	case ACTION_QUIT:
		quit();
		break;

		// Menu Edit
	case ACTION_UNDO:
		undoRedo->undo();
		break;
	case ACTION_REDO:
		undoRedo->redo();
		break;
	case ACTION_CUT:
		win->getXournal()->cut();
		break;
	case ACTION_COPY:
		win->getXournal()->copy();
		break;
	case ACTION_PASTE:
		win->getXournal()->paste();
		break;
	case ACTION_SEARCH:
		searchBar->showSearchBar(true);
		break;
	case ACTION_DELETE:
		win->getXournal()->actionDelete();
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
		xxxxx();
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
		setPageBackground(type);
		break;

	case ACTION_NEW_PAGE_PLAIN:
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_PLAIN);
		}
		break;
	case ACTION_NEW_PAGE_LINED:
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_LINED);
		}
		break;
	case ACTION_NEW_PAGE_RULED:
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_RULED);
		}
		break;
	case ACTION_NEW_PAGE_GRAPH:
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_GRAPH);
		}
		break;
	case ACTION_NEW_PAGE_COPY:
		if (enabled) {
			setPageInsertType(PAGE_INSERT_TYPE_COPY);
		}
		break;
	case ACTION_NEW_PAGE_PDF_BACKGROUND:
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
		break;
	case ACTION_SHAPE_RECOGNIZER:
		this->toolHandler->setShapeRecognizer(enabled);
		fireActionSelected(GROUP_SHAPE_RECOGNIZER, enabled ? ACTION_SHAPE_RECOGNIZER : ACTION_NONE);
		break;
	case ACTION_SIZE_VERY_THIN:
		if (enabled) {
			this->toolHandler->setSize(TOOL_SIZE_VERY_FINE);
		}
		break;
	case ACTION_SIZE_FINE:
		if (enabled) {
			this->toolHandler->setSize(TOOL_SIZE_FINE);
		}
		break;
	case ACTION_SIZE_MEDIUM:
		if (enabled) {
			this->toolHandler->setSize(TOOL_SIZE_MEDIUM);
		}
		break;
	case ACTION_SIZE_THICK:
		if (enabled) {
			this->toolHandler->setSize(TOOL_SIZE_THICK);
		}
		break;
	case ACTION_SIZE_VERY_THICK:
		if (enabled) {
			this->toolHandler->setSize(TOOL_SIZE_VERY_THICK);
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
		xxxxx();
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
		// TODO: implement help
		xxxxx();
		break;
	case ACTION_ABOUT:
		showAbout();
		break;

	default:
		g_warning("Unhandled action event: %i / %i", type, group);
	}

	if (type >= ACTION_TOOL_PEN && type <= ACTION_TOOL_HAND) {
		ActionType at = (ActionType) (toolHandler->getToolType() - TOOL_PEN + ACTION_TOOL_PEN);
		if (type == at && !enabled) {
			fireActionSelected(GROUP_TOOL, at);
		}
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

void Control::enableFullscreen(bool enabled) {
	if (enabled) {
		gtk_window_fullscreen((GtkWindow*) *win);

		String e = settings->getFullscreenHideElements();
		char * str = g_strdup(e.c_str());

		char * part = strtok(str, ",");

		while (part) {
			GtkWidget * w = win->get(part);
			if (w == NULL) {
				g_warning("Fullscreen: Try to hide \"%s\", but coulden't find it. Wrong entry in ~/" CONFIG_DIR "/" SETTINGS_XML_FILE "?", part);
			} else {
				if (gtk_widget_get_visible(w)) {
					gtk_widget_hide(w);
					this->hiddenFullscreenWidgets = g_list_append(this->hiddenFullscreenWidgets, w);
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
		g_list_free(this->hiddenFullscreenWidgets);
		this->hiddenFullscreenWidgets = NULL;
	}

	fireActionSelected(GROUP_FULLSCREEN, enabled ? ACTION_FULLSCREEN : ACTION_NONE);
	this->fullscreen = enabled;
}

void Control::addDefaultPage() {
	PageInsertType type = settings->getPageInsertType();

	XojPage * page = new XojPage();
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

void Control::updateDeletePageButton() {
	if (this->win) {
		GtkWidget * w = this->win->get("menuDeletePage");
		gtk_widget_set_sensitive(w, doc->getPageCount() > 1);
	}
}

void Control::deletePage() {
	// don't allow delete pages if we have less than 2 pages, so we can be (more or less) sure there is at least one page.
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
	PageInsertType type = settings->getPageInsertType();

	if (position > doc->getPageCount()) {
		position = doc->getPageCount();
	}

	XojPage * page = new XojPage();
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
				// TODO: copy background image!
			} else {
				page->setBackgroundColor(current->getBackgroundColor());
			}
		}
	} else if (PAGE_INSERT_TYPE_PDF_BACKGROUND == type) {
		if (doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new(GTK_WINDOW(win->getWindow()), GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("You don't have any PDF pages to select from. Cancel operation,\n"
							"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			// delete page
			page->unreference();
			return;
		} else {
			PdfPagesDialog * dlg = new PdfPagesDialog(this->doc);
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
		}
	}

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

	if(page == NULL) {
		return;
	}

	if (ACTION_SET_PAPER_BACKGROUND_PLAIN == type) {
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	} else if (ACTION_SET_PAPER_BACKGROUND_LINED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	} else if (ACTION_SET_PAPER_BACKGROUND_RULED == type) {
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	} else if (ACTION_SET_PAPER_BACKGROUND_GRAPH == type) {
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	} else if (ACTION_SET_PAPER_BACKGROUND_IMAGE == type) {
		// TODO: copy background image!
	} else if (ACTION_SET_PAPER_BACKGROUND_PDF == type) {
		if (doc->getPdfPageCount() == 0) {
			GtkWidget * dialog = gtk_message_dialog_new(GTK_WINDOW(win->getWindow()), GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					_("You don't have any PDF pages to select from. Cancel operation,\n"
							"Please select another background type: Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		} else {
			PdfPagesDialog * dlg = new PdfPagesDialog(this->doc);
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

	firePageChanged(doc->indexOf(page));
}

void Control::changePageBackgroundColor() {
	int pNr = getCurrentPageNo();
	XojPage * p = doc->getPage(pNr);

	if (p == NULL) {
		return;
	}

	BackgroundType bg = p->getBackgroundType();
	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg && BACKGROUND_TYPE_RULED != bg
			&& BACKGROUND_TYPE_GRAPH != bg) {
		GtkWidget * dialog = gtk_message_dialog_new(GTK_WINDOW(win->getWindow()), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("You cannot set the background color for this page.\n"
						"The page type has to be one of this:\n"
						"- Plain\n"
						"- Line\n"
						"- Ruled\n"
						"- Graph"));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

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
		GtkWidget * scrol = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(w));

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

	fireActionSelected(GROUP_SHAPE_RECOGNIZER, toolHandler->isShapeRecognizer() ? ACTION_SHAPE_RECOGNIZER
			: ACTION_NOT_SELECTED);
	fireActionSelected(GROUP_RULER, toolHandler->isRuler() ? ACTION_RULER : ACTION_NOT_SELECTED);

	cursor->updateCursor();

	if (type != TOOL_SELECT_RECT && type != TOOL_SELECT_REGION) {
		if (win) {
			win->clearSelection();
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
}

void Control::setCustomColorSelected() {
	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR_CUSTOM);
}

void Control::showSettings() {
	bool xeventEnabled = settings->isUseXInput();

	SettingsDialog * dlg = new SettingsDialog(settings);
	dlg->show();

	if (xeventEnabled != settings->isUseXInput()) {
		win->getXournal()->updateXEvents();
	}

	win->updateScrollbarSidebarPosition();

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
	if (type != PAGE_INSERT_TYPE_PLAIN && type != PAGE_INSERT_TYPE_LINED && type != PAGE_INSERT_TYPE_RULED && type
			!= PAGE_INSERT_TYPE_GRAPH) {
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
	String filename;

	GtkWidget *dialog;
	GtkFileFilter *filterAll;
	GtkFileFilter *filterPdf;
	GtkFileFilter *filterXoj;
	GtkFileFilter *filterSupported;

	char *name;
	int file_domain;
	gboolean success;

	dialog = gtk_file_chooser_dialog_new(_("Open File"), GTK_WINDOW(this->win->getWindow()),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");

	filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");

	filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");

	filterSupported = gtk_file_filter_new();
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
	name = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));

	if (attachOpt) {
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
	}

	filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
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

	if (v + scroll < max && v + scroll >= 0) {
		gtk_adjustment_set_value(adj, v + scroll);
	}
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
			if (h.loadDocument(f, doc)) {
				fileLoaded();
				return true;
			}
		}

		bool an = annotatePdf(filename, false);
		fileLoaded();
		return an;
	}

	if (!h.loadDocument(filename, doc)) {
		GtkWidget * dialog = gtk_message_dialog_new(GTK_WINDOW(win->getWindow()), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error opening file '%s'\n%s"), filename.c_str(),
				h.getLastError().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		fileLoaded();
		return false;
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
	}

	recent->addRecentFile(file);

	fileLoaded();
	return true;
	// TODO: handle if PDF not found!
}

void Control::fileLoaded() {
	updateDocName();
	win->updateLayerCombobox();
	win->getXournal()->forceUpdatePagenumbers();
	cursor->updateCursor();
	updateDeletePageButton();
}

bool Control::annotatePdf(String filename, bool attachPdf) {
	if (!this->close()) {
		return false;
	}

	if (filename.isEmpty()) {
		bool attachPdf = false;
		filename = showOpenDialog(true, attachPdf);
		if (filename.isEmpty()) {
			return false;
		}
	}

	cursor->setCursorBusy(true);
	if (doc->readPdf(filename, true)) {
		int page = 0;
		GValue value = { 0 };

		recent->addRecentFile(filename.c_str());

		const char * file = doc->getEvMetadataFilename();
		if (file && ev_metadata_manager_get(file, "page", &value, TRUE) && G_VALUE_TYPE(&value) == G_TYPE_INT) {
			page = g_value_get_int(&value);
		}
		scrollToPage(page);
	} else {
		GtkWidget * dialog = gtk_message_dialog_new(GTK_WINDOW(win->getWindow()), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Error annotate PDF file '%s'\n%s"), filename.c_str(),
				doc->getLastErrorMsg().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	cursor->setCursorBusy(false);

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
	cursor->updateCursor();

	printf("annotate pdf %s\n", filename.c_str());

	//
	//		ui.saved = TRUE; // force close_journal to work
	//		control->close_journal();
	//		while (bgpdf.status != STATUS_NOT_INIT) {
	//			// waiting for pdf processes to finish dying
	//			gtk_main_iteration();
	//		}
	//		control->new_journal();
	//		ui.zoom = ui.startup_zoom;
	//		gnome_canvas_set_pixels_per_unit(canvas, ui.zoom);
	//		control->update_page_stuff();
	//		success = init_bgpdf(filename, TRUE, file_domain);
	//		set_cursor_busy(FALSE);
	//		if (success) {
	//			g_free(filename);
	//			return;
	//		}
	//
	//		/* open failed */
	//		dialog = gtk_message_dialog_new(GTK_WINDOW(winMain), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
	//				GTK_BUTTONS_OK, _("Error opening file '%s'"), filename);
	//		gtk_dialog_run(GTK_DIALOG(dialog));
	//		gtk_widget_destroy(dialog);


}

void Control::print() {
	printf("print\n");
}

void Control::copyProgressCallback(goffset current_num_bytes, goffset total_num_bytes, Control * control) {
	printf("copyProgressCallback: %i, %i\n", (int) current_num_bytes, (int) total_num_bytes);
}

bool Control::copy(String source, String target) {
	GFile * src = g_file_new_for_uri(source.c_str());
	GFile * trg = g_file_new_for_uri(target.c_str());
	GError * err = NULL;

	printf("copy: %s to %s\n", source .c_str(), target.c_str());

	bool ok = g_file_copy(src, trg, G_FILE_COPY_OVERWRITE, NULL, (GFileProgressCallback) &copyProgressCallback, this,
			&err);

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

bool Control::save() {
	if (doc->getFilename().isEmpty()) {
		if (!showSaveDialog()) {
			return false;
		}
	}

	SaveHandler h;
	h.prepareSave(doc);

	if (doc->shouldCreateBackupOnSave()) {
		String backup = doc->getFilename();
		backup += ".bak";
		if (!copy(doc->getFilename(), backup)) {
			//TODO: show error!
			printf("error: could not create backup!\n%s\n", this->copyError.c_str());
			return false;
		}

		doc->setCreateBackupOnSave(false);
	}

	GzOutputStream * out = new GzOutputStream(doc->getFilename());

	if (!out->getLastError().isEmpty()) {
		printf("error: %s\n", out->getLastError().c_str());
		return false;
	}

	h.saveTo(out);
	out->close();

	if (!out->getLastError().isEmpty()) {
		printf("error: %s\n", out->getLastError().c_str());
		return false;
	}

	delete out;

	recent->addRecentFile(doc->getFilename().c_str());
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
	GtkWidget *dialog;
	GtkFileFilter *filterXoj;

	char *name;
	int file_domain;
	gboolean success;

	dialog = gtk_file_chooser_dialog_new(_("Save File"), GTK_WINDOW(this->win->getWindow()),
			GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);

	filterXoj = gtk_file_filter_new();
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

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return false;
	}

	name = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));

	String filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	doc->setFilename(filename);
	updateDocName();

	return true;
}

void Control::updateDocName() {
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

class PdfExportRunnable: public Runnable {
public:
	PdfExportRunnable(Control * control) {
		this->control = control;
	}

	virtual bool run(bool * cancel) {
		PdfExport pdf(control->getDocument());
		printf("export as pdf\n");
		if (!pdf.createPdf("file:///home/andreas/tmp/pdffile.pdf", cancel)) {
			printf("create pdf failed\n");
			printf("error: %s\n", pdf.getLastError().c_str());
		}
	}

private:
	Control * control;
};

void Control::exportAsPdf() {
	background->run(new PdfExportRunnable(this));
}

void Control::exportAs() {
	//	ExportDialog * dlg = new ExportDialog();
	//	dlg->show();
	//
	//	delete dlg;


	FILE * fp = fopen("/home/andreas/tmp/pdfexp/exporttest.pdf", "w");

	//	for (int i = 0; i < doc->getPageCount(); i++) {
	//		PopplerPage * page = doc->getPdfPage(i);
	//		Object * o = new Object();
	//		o = page->page->getContents(o);
	//
	//	}

	fclose(fp);
}

void Control::saveAs() {
	if (!showSaveDialog()) {
		return;
	}
	if (doc->getFilename().isEmpty()) {
		return;
	}
	save();
}

void Control::quit() {
	if (!this->close()) {
		return;
	}
	if (background->isRunning()) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, _("There is still an operation running, exit anyway?"));

		gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Stop operation", 2);

		if (gtk_dialog_run(GTK_DIALOG(dialog)) == 1) {
			gtk_widget_destroy(dialog);
			return;
		}
		gtk_widget_destroy(dialog);

		background->stop();
	}

	settings->save();
	gtk_main_quit();
}

bool Control::close() {
	// TODO: Check if saved
	undoRedo->clearContents();
	return true;
}

void Control::showAbout() {
	AboutDialog * dlg = new AboutDialog();
	dlg->show();
	delete dlg;
}

Settings * Control::getSettings() {
	return settings;
}
