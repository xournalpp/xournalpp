#include "Control.h"

#include "PrintHandler.h"

#include "gui/Cursor.h"
#include "gui/dialog/AboutDialog.h"
#include "gui/dialog/backgroundSelect/ImagesDialog.h"
#include "gui/dialog/backgroundSelect/PdfPagesDialog.h"
#include "gui/dialog/GotoDialog.h"
#include "gui/dialog/FormatDialog.h"
#include "gui/dialog/PageTemplateDialog.h"
#include "gui/dialog/SettingsDialog.h"
#include "gui/dialog/SelectBackgroundColorDialog.h"
#include "gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"
#include "gui/dialog/ToolbarManageDialog.h"
#include "gui/TextEditor.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/XournalView.h"
#include "jobs/AutosaveJob.h"
#include "jobs/BlockingJob.h"
#include "jobs/CustomExportJob.h"
#include "jobs/PdfExportJob.h"
#include "jobs/SaveJob.h"
#include "xojfile/LoadHandler.h"
#include "model/BackgroundImage.h"
#include "model/FormatDefinitions.h"
#include "model/XojPage.h"
#include "settings/ButtonConfig.h"
#include "stockdlg/ImageOpenDlg.h"
#include "stockdlg/XojOpenDlg.h"
#include "undo/AddUndoAction.h"
#include "undo/DeleteUndoAction.h"
#include "undo/InsertDeletePageUndoAction.h"
#include "undo/InsertLayerUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "undo/PageBackgroundChangedUndoAction.h"
#include "undo/RemoveLayerUndoAction.h"
#include "view/DocumentView.h"

#include <config.h>
#include <config-dev.h>
#include <config-features.h>
#include <CrashHandler.h>
#include <i18n.h>
#include <serializing/ObjectInputStream.h>
#include <Stacktrace.h>
#include <XInputUtils.h>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
namespace bf = boost::filesystem;

#include <gtk/gtk.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <vector>
using std::vector;

// TODO Check for error log on startup, also check for emergency save document!

Control::Control(GladeSearchpath* gladeSearchPath, bool noThreads)
{
	XOJ_INIT_TYPE(Control);

	this->win = NULL;
	this->recent = new RecentManager();
	this->undoRedo = new UndoRedoHandler(this);
	this->recent->addListener(this);
	this->undoRedo->addUndoRedoListener(this);
	this->isBlocking = false;

	this->gladeSearchPath = gladeSearchPath;

	this->pageInserType = PAGE_INSERT_TYPE_LINED;

	this->metadata = new MetadataManager();
	this->cursor = new Cursor(this);

	this->lastAction = ACTION_NONE;
	this->lastGroup = GROUP_NOGROUP;
	this->lastEnabled = false;
	this->fullscreen = false;

	path name = path(g_get_home_dir());
	name /= CONFIG_DIR;
	name /= SETTINGS_XML_FILE;
	this->settings = new Settings(name);
	this->settings->load();

	this->sidebar = NULL;
	this->searchBar = NULL;

	this->scrollHandler = new ScrollHandler(this);

	this->scheduler = new XournalScheduler(noThreads);

	this->hiddenFullscreenWidgets = NULL;
	this->sidebarHidden = false;
	this->autosaveTimeout = 0;

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
	this->changeTimout = g_timeout_add_seconds(5, (GSourceFunc) checkChangedDocument, this);
	
	this->clipboardHandler = NULL;

	this->dragDropHandler = NULL;
}

Control::~Control()
{
	XOJ_CHECK_TYPE(Control);

	g_source_remove(this->changeTimout);
	this->enableAutosave(false);

	deleteLastAutosaveFile("");

	this->scheduler->stop();

	for (XojPage* page : this->changedPages)
	{
		page->unreference();
	}

	delete this->clipboardHandler;
	delete this->recent;
	delete this->undoRedo;
	delete this->settings;
	delete this->toolHandler;
	delete this->sidebar;
	delete this->doc;
	delete this->searchBar;
	delete this->scrollHandler;
	delete this->metadata;
	delete this->cursor;
	delete this->zoom;
	delete this->scheduler;
	delete this->dragDropHandler;

	XOJ_RELEASE_TYPE(Control);
}

void Control::renameLastAutosaveFile()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->lastAutosaveFilename.empty())
	{
		path filename = this->lastAutosaveFilename;
		path renamed = Util::getAutosaveFilename();
		renamed.replace_extension(filename.filename());
		
		if (bf::exists(filename))
		{
			bf::remove(renamed);
			bf::rename(filename, renamed);
		}
		else
		{
			bf::remove(renamed);
			this->save(false);
			bf::rename(filename, renamed);
			bf::remove(filename);
		}
	}
}

void Control::setLastAutosaveFile(path newAutosaveFile)
{
	this->lastAutosaveFilename = newAutosaveFile;
}

void Control::deleteLastAutosaveFile(path newAutosaveFile)
{
	XOJ_CHECK_TYPE(Control);

	if (!this->lastAutosaveFilename.empty())
	{
		// delete old autosave file
		bf::remove(path(this->lastAutosaveFilename));
	}
	this->lastAutosaveFilename = newAutosaveFile;
}

bool Control::checkChangedDocument(Control* control)
{
	XOJ_CHECK_TYPE_OBJ(control, Control);

	if (!control->doc->tryLock())
	{
		// call again later
		return true;
	}
	for (XojPage* page : control->changedPages)
	{
		int p = control->doc->indexOf(page);
		if (p != -1)
		{
			control->firePageChanged(p);
		}

		page->unreference();
	}
	control->changedPages.clear();

	control->doc->unlock();

	// Call again
	return true;
}

void Control::saveSettings()
{
	XOJ_CHECK_TYPE(Control);

	this->toolHandler->saveSettings();

	gint width = 0;
	gint height = 0;
	gtk_window_get_size((GtkWindow*) *this->win, &width, &height);

	if (!this->win->isMaximized())
	{
		this->settings->setMainWndSize(width, height);
	}
	this->settings->setMainWndMaximized(this->win->isMaximized());

	this->sidebar->saveSize();
}

void Control::initWindow(MainWindow* win)
{
	XOJ_CHECK_TYPE(Control);

	win->setRecentMenu(recent->getMenu());
	selectTool(toolHandler->getToolType());
	this->win = win;
	this->zoom->initZoomHandler(win->getXournal()->getWidget());
	this->sidebar = new Sidebar(win, this);

	updatePageNumbers(0, size_t_npos);

	toolHandler->eraserTypeChanged();

	this->searchBar = new SearchBar(this);

	// Disable undo buttons
	undoRedoChanged();

	setViewTwoPages(settings->isShowTwoPages());
	setViewPresentationMode(settings->isPresentationMode());

	PageTemplateSettings model;
	model.parse(settings->getPageTemplate());
 	setPageInsertType(model.getPageInsertType());

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

bool Control::autosaveCallback(Control* control)
{
	XOJ_CHECK_TYPE_OBJ(control, Control);

	if (!control->undoRedo->isChangedAutosave())
	{
		// do nothing, nothing changed
		return true;
	}
	else
	{
		cout << "Info: autosave document..." << endl;
	}

	AutosaveJob* job = new AutosaveJob(control);
	control->scheduler->addJob(job, JOB_PRIORITY_NONE);
	job->unref();

	return true;
}

void Control::enableAutosave(bool enable)
{
	XOJ_CHECK_TYPE(Control);

	if (this->autosaveTimeout)
	{
		g_source_remove(this->autosaveTimeout);
		this->autosaveTimeout = 0;
	}

	if (enable)
	{
		int timeout = settings->getAutosaveTimeout() * 60;
		this->autosaveTimeout = g_timeout_add_seconds(timeout, (GSourceFunc) autosaveCallback, this);
	}
}

void Control::updatePageNumbers(size_t page, size_t pdfPage)
{
	XOJ_CHECK_TYPE(Control);

	if (this->win == NULL)
	{
		return;
	}

	this->win->updatePageNumbers(page, this->doc->getPageCount(), pdfPage);
	this->sidebar->selectPageNr(page, pdfPage);

	this->metadata->storeMetadata(this->doc->getEvMetadataFilename().c_str(), page, getZoomControl()->getZoom());

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

void Control::actionPerformed(ActionType type, ActionGroup group, GdkEvent* event, GtkMenuItem* menuitem,
							  GtkToolButton* toolbutton, bool enabled)
{
	XOJ_CHECK_TYPE(Control);

	switch (type)
	{
		// Menu File
	case ACTION_NEW:
		clearSelectionEndText();
		newFile();
		break;
	case ACTION_OPEN:
		//clearSelectionEndText();
		openFile();
		break;
	case ACTION_ANNOTATE_PDF:
		clearSelectionEndText();
		annotatePdf("", false, false);
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
	case ACTION_PRINT:
		print();
		break;
	case ACTION_QUIT:
		quit();
		break;
		// Menu Edit
	case ACTION_UNDO:
		this->clearSelection();
		// Move out of text mode to allow textboxundo to work
		clearSelectionEndText();
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
		if (!win->getXournal()->actionDelete())
		{
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
	case ACTION_GOTO_NEXT_LAYER:
		{
			int layer = this->win->getCurrentLayer();
			PageRef p = getCurrentPage();
			if (layer < p->getLayerCount())
			{
				switchToLay(layer + 1);
			}
		}
		break;
	case ACTION_GOTO_PREVIOUS_LAYER:
		{
			int layer = this->win->getCurrentLayer();
			if (layer > 0)
			{
				switchToLay(layer - 1);
			}
		}
		break;
	case ACTION_GOTO_TOP_LAYER:
		{
			PageRef p = getCurrentPage();
			switchToLay(p->getLayerCount());
		}
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
	case ACTION_CONFIGURE_PAGE_TEMPLATE:
		paperTemplate();
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
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_PLAIN);
		}
		break;
	case ACTION_NEW_PAGE_LINED:
		clearSelectionEndText();
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_LINED);
		}
		break;
	case ACTION_NEW_PAGE_RULED:
		clearSelectionEndText();
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_RULED);
		}
		break;
	case ACTION_NEW_PAGE_GRAPH:
		clearSelectionEndText();
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_GRAPH);
		}
		break;
	case ACTION_NEW_PAGE_COPY:
		clearSelectionEndText();
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_COPY);
		}
		break;
	case ACTION_NEW_PAGE_PDF_BACKGROUND:
		clearSelectionEndText();
		if (enabled)
		{
			setPageInsertType(PAGE_INSERT_TYPE_PDF_BACKGROUND);
		}
		break;

		// Menu Tools
	case ACTION_TOOL_PEN:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_PEN);
		}
		break;
	case ACTION_TOOL_ERASER:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_ERASER);
		}
		break;

	case ACTION_TOOL_ERASER_STANDARD:
		if (enabled)
		{
			toolHandler->setEraserType(ERASER_TYPE_DEFAULT);
		}
		break;
	case ACTION_TOOL_ERASER_DELETE_STROKE:
		if (enabled)
		{
			toolHandler->setEraserType(ERASER_TYPE_DELETE_STROKE);
		}
		break;
	case ACTION_TOOL_ERASER_WHITEOUT:
		if (enabled)
		{
			toolHandler->setEraserType(ERASER_TYPE_WHITEOUT);
		}
		break;

	case ACTION_TOOL_HILIGHTER:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_HILIGHTER);
		}
		break;
	case ACTION_TOOL_TEXT:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_TEXT);
		}
		break;
	case ACTION_TOOL_IMAGE:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_IMAGE);
		}
		break;
	case ACTION_TOOL_SELECT_RECT:
		if (enabled)
		{
			selectTool(TOOL_SELECT_RECT);
		}
		break;
	case ACTION_TOOL_SELECT_REGION:
		if (enabled)
		{
			selectTool(TOOL_SELECT_REGION);
		}
		break;
	case ACTION_TOOL_SELECT_OBJECT:
		if (enabled)
		{
			selectTool(TOOL_SELECT_OBJECT);
		}
		break;
	case ACTION_TOOL_VERTICAL_SPACE:
		clearSelection();
		if (enabled)
		{
			selectTool(TOOL_VERTICAL_SPACE);
		}
		break;

	case ACTION_TOOL_HAND:
		if (enabled)
		{
			selectTool(TOOL_HAND);
		}
		break;

	case ACTION_TOOL_DRAW_RECT:
	case ACTION_TOOL_DRAW_CIRCLE:
	case ACTION_TOOL_DRAW_ARROW:
	case ACTION_RULER:
	case ACTION_SHAPE_RECOGNIZER:
		setShapeTool(type, enabled);
		break;

	case ACTION_TOOL_DEFAULT:
		if (enabled)
		{
			selectDefaultTool();
		}
		break;

	case ACTION_SIZE_VERY_THIN:
		if (enabled)
		{
			setToolSize(TOOL_SIZE_VERY_FINE);
		}
		break;
	case ACTION_SIZE_FINE:
		if (enabled)
		{
			setToolSize(TOOL_SIZE_FINE);
		}
		break;
	case ACTION_SIZE_MEDIUM:
		if (enabled)
		{
			setToolSize(TOOL_SIZE_MEDIUM);
		}
		break;
	case ACTION_SIZE_THICK:
		if (enabled)
		{
			setToolSize(TOOL_SIZE_THICK);
		}
		break;
	case ACTION_SIZE_VERY_THICK:
		if (enabled)
		{
			setToolSize(TOOL_SIZE_VERY_THICK);
		}
		break;

	case ACTION_TOOL_ERASER_SIZE_FINE:
		if (enabled)
		{
			this->toolHandler->setEraserSize(TOOL_SIZE_FINE);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_ERASER_SIZE_MEDIUM:
		if (enabled)
		{
			this->toolHandler->setEraserSize(TOOL_SIZE_MEDIUM);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_ERASER_SIZE_THICK:
		if (enabled)
		{
			this->toolHandler->setEraserSize(TOOL_SIZE_THICK);
			eraserSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_VERY_THIN:
		if (enabled)
		{
			this->toolHandler->setPenSize(TOOL_SIZE_VERY_FINE);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_FINE:
		if (enabled)
		{
			this->toolHandler->setPenSize(TOOL_SIZE_FINE);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_MEDIUM:
		if (enabled)
		{
			this->toolHandler->setPenSize(TOOL_SIZE_MEDIUM);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_THICK:
		if (enabled)
		{
			this->toolHandler->setPenSize(TOOL_SIZE_THICK);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_PEN_SIZE_VERY_THICK:
		if (enabled)
		{
			this->toolHandler->setPenSize(TOOL_SIZE_VERY_THICK);
			penSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_FINE:
		if (enabled)
		{
			this->toolHandler->setHilighterSize(TOOL_SIZE_FINE);
			hilighterSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_MEDIUM:
		if (enabled)
		{
			this->toolHandler->setHilighterSize(TOOL_SIZE_MEDIUM);
			hilighterSizeChanged();
		}
		break;
	case ACTION_TOOL_HILIGHTER_SIZE_THICK:
		if (enabled)
		{
			this->toolHandler->setHilighterSize(TOOL_SIZE_THICK);
			hilighterSizeChanged();
		}
		break;

	case ACTION_FONT_BUTTON_CHANGED:
		fontChanged();
		break;

	case ACTION_SELECT_FONT:
		if (win)
		{
			win->getToolMenuHandler()->showFontSelectionDlg();
		}
		break;

		// Used for all colors
	case ACTION_SELECT_COLOR:
	case ACTION_SELECT_COLOR_CUSTOM:
		// nothing to do here, the color toolbar item handles the color
		break;
	case ACTION_TEX:
		runLatex();
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

	case ACTION_VIEW_PRESENTATION_MODE:
		setViewPresentationMode(enabled);
		break;

	case ACTION_FOOTER_LAYER:
		switchToLay(this->win->getCurrentLayer());
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

	if (type >= ACTION_TOOL_PEN && type <= ACTION_TOOL_HAND)
	{
		ActionType at = (ActionType) (toolHandler->getToolType() - TOOL_PEN + ACTION_TOOL_PEN);
		if (type == at && !enabled)
		{
			fireActionSelected(GROUP_TOOL, at);
		}
	}
}

#define XOJ_HELP "https://github.com/xournalpp/xournalpp/wiki/User-Manual"

void Control::help()
{
	GError* error = NULL;

	gtk_show_uri(gtk_window_get_screen(GTK_WINDOW(this->win->getWindow())), XOJ_HELP, gtk_get_current_event_time(), &error);
	if (error)
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) getWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
												   GTK_BUTTONS_OK, "%s",
												   FC(_F("There was an error displaying help: {1}") % error->message));
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		g_error_free(error);
	}
}

bool Control::copy()
{
	if (this->win && this->win->getXournal()->copy())
	{
		return true;
	}
	return this->clipboardHandler->copy();
}

bool Control::cut()
{
	if (this->win && this->win->getXournal()->cut())
	{
		return true;
	}
	return this->clipboardHandler->cut();
}

bool Control::paste()
{
	if (this->win && this->win->getXournal()->paste())
	{
		return true;
	}
	return this->clipboardHandler->paste();
}

void Control::clearSelectionEndText()
{
	XOJ_CHECK_TYPE(Control);

	clearSelection();
	if (win)
	{
		win->getXournal()->endTextAllPages();
	}
}

void Control::invokeLater(ActionType type)
{
	XOJ_CHECK_TYPE(Control);

	g_idle_add((GSourceFunc) &invokeCallback, new CallbackData(this, type));
}

/**
 * Fire page selected, but first check if the page Number is valid
 *
 * @return the page ID or size_t_npos if the page is not found
 */
size_t Control::firePageSelected(PageRef page)
{
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	size_t pageId = this->doc->indexOf(page);
	this->doc->unlock();
	if (pageId == size_t_npos)
	{
		return size_t_npos;
	}

	DocumentHandler::firePageSelected(pageId);
	return pageId;
}

void Control::firePageSelected(size_t page)
{
	XOJ_CHECK_TYPE(Control);

	DocumentHandler::firePageSelected(page);
}

void Control::manageToolbars()
{
	XOJ_CHECK_TYPE(Control);

	ToolbarManageDialog dlg(this->gladeSearchPath, this->win->getToolbarModel());
	dlg.show(GTK_WINDOW(this->win->getWindow()));

	this->win->updateToolbarMenu();

	path file = Util::getConfigFile(TOOLBAR_CONFIG);
	this->win->getToolbarModel()->save(file.c_str());
}

void Control::customizeToolbars()
{
	XOJ_CHECK_TYPE(Control);

	g_return_if_fail(this->win != NULL);

	if (this->win->getSelectedToolbar()->isPredefined())
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this->win, GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
												   FC(_F("The Toolbarconfiguration \"{1}\" is predefined, "
												   "would you create a copy to edit?")
													% this->win->getSelectedToolbar()->getName()));

		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (res == -8) // Yes
		{
			ToolbarData* data = new ToolbarData(*this->win->getSelectedToolbar());

			ToolbarModel* model = this->win->getToolbarModel();

			for (int i = 0; i < 100; i++)
			{
				string id = data->getId() + " Copy";

				if (i != 0)
				{
					id += " ";
					id += std::to_string(i);
				}

				if (!model->existsId(id))
				{
					if (i != 0)
					{
						string filename = data->getName();
						filename += " ";
						filename += FS(_("Copy"));
						filename += " ";
						filename += std::to_string(i);

						data->setName(filename);
					}
					else
					{
						data->setName(data->getName() + " " + FS(_("Copy")));
					}
					data->setId(id);
					break;
				}
			}

			model->add(data);
			this->win->toolbarSelected(data);
			this->win->updateToolbarMenu();
		}
		else
		{
			return;
		}
	}

	if (!this->dragDropHandler)
	{
		this->dragDropHandler = new ToolbarDragDropHandler(this);
	}
	this->dragDropHandler->configure();
}

void Control::endDragDropToolbar()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler)
	{
		return;
	}

	this->dragDropHandler->clearToolbarsFromDragAndDrop();
}

void Control::startDragDropToolbar()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler)
	{
		return;
	}

	this->dragDropHandler->prepareToolbarsForDragAndDrop();
}

bool Control::isInDragAndDropToolbar()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->dragDropHandler)
	{
		return false;
	}

	return this->dragDropHandler->isInDragAndDrop();
}

void Control::setShapeTool(ActionType type, bool enabled)
{
	XOJ_CHECK_TYPE(Control);

	if (enabled == false)
	{
		// Disable all entries
		this->toolHandler->setRuler(false);
		this->toolHandler->setRectangle(false);
		this->toolHandler->setArrow(false);
		this->toolHandler->setCircle(false);
		this->toolHandler->setShapeRecognizer(false);

		// fire disabled and return
		fireActionSelected(GROUP_RULER, ACTION_NONE);
		return;
	}

	// Check for nothing changed, and return in this case
	if ((this->toolHandler->isRuler() && type == ACTION_RULER) ||
		(this->toolHandler->isRectangle() && type == ACTION_TOOL_DRAW_RECT) ||
		(this->toolHandler->isArrow() && type == ACTION_TOOL_DRAW_ARROW) ||
		(this->toolHandler->isCircle() && type == ACTION_TOOL_DRAW_CIRCLE) ||
		(this->toolHandler->isShapeRecognizer() && type == ACTION_SHAPE_RECOGNIZER))
	{
		return;
	}

	switch (type)
	{
	case ACTION_TOOL_DRAW_RECT:
		this->toolHandler->setRectangle(true, true);
		break;

	case ACTION_TOOL_DRAW_CIRCLE:
		this->toolHandler->setCircle(true, true);
		break;

	case ACTION_TOOL_DRAW_ARROW:
		this->toolHandler->setArrow(true, true);
		break;

	case ACTION_RULER:
		this->toolHandler->setRuler(true, true);
		break;

	case ACTION_SHAPE_RECOGNIZER:
		this->toolHandler->setShapeRecognizer(true, true);
		this->resetShapeRecognizer();
		break;

	default:
		g_warning("Invalid type for setShapeTool: %i", type);
		break;
	}

	fireActionSelected(GROUP_RULER, type);
}

void Control::enableFullscreen(bool enabled, bool presentation)
{
	XOJ_CHECK_TYPE(Control);

	if (enabled)
	{
		gtk_window_fullscreen((GtkWindow*) *win);

		string str = presentation ? settings->getPresentationHideElements() : settings->getFullscreenHideElements();

		using std::vector;
		vector<string> part;
		ba::split(part, str, boost::is_any_of(","));

		for (string s : part)
		{
			if ("sidebarContents" == s && settings->isSidebarVisible())
			{
				this->sidebarHidden = true;
				win->setSidebarVisible(false);
			}
			else
			{
				GtkWidget* w = win->get(s.c_str());
				if (w == NULL)
				{
					g_warning("Fullscreen: Try to hide \"%s\", but coulden't find it. Wrong entry in ~/"
							  CONFIG_DIR "/" SETTINGS_XML_FILE "?", s.c_str());
				}
				else
				{
					if (gtk_widget_get_visible(w))
					{
						gtk_widget_hide(w);
						this->hiddenFullscreenWidgets = g_list_append(this->hiddenFullscreenWidgets, w);
					}
				}
			}
		}
	}
	else
	{
		gtk_window_unfullscreen((GtkWindow*) *win);

		for (GList* l = this->hiddenFullscreenWidgets; l != NULL; l = l->next)
		{
			gtk_widget_show(GTK_WIDGET(l->data));
		}

		if (this->sidebarHidden)
		{
			this->sidebarHidden = false;
			win->setSidebarVisible(true);
		}

		g_list_free(this->hiddenFullscreenWidgets);
		this->hiddenFullscreenWidgets = NULL;
	}

	fireActionSelected(GROUP_FULLSCREEN, enabled ? ACTION_FULLSCREEN : ACTION_NONE);
	this->fullscreen = enabled;
}

void Control::disableSidebarTmp(bool disabled)
{
	XOJ_CHECK_TYPE(Control);

	this->sidebar->setTmpDisabled(disabled);
}

void Control::addDefaultPage()
{
	XOJ_CHECK_TYPE(Control);

	PageTemplateSettings model;
	model.parse(settings->getPageTemplate());

	PageRef page = new XojPage(model.getPageWidth(), model.getPageHeight());
	page->setBackgroundColor(model.getBackgroundColor());
	page->setBackgroundType(model.getBackgroundType());

	this->doc->lock();
	this->doc->addPage(page);
	this->doc->unlock();

	updateDeletePageButton();
}

void Control::updateDeletePageButton()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win)
	{
		GtkWidget* w = this->win->get("menuDeletePage");
		gtk_widget_set_sensitive(w, this->doc->getPageCount() > 1);
	}
}

void Control::deletePage()
{
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	// don't allow delete pages if we have less than 2 pages,
	// so we can be (more or less) sure there is at least one page.
	if (this->doc->getPageCount() < 2)
	{
		return;
	}

	size_t pNr = getCurrentPageNo();
	if (pNr == size_t_npos || pNr > this->doc->getPageCount())
	{
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

	if (pNr >= this->doc->getPageCount())
	{
		pNr = this->doc->getPageCount() - 1;
	}

	scrollHandler->scrollToPage(pNr, 0);
}

void Control::insertNewPage(size_t position)
{
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();

	if (position > doc->getPageCount())
	{
		position = doc->getPageCount();
	}

	double width = 0;
	double height = 0;

	PageTemplateSettings model;
	model.parse(settings->getPageTemplate());

	int lastPage = position - 1;
	if (lastPage < 0 || !model.isCopyLastPageSettings())
	{
		width = model.getPageWidth();
		height = model.getPageHeight();
	}
	else
	{
		PageRef page = doc->getPage(lastPage);

		if (page.isValid())
		{
			width = page->getWidth();
			height = page->getHeight();
		}
		else
		{
			width = model.getPageWidth();
			height = model.getPageHeight();
		}
	}

	PageRef page = new XojPage(width, height);
	page->setBackgroundColor(model.getBackgroundColor());

	if (PAGE_INSERT_TYPE_PLAIN == this->pageInserType)
	{
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	}
	else if (PAGE_INSERT_TYPE_LINED == this->pageInserType)
	{
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	}
	else if (PAGE_INSERT_TYPE_RULED == this->pageInserType)
	{
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	}
	else if (PAGE_INSERT_TYPE_GRAPH == this->pageInserType)
	{
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	}
	else if (PAGE_INSERT_TYPE_COPY == this->pageInserType)
	{
		PageRef current = getCurrentPage();
		if (!current.isValid()) // should not happen, but if you open an invalid file or something like this...
		{
			page->setBackgroundType(BACKGROUND_TYPE_LINED);
		}
		else
		{
			BackgroundType bg = current->getBackgroundType();
			page->setBackgroundType(bg);
			if (bg == BACKGROUND_TYPE_PDF)
			{
				page->setBackgroundPdfPageNr(current->getPdfPageNr());
			}
			else if (bg == BACKGROUND_TYPE_IMAGE)
			{
				page->setBackgroundImage(current->getBackgroundImage());
			}
			else
			{
				page->setBackgroundColor(current->getBackgroundColor());
			}

			page->setSize(current->getWidth(), current->getHeight());
		}
	}
	else if (PAGE_INSERT_TYPE_PDF_BACKGROUND == this->pageInserType)
	{
		if (this->doc->getPdfPageCount() == 0)
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
													   GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
													   "%s", _C("You don't have any PDF pages to select from. "
																"Cancel operation.\nPlease select another background type: "
																"Menu \"Journal\" / \"Insert Page Type\"."));
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			return;
		}
		else
		{
			this->doc->lock();
			PdfPagesDialog* dlg = new PdfPagesDialog(this->gladeSearchPath, this->doc, this->settings);

			dlg->show(GTK_WINDOW(this->win->getWindow()));

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected >= 0 && selected < doc->getPdfPageCount())
			{
				// no need to set a type, if we set the page number the type is also set
				page->setBackgroundPdfPageNr(selected);

				XojPopplerPage* p = doc->getPdfPage(selected);
				page->setSize(p->getWidth(), p->getHeight());
			}

			this->doc->unlock();
		}
	}

	insertPage(page, position);
}

void Control::insertPage(PageRef page, size_t position)
{
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	this->doc->insertPage(page, position);
	this->doc->unlock();
	firePageInserted(position);

	getCursor()->updateCursor();

	int visibleHeight = 0;
	scrollHandler->isPageVisible(position, &visibleHeight);

	if (visibleHeight < 10)
	{
		scrollHandler->scrollToPage(position);
	}
	firePageSelected(position);

	updateDeletePageButton();

	undoRedo->addUndoAction(new InsertDeletePageUndoAction(page, position, true));
}

void Control::addNewLayer()
{
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	PageRef p = getCurrentPage();
	if (!p.isValid())
	{
		return;
	}

	Layer* l = new Layer();
	p->insertLayer(l, p->getSelectedLayerId());
	if (win)
	{
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new InsertLayerUndoAction(p, l));
}

void Control::setPageBackground(ActionType type)
{
	XOJ_CHECK_TYPE(Control);

	PageRef page = getCurrentPage();

	if (!page.isValid())
	{
		return;
	}

	this->doc->lock();
	size_t pageNr = this->doc->indexOf(page);
	this->doc->unlock();
	if (pageNr == size_t_npos)
	{
		return; // should not happen...
	}

	int origPdfPage = page->getPdfPageNr();
	BackgroundType origType = page->getBackgroundType();
	BackgroundImage origBackgroundImage = page->getBackgroundImage();
	double origW = page->getWidth();
	double origH = page->getHeight();

	if (ACTION_SET_PAPER_BACKGROUND_PLAIN == type)
	{
		page->setBackgroundType(BACKGROUND_TYPE_NONE);
	}
	else if (ACTION_SET_PAPER_BACKGROUND_LINED == type)
	{
		page->setBackgroundType(BACKGROUND_TYPE_LINED);
	}
	else if (ACTION_SET_PAPER_BACKGROUND_RULED == type)
	{
		page->setBackgroundType(BACKGROUND_TYPE_RULED);
	}
	else if (ACTION_SET_PAPER_BACKGROUND_GRAPH == type)
	{
		page->setBackgroundType(BACKGROUND_TYPE_GRAPH);
	}
	else if (ACTION_SET_PAPER_BACKGROUND_IMAGE == type)
	{
		this->doc->lock();
		ImagesDialog* dlg = new ImagesDialog(this->gladeSearchPath, this->doc, this->settings);
		this->doc->unlock();

		dlg->show(GTK_WINDOW(this->win->getWindow()));
		BackgroundImage img = dlg->getSelectedImage();
		if (!img.isEmpty())
		{
			page->setBackgroundImage(img);
			page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
		}
		else if (dlg->shouldShowFilechooser())
		{

			bool attach = false;
			GFile* file = ImageOpenDlg::show((GtkWindow*) *win, settings, true, &attach);
			if (file == NULL)
			{
				return;
			}
			char* name = g_file_get_path(file);
			string filename = name;
			g_free(name);
			g_object_unref(file);

			BackgroundImage newImg;
			GError* err = NULL;
			newImg.loadFile(filename, &err);
			newImg.setAttach(attach);
			if (err)
			{
				GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
														   GTK_DIALOG_DESTROY_WITH_PARENT,
														   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
														   FC(_F("This image could not be loaded. Error message: {1}")
															% err->message));
				gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
				gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);
				g_error_free(err);
				return;
			}
			else
			{
				page->setBackgroundImage(newImg);
				page->setBackgroundType(BACKGROUND_TYPE_IMAGE);
			}
		}

		GdkPixbuf* pixbuf = page->getBackgroundImage().getPixbuf();
		if (pixbuf)
		{
			page->setSize(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
			firePageSizeChanged(pageNr);
		}

		delete dlg;
	}
	else if (ACTION_SET_PAPER_BACKGROUND_PDF == type)
	{
		if (doc->getPdfPageCount() == 0)
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
													   GTK_DIALOG_DESTROY_WITH_PARENT,
													   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s",
													   _C("You don't have any PDF pages to select from. Cancel operation.\n"
														  "Please select another background type: Menu \"Journal\" → \"Insert Page Type\"."));
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		}
		else
		{
			this->doc->lock();
			PdfPagesDialog* dlg = new PdfPagesDialog(this->gladeSearchPath, this->doc, this->settings);
			this->doc->unlock();

			dlg->show(GTK_WINDOW(this->win->getWindow()));

			int selected = dlg->getSelectedPage();
			delete dlg;

			if (selected >= 0 && selected < doc->getPdfPageCount())
			{
				// no need to set a type, if we set the page number the type is also set
				page->setBackgroundPdfPageNr(selected);
			}
		}
	}

	firePageChanged(pageNr);
	updateBackgroundSizeButton();

	this->undoRedo->addUndoAction(
		new PageBackgroundChangedUndoAction(page, origType, origPdfPage, origBackgroundImage, origW, origH));
}

void Control::gotoPage()
{
	GotoDialog* dlg = new GotoDialog(this->gladeSearchPath, this->doc->getPageCount());

	dlg->show(GTK_WINDOW(this->win->getWindow()));
	int page = dlg->getSelectedPage();

	if (page != -1)
	{
		this->scrollHandler->scrollToPage(page - 1, 0);
	}

	delete dlg;
}

void Control::updateBackgroundSizeButton()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win == NULL)
	{
		return;
	}

	// Update paper color button
	PageRef p = getCurrentPage();
	if (!p.isValid() || this->win == NULL)
	{
		return;
	}
	BackgroundType bg = p->getBackgroundType();
	GtkWidget* paperColor = win->get("menuJournalPaperColor");
	GtkWidget* pageSize = win->get("menuJournalPaperFormat");

	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg &&
		BACKGROUND_TYPE_RULED != bg && BACKGROUND_TYPE_GRAPH != bg)
	{
		gtk_widget_set_sensitive(paperColor, false);
	}
	else
	{
		gtk_widget_set_sensitive(paperColor, true);
	}

	// PDF page size is defined, you cannot change it
	gtk_widget_set_sensitive(pageSize, bg != BACKGROUND_TYPE_PDF);
}

void Control::paperTemplate()
{
	XOJ_CHECK_TYPE(Control);

	PageTemplateDialog* dlg = new PageTemplateDialog(this->gladeSearchPath, settings);
	dlg->show(GTK_WINDOW(this->win->getWindow()));

	if (dlg->isSaved())
	{
		PageTemplateSettings model;
		model.parse(settings->getPageTemplate());
	 	setPageInsertType(model.getPageInsertType());
	}

	delete dlg;
}

void Control::paperFormat()
{
	XOJ_CHECK_TYPE(Control);

	PageRef page = getCurrentPage();
	if (!page.isValid() || page->getBackgroundType() == BACKGROUND_TYPE_PDF)
	{
		return;
	}
	clearSelectionEndText();

	FormatDialog* dlg = new FormatDialog(this->gladeSearchPath, settings, page->getWidth(), page->getHeight());
	dlg->show(GTK_WINDOW(this->win->getWindow()));

	double width = dlg->getWidth();
	double height = dlg->getHeight();

	if (width > 0)
	{
		this->doc->lock();
		this->doc->setPageSize(page, width, height);
		this->doc->unlock();
	}

	delete dlg;
}

void Control::changePageBackgroundColor()
{
	XOJ_CHECK_TYPE(Control);

	int pNr = getCurrentPageNo();
	this->doc->lock();
	PageRef p = this->doc->getPage(pNr);
	this->doc->unlock();

	if (!p.isValid())
	{
		return;
	}

	clearSelectionEndText();

	BackgroundType bg = p->getBackgroundType();
	if (BACKGROUND_TYPE_NONE != bg && BACKGROUND_TYPE_LINED != bg &&
		BACKGROUND_TYPE_RULED != bg && BACKGROUND_TYPE_GRAPH != bg)
	{
		return;
	}

	SelectBackgroundColorDialog dlg(this);
	dlg.show(GTK_WINDOW(this->win->getWindow()));
	int color = dlg.getSelectedColor();

	if (color != -1)
	{
		p->setBackgroundColor(color);
		firePageChanged(pNr);
	}
}

void Control::switchToLay(int layer)
{
	clearSelectionEndText();
	PageRef p = getCurrentPage();
	if (p.isValid())
	{
		p->setSelectedLayerId(layer);
		this->win->getXournal()->layerChanged(getCurrentPageNo());
		this->win->updateLayerCombobox();
	}
}

void Control::deleteCurrentLayer()
{
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	PageRef p = getCurrentPage();
	int pId = getCurrentPageNo();
	if (!p.isValid())
	{
		return;
	}

	int lId = p->getSelectedLayerId();
	if (lId < 1)
	{
		return;
	}
	Layer* l = p->getSelectedLayer();

	p->removeLayer(l);
	if (win)
	{
		win->getXournal()->layerChanged(pId);
		win->updateLayerCombobox();
	}

	undoRedo->addUndoAction(new RemoveLayerUndoAction(p, l, lId - 1));
	this->resetShapeRecognizer();
}

void Control::calcZoomFitSize()
{
	XOJ_CHECK_TYPE(Control);

	if (this->doc && this->win)
	{
		PageRef p = getCurrentPage();
		if (!p.isValid())
		{
			return;
		}
		double width = p->getWidth() + 20;

		GtkAllocation allocation = {0};

		gtk_widget_get_allocation(win->getXournal()->getWidget(), &allocation);
		double factor = ((double) allocation.width) / width;
		zoom->setZoomFit(factor);
	}
}

void Control::zoomFit()
{
	XOJ_CHECK_TYPE(Control);

	calcZoomFitSize();
	zoom->zoomFit();
}

void Control::setViewTwoPages(bool twoPages)
{
	XOJ_CHECK_TYPE(Control);

	settings->setShowTwoPages(twoPages);
	fireActionSelected(GROUP_TWOPAGES, twoPages ? ACTION_VIEW_TWO_PAGES : ACTION_NOT_SELECTED);

	int currentPage = getCurrentPageNo();
	win->getXournal()->layoutPages();
	scrollHandler->scrollToPage(currentPage);
}

void Control::setViewPresentationMode(bool presentationMode)
{
	XOJ_CHECK_TYPE(Control);

	settings->setPresentationMode(presentationMode);
	fireActionSelected(GROUP_PRESENTATION_MODE, presentationMode ? ACTION_VIEW_PRESENTATION_MODE : ACTION_NOT_SELECTED);

	int currentPage = getCurrentPageNo();
	win->getXournal()->layoutPages();
	scrollHandler->scrollToPage(currentPage);
}

void Control::setPageInsertType(PageInsertType type)
{
	XOJ_CHECK_TYPE(Control);

	this->pageInserType = type;
	fireActionSelected(GROUP_PAGE_INSERT_TYPE, (ActionType) (type - PAGE_INSERT_TYPE_PLAIN + ACTION_NEW_PAGE_PLAIN));
}

bool Control::invokeCallback(CallbackData* cb)
{
	gdk_threads_enter();

	XOJ_CHECK_TYPE_OBJ(cb->control, Control);

	ZoomControl* zoom = cb->control->getZoomControl();

	switch (cb->type)
	{
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
	default:
		break;
	}

	delete cb;

	gdk_threads_leave();

	return false;
}

size_t Control::getCurrentPageNo()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win)
	{
		return this->win->getXournal()->getCurrentPage();
	}
	return 0;
}

bool Control::searchTextOnPage(string text, int p, int* occures,
							   double* top)
{
	XOJ_CHECK_TYPE(Control);

	return getWindow()->getXournal()->searchTextOnPage(text, p, occures, top);
}

PageRef Control::getCurrentPage()
{
	XOJ_CHECK_TYPE(Control);

	int page = this->win->getXournal()->getCurrentPage();

	this->doc->lock();
	PageRef p = this->doc->getPage(page);
	this->doc->unlock();

	return p;
}

void Control::fileOpened(const char* uri)
{
	XOJ_CHECK_TYPE(Control);

	openFile(uri);
	win->updateLayerCombobox();
}

void Control::undoRedoChanged()
{
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_UNDO, undoRedo->canUndo());
	fireEnableAction(ACTION_REDO, undoRedo->canRedo());

	win->setUndoDescription(undoRedo->undoDescription());
	win->setRedoDescription(undoRedo->redoDescription());

	updateWindowTitle();
}

void Control::undoRedoPageChanged(PageRef page)
{
	XOJ_CHECK_TYPE(Control);

	for (XojPage* p : this->changedPages)
	{
		if (p == (XojPage*) page)
		{
			return;
		}
	}

	XojPage* p = (XojPage*) page;
	this->changedPages.push_back(p);
	p->reference();
}

void Control::selectTool(ToolType type)
{
	XOJ_CHECK_TYPE(Control);

	toolHandler->selectTool(type);
}

void Control::selectDefaultTool()
{
	XOJ_CHECK_TYPE(Control);

	ButtonConfig* cfg = settings->getDefaultButtonConfig();
	cfg->acceptActions(toolHandler);
}

void Control::toolChanged()
{
	XOJ_CHECK_TYPE(Control);

	ToolType type = toolHandler->getToolType();

	// Convert enum values, enums has to be in the same order!
	ActionType at = (ActionType) (type - TOOL_PEN + ACTION_TOOL_PEN);

	fireActionSelected(GROUP_TOOL, at);

	fireEnableAction(ACTION_SELECT_COLOR, toolHandler->isEnableColor());
	fireEnableAction(ACTION_SELECT_COLOR_CUSTOM, toolHandler->isEnableColor());

	fireEnableAction(ACTION_RULER, toolHandler->isEnableRuler());
	fireEnableAction(ACTION_TOOL_DRAW_RECT, toolHandler->isEnableRectangle());
	fireEnableAction(ACTION_TOOL_DRAW_CIRCLE, toolHandler->isEnableCircle());
	fireEnableAction(ACTION_TOOL_DRAW_ARROW, toolHandler->isEnableArrow());
	fireEnableAction(ACTION_SHAPE_RECOGNIZER, toolHandler->isEnableShapreRecognizer());

	bool enableSize = toolHandler->isEnableSize();

	fireEnableAction(ACTION_SIZE_MEDIUM, enableSize);
	fireEnableAction(ACTION_SIZE_THICK, enableSize);
	fireEnableAction(ACTION_SIZE_FINE, enableSize);
	fireEnableAction(ACTION_SIZE_VERY_THICK, enableSize);
	fireEnableAction(ACTION_SIZE_VERY_THIN, enableSize);

	if (enableSize)
	{
		toolSizeChanged();
	}

	// Update color
	if (toolHandler->isEnableColor())
	{
		toolColorChanged();
	}

	ActionType rulerAction = ACTION_NOT_SELECTED;
	if (toolHandler->isShapeRecognizer())
	{
		rulerAction = ACTION_SHAPE_RECOGNIZER;
	}
	else if (toolHandler->isRuler())
	{
		rulerAction = ACTION_RULER;
	}
	else if (toolHandler->isRectangle())
	{
		rulerAction = ACTION_TOOL_DRAW_RECT;
	}
	else if (toolHandler->isCircle())
	{
		rulerAction = ACTION_TOOL_DRAW_CIRCLE;
	}
	else if (toolHandler->isArrow())
	{
		rulerAction = ACTION_TOOL_DRAW_ARROW;
	}

	fireActionSelected(GROUP_RULER, rulerAction);

	getCursor()->updateCursor();

	if (type != TOOL_TEXT)
	{
		if (win)
		{
			win->getXournal()->endTextAllPages();
		}
	}
}

void Control::eraserSizeChanged()
{
	XOJ_CHECK_TYPE(Control);

	switch (toolHandler->getEraserSize())
	{
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

void Control::penSizeChanged()
{
	XOJ_CHECK_TYPE(Control);

	switch (toolHandler->getPenSize())
	{
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

void Control::hilighterSizeChanged()
{
	XOJ_CHECK_TYPE(Control);

	switch (toolHandler->getHilighterSize())
	{
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

void Control::toolSizeChanged()
{
	XOJ_CHECK_TYPE(Control);

	if (toolHandler->getToolType() == TOOL_PEN)
	{
		penSizeChanged();
	}
	else if (toolHandler->getToolType() == TOOL_ERASER)
	{
		eraserSizeChanged();
	}
	else if (toolHandler->getToolType() == TOOL_HILIGHTER)
	{
		hilighterSizeChanged();
	}

	switch (toolHandler->getSize())
	{
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

void Control::toolColorChanged()
{
	XOJ_CHECK_TYPE(Control);

	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR);
	getCursor()->updateCursor();

	if (this->win && toolHandler->getColor() != -1)
	{
		EditSelection* sel = this->win->getXournal()->getSelection();
		if (sel)
		{
			UndoAction* undo = sel->setColor(toolHandler->getColor());
			undoRedo->addUndoAction(undo);
		}

		TextEditor* edit = getTextEditor();


		if (this->toolHandler->getToolType() == TOOL_TEXT && edit != NULL)
		{
			UndoAction* undo = edit->setColor(toolHandler->getColor());
			undoRedo->addUndoAction(undo);
		}
	}
}

void Control::setCustomColorSelected()
{
	XOJ_CHECK_TYPE(Control);

	fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR_CUSTOM);
}

void Control::showSettings()
{
	XOJ_CHECK_TYPE(Control);

	int selectionColor = settings->getSelectionColor();
	bool verticalSpace = settings->getAddVerticalSpace();
	bool horizontalSpace = settings->getAddHorizontalSpace();
	bool bigCursor = settings->isShowBigCursor();

	SettingsDialog* dlg = new SettingsDialog(this->gladeSearchPath, settings);
	dlg->show(GTK_WINDOW(this->win->getWindow()));

	if (selectionColor != settings->getSelectionColor())
	{
		win->getXournal()->forceUpdatePagenumbers();
	}

	if (verticalSpace != settings->getAddVerticalSpace() || horizontalSpace != settings->getAddHorizontalSpace())
	{
		int currentPage = getCurrentPageNo();
		win->getXournal()->layoutPages();
		scrollHandler->scrollToPage(currentPage);
	}

	if (bigCursor != settings->isShowBigCursor())
	{
		getCursor()->updateCursor();
	}

	win->updateScrollbarSidebarPosition();

	enableAutosave(settings->isAutosaveEnabled());

	getWindow()->getXournal()->setEventCompression(settings->isEventCompression());

	this->zoom->setZoom100(settings->getDisplayDpi() / 72.0);
	delete dlg;
}

bool Control::newFile()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->close())
	{
		return false;
	}

	Document newDoc(this);

	this->doc->lock();
	*doc = newDoc;
	this->doc->unlock();

	addDefaultPage();

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

	fileLoaded();

	return true;
}


/**
 * Check if this is an autosave file, return false in this case and display a user instruction
 */
bool Control::shouldFileOpen(string filename)
{
	// Compare case insensitive, just in case (Windows, FAT Filesystem etc.)
	filename = boost::algorithm::to_lower_copy(filename);
	string basename = boost::algorithm::to_lower_copy(Util::getConfigSubfolder("").string());

	if (basename.size() > filename.size())
	{
		return true;
	}

	filename = filename.substr(0, basename.size());

	if (filename == basename)
	{
		string msg = (_F("Do not open Autosave files. They may will be overwritten!\n"
				"Copy the files to another folder.\n"
				"Files from Folder {1} cannot be opened.") % basename).str();
		GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW((GtkWindow*) *win), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
												   GTK_BUTTONS_OK, "%s", msg.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return false;
	}

	return true;
}

bool Control::openFile(path filename, int scrollToPage)
{
	XOJ_CHECK_TYPE(Control);

	if (!shouldFileOpen(filename.string()))
	{
		return false;
	}

	if (!this->close())
	{
		return false;
	}

	if (filename.empty())
	{
		bool attachPdf = false;
		XojOpenDlg dlg((GtkWindow*) *win, this->settings);
		filename = dlg.showOpenDialog(false, attachPdf);

		cout << _F("Filename: {1}") % filename.string() << endl;

		if (filename.empty())
		{
			return false;
		}

		if (!shouldFileOpen(filename.string()))
		{
			return false;
		}
	}

	LoadHandler h;

	if (filename.extension() == ".pdf")
	{
		if (settings->isAutloadPdfXoj())
		{
			path f = path(filename).replace_extension(".pdf.xoj");
			Document* tmp = h.loadDocument(f.string());
			if (tmp)
			{

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

	Document* tmp = h.loadDocument(filename.string());
	if ((tmp != NULL && h.isAttachedPdfMissing()) || !h.getMissingPdfFilename().empty())
	{
		// give the user a second chance to select a new PDF file, or to discard the PDF


		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *getWindow(),
												   GTK_DIALOG_DESTROY_WITH_PARENT,
													   GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
													   h.isAttachedPdfMissing()
															? _C("The attached background PDF could not be found.")
															: _C("The background PDF could not be found."));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Select another PDF"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Remove PDF Background"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Cancel"), 3);
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		if (res == 2) // remove PDF background
		{
			h.removePdfBackground();
			tmp = h.loadDocument(filename.string());
		}
		else if (res == 1) // select another PDF background
		{
			bool attachToDocument = false;
			XojOpenDlg dlg((GtkWindow*) *win, this->settings);
			path pdfFilename = dlg.showOpenDialog(true, attachToDocument);
			if (!pdfFilename.empty())
			{
				h.setPdfReplacement(pdfFilename.string(), attachToDocument);
				tmp = h.loadDocument(filename.string());
			}
		}
	}

	if (!tmp)
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
												   GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
												   GTK_BUTTONS_OK, "%s\n%s",
												   FC(_F("Error opening file \"{1}\"") % filename),
												   h.getLastError().c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		fileLoaded(scrollToPage);
		return false;
	}
	else
	{
		this->doc->lock();
		this->doc->clearDocument();
		*this->doc = *tmp;
		this->doc->unlock();
	}

	fileLoaded(scrollToPage);
	return true;
}

void Control::fileLoaded(int scrollToPage)
{
	XOJ_CHECK_TYPE(Control);

	this->doc->lock();
	path file = this->doc->getEvMetadataFilename();
	this->doc->unlock();

	if (!file.empty())
	{
		MetadataEntry md = metadata->getForFile(file.c_str());
		if (!md.valid)
		{
			md.zoom = -1;
			md.page = 0;
		}

		if (scrollToPage >= 0)
		{
			md.page = scrollToPage;
		}

		loadMetadata(md);
		recent->addRecentFileFilename(file);
	}
	else
	{
		this->zoom->zoomFit();
	}

	updateWindowTitle();
	win->updateLayerCombobox();
	win->getXournal()->forceUpdatePagenumbers();
	getCursor()->updateCursor();
	updateDeletePageButton();
}

class MetadataCallbackData {
public:
	Control* ctrl;
	MetadataEntry md;
};

/**
 * Load the data after processing the document...
 */
bool Control::loadMetadataCallback(MetadataCallbackData* data)
{
	if (!data->md.valid)
	{
		delete data;
		return false;
	}

	data->ctrl->scrollHandler->scrollToPage(data->md.page);
	data->ctrl->zoom->setZoom(data->md.zoom);

	delete data;

	// Do not call again!
	return false;
}

void Control::loadMetadata(MetadataEntry md)
{
	MetadataCallbackData* data = new MetadataCallbackData();
	data->md = md;
	data->ctrl = this;

	g_idle_add((GSourceFunc) loadMetadataCallback, data);
}

bool Control::annotatePdf(path filename, bool attachPdf, bool attachToDocument)
{
	XOJ_CHECK_TYPE(Control);

	if (!this->close())
	{
		return false;
	}

	if (filename.empty())
	{
		XojOpenDlg dlg((GtkWindow*) *win, this->settings);
		filename = dlg.showOpenDialog(true, attachToDocument);
		if (filename.empty())
		{
			return false;
		}
	}

	getCursor()->setCursorBusy(true);

	bool res = this->doc->readPdf(filename, true, attachToDocument);

	if (res)
	{
		this->recent->addRecentFileFilename(filename.c_str());

		this->doc->lock();
		path file = this->doc->getEvMetadataFilename();
		this->doc->unlock();
		MetadataEntry md = metadata->getForFile(file.c_str());
		loadMetadata(md);
	}
	else
	{
		this->doc->lock();
		string errMsg = doc->getLastErrorMsg();
		this->doc->unlock();

		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *win,
												   GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
												   GTK_BUTTONS_OK, "%s",
												   FC(_F("Error annotate PDF file \"{1}\"\n{2}") % filename % errMsg));
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	getCursor()->setCursorBusy(false);

	fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

	getCursor()->updateCursor();

	return true;
}

void Control::print()
{
	XOJ_CHECK_TYPE(Control);

	PrintHandler print;
	this->doc->lock();
	print.print(this->doc, getCurrentPageNo());
	this->doc->unlock();
}

void Control::block(string name)
{
	XOJ_CHECK_TYPE(Control);

	if (this->isBlocking)
	{
		return;
	}

	// Disable all gui Control, to get full control over the application
	win->setControlTmpDisabled(true);
	getCursor()->setCursorBusy(true);
	disableSidebarTmp(true);

	this->statusbar = this->win->get("statusbar");
	this->lbState = GTK_LABEL(this->win->get("lbState"));
	this->pgState = GTK_PROGRESS_BAR(this->win->get("pgState"));

	gtk_label_set_text(this->lbState, name.c_str());
	gtk_widget_show(this->statusbar);

	this->maxState = 100;
	this->isBlocking = true;
}

void Control::unblock()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->isBlocking)
	{
		return;
	}

	this->win->setControlTmpDisabled(false);
	getCursor()->setCursorBusy(false);
	disableSidebarTmp(false);

	gtk_widget_hide(this->statusbar);

	this->isBlocking = false;
}

void Control::setMaximumState(int max)
{
	XOJ_CHECK_TYPE(Control);

	this->maxState = max;
}

void Control::setCurrentState(int state)
{
	XOJ_CHECK_TYPE(Control);

	gdk_threads_enter();
	gtk_progress_bar_set_fraction(this->pgState, gdouble(state) / this->maxState);
	gdk_threads_leave();
}

bool Control::save(bool synchron)
{
	XOJ_CHECK_TYPE(Control);

	//clear selection before saving
	clearSelectionEndText();

	this->doc->lock();
	path filename = this->doc->getFilename();
	this->doc->unlock();

	if (filename.empty())
	{
		if (!showSaveDialog())
		{
			return false;
		}
	}

	SaveJob* job = new SaveJob(this);
	bool result = true;
	if (synchron)
	{
		result = job->save();
		unblock();
	}
	else
	{
		this->scheduler->addJob(job, JOB_PRIORITY_URGENT);
	}
	job->unref();

	return result;
}

bool Control::showSaveDialog()
{
	XOJ_CHECK_TYPE(Control);

	GtkWidget* dialog = gtk_file_chooser_dialog_new(_C("Save File"), (GtkWindow*) *win,
													GTK_FILE_CHOOSER_ACTION_SAVE, _C("_Cancel"), GTK_RESPONSE_CANCEL,
													_C("_Save"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _C("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

	if (!settings->getLastSavePath().empty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	string saveFilename = "";

	this->doc->lock();
	if (!doc->getFilename().empty())
	{
		saveFilename = doc->getFilename().filename().string();
	}
	else if (!doc->getPdfFilename().empty())
	{
		saveFilename = doc->getPdfFilename().filename().replace_extension(".pdf.xoj").string();
	}
	else
	{
		time_t curtime = time(NULL);
		char stime[128];
		strftime(stime, sizeof(stime), settings->getDefaultSaveName().c_str(), localtime(&curtime));

		saveFilename = stime;
	}
	this->doc->unlock();

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), saveFilename.c_str());
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), true);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return false;
	}

	char* name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	string filename = name;
	char* folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	this->doc->lock();

	this->doc->setFilename(path(filename));
	this->doc->unlock();

	return true;
}

void Control::updateWindowTitle()
{
	XOJ_CHECK_TYPE(Control);

	string title = "";

	this->doc->lock();
	if (doc->getFilename().empty())
	{
		if (doc->getPdfFilename().empty())
		{
			title = _("Unsaved Document");
		}
		else
		{
			if (undoRedo->isChanged())
			{
				title += "*";
			}
			title += doc->getPdfFilename().string();
		}
	}
	else
	{
		if (undoRedo->isChanged())
		{
			title += "*";
		}

		title += doc->getFilename().filename().string();
	}
	this->doc->unlock();

	title += " - Xournal++";

	gtk_window_set_title((GtkWindow*) *win, title.c_str());
}

void Control::exportAsPdf()
{
	XOJ_CHECK_TYPE(Control);

	exportBase(new PdfExportJob(this));
}

void Control::exportAs()
{
	XOJ_CHECK_TYPE(Control);
	exportBase(new CustomExportJob(this));
}

void Control::exportBase(BaseExportJob* job)
{
	if (job->showFilechooser())
	{
		this->scheduler->addJob(job, JOB_PRIORITY_NONE);
	}
	else
	{
		// The job blocked, so we have to unblock, because the job unblocks only after run
		unblock();
	}
	job->unref();
}

bool Control::saveAs()
{
	XOJ_CHECK_TYPE(Control);

	if (!showSaveDialog())
	{
		return false;
	}
	this->doc->lock();
	path filename = doc->getFilename();
	this->doc->unlock();

	if (filename.empty())
	{
		return false;
	}

	// no lock needed, this is an uncritical operation
	this->doc->setCreateBackupOnSave(false);
	return save();
}

void Control::quit()
{
	XOJ_CHECK_TYPE(Control);

	if (!this->close(true))
	{
		return;
	}

	this->scheduler->lock();

	settings->save();

	this->scheduler->removeAllJobs();
	this->scheduler->unlock();
	gtk_main_quit();
}

bool Control::close(bool destroy)
{
	XOJ_CHECK_TYPE(Control);

	clearSelectionEndText();
	metadata->documentChanged();

	if (undoRedo->isChanged())
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
												   _C("This document is not saved yet."));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Save"), 1);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Discard"), 2);
		gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Cancel"), 3);
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
		int resNotSaved = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		// save
		if (resNotSaved == 1)
		{
			if (this->save(true))
			{
				return true;
			}
			else
			{
				// if not saved cancel, else close
				return false;
			}
		}

		// cancel or closed
		if (resNotSaved != 2) // 2 = discard
		{
			return false;
		}
	}
	
	if (!doc->getFilename().empty())
	{
		namespace bf = boost::filesystem;
		if (!bf::exists(this->doc->getFilename()))
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT,
													   GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE, "%s",
													   _C("Document file was removed."));

			gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Save As"), 1);
			gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Discard"), 2);
			gtk_dialog_add_button(GTK_DIALOG(dialog), _C("Cancel"), 3);
			gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
			int resDocRemoved = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (resDocRemoved == 1)
			{
				if (this->saveAs())
				{
					return true;
				}
				else
				{
					// if not saved cancel, else close
					return false;
				}
			}

			// cancel or closed
			if (resDocRemoved != 2) // 2 = discard
			{
				return false;
			}
		}
	}

	if (destroy)
	{
		undoRedo->clearContents();

		this->doc->lock();
		this->doc->clearDocument(destroy);
		this->doc->unlock();

		//updateWindowTitle();
		undoRedoChanged();
	}
	return true;
}

void Control::resetShapeRecognizer()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win)
	{
		this->win->getXournal()->resetShapeRecognizer();
	}
}

void Control::showAbout()
{
	XOJ_CHECK_TYPE(Control);

	AboutDialog dlg(this->gladeSearchPath);
	dlg.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::clipboardCutCopyEnabled(bool enabled)
{
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_CUT, enabled);
	fireEnableAction(ACTION_COPY, enabled);
}

void Control::clipboardPasteEnabled(bool enabled)
{
	XOJ_CHECK_TYPE(Control);

	fireEnableAction(ACTION_PASTE, enabled);
}

void Control::clipboardPasteText(string text)
{
	XOJ_CHECK_TYPE(Control);

	Text* t = new Text();
	t->setText(text);
	t->setFont(settings->getFont());
	t->setColor(toolHandler->getColor());

	clipboardPaste(t);
}

void Control::clipboardPasteImage(GdkPixbuf* img)
{
	XOJ_CHECK_TYPE(Control);

	Image* image = new Image();
	image->setImage(img);

	int width = gdk_pixbuf_get_width(img);
	int height = gdk_pixbuf_get_height(img);

	int pageNr = getCurrentPageNo();
	if (pageNr == -1)
	{
		return;
	}

	this->doc->lock();
	PageRef page = this->doc->getPage(pageNr);
	int pageWidth = page->getWidth();
	int pageHeight = page->getHeight();
	this->doc->unlock();

	// Size: 3/4 of the page size
	pageWidth = pageWidth * 3 / 4;
	pageHeight = pageHeight * 3 / 4;

	int scaledWidth = width;
	int scaledHeight = height;

	if (width > pageWidth)
	{
		scaledWidth = pageWidth;
		scaledHeight = (scaledWidth * height) / width;
	}

	if (scaledHeight > pageHeight)
	{
		scaledHeight = pageHeight;
		scaledWidth = (scaledHeight * width) / height;
	}

	image->setWidth(scaledWidth);
	image->setHeight(scaledHeight);

	clipboardPaste(image);
}

void Control::clipboardPasteTex(GdkPixbuf* img, const char* text, int textLength)
{
	XOJ_CHECK_TYPE(Control);

	TexImage* image = new TexImage();
	image->setImage(img);

	int width = gdk_pixbuf_get_width(img);
	int height = gdk_pixbuf_get_height(img);

	image->setWidth(width);
	image->setHeight(height);
	image->setText(string(text, textLength));

	clipboardPaste(image);
}

void Control::clipboardPaste(Element* e)
{
	XOJ_CHECK_TYPE(Control);

	double x = 0;
	double y = 0;
	int pageNr = getCurrentPageNo();
	if (pageNr == -1)
	{
		return;
	}

	XojPageView* view = win->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		return;
	}

	this->doc->lock();
	PageRef page = this->doc->getPage(pageNr);
	Layer* layer = page->getSelectedLayer();
	win->getXournal()->getPasteTarget(x, y);

	double width = e->getElementWidth();
	double height = e->getElementHeight();

	e->setX(x - width / 2);
	e->setY(y - height / 2);
	layer->addElement(e);

	this->doc->unlock();

	undoRedo->addUndoAction(new InsertUndoAction(page, layer, e));

	EditSelection* selection = new EditSelection(this->undoRedo, e, view, page);

	win->getXournal()->setSelection(selection);
}

void Control::clipboardPasteXournal(ObjectInputStream& in)
{
	XOJ_CHECK_TYPE(Control);

	int pNr = getCurrentPageNo();
	if (pNr == -1 && win != NULL)
	{
		return;
	}

	this->doc->lock();
	PageRef page = this->doc->getPage(pNr);
	Layer* layer = page->getSelectedLayer();

	XojPageView* view = win->getXournal()->getViewFor(pNr);

	if (!view || !page)
	{
		this->doc->unlock();
		return;
	}

	EditSelection* selection = NULL;
	Element* element = NULL;
	try
	{
		string version = in.readString();
		if (version != PROJECT_STRING)
		{
			g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PROJECT_STRING);
		}

		selection = new EditSelection(this->undoRedo, page, view);
		in >> selection;

		// document lock not needed anymore, because we don't change the document, we only change the selection
		this->doc->unlock();

		int count = in.readInt();

		AddUndoAction* pasteAddUndoAction = new AddUndoAction(page, false);
		//this will undo a group of elements that are inserted

		for (int i = 0; i < count; i++)
		{
			string name = in.getNextObjectName();
			element = NULL;

			if (name == "Stroke")
			{
				element = new Stroke();
			}
			else if (name == "Image")
			{
				element = new Image();
			}
			else if (name == "TexImage")
			{
				element = new TexImage();
			}
			else if (name == "Text")
			{
				element = new Text();
			}
			else
			{
				throw INPUT_STREAM_EXCEPTION("Get unknown object {1}", name);
			}

			in >> element;

			//undoRedo->addUndoAction(new InsertUndoAction(page, layer, element, view));
			pasteAddUndoAction->addElement(layer, element, layer->indexOf(element));
			selection->addElement(element);
			element = NULL;
		}
		undoRedo->addUndoAction(pasteAddUndoAction);

		win->getXournal()->setSelection(selection);

	}
	catch (std::exception& e)
	{
		g_warning("could not paste, Exception occurred: %s", e.what());
		Stacktrace::printStracktrace();

		// cleanup
		if (element)
		{
			delete element;
		}

		if (selection)
		{
			for (Element* e : *selection->getElements()) delete e;
			delete selection;
		}
	}
}

void Control::deleteSelection()
{
	XOJ_CHECK_TYPE(Control);

	if (win)
	{
		win->getXournal()->deleteSelection();
	}
}

void Control::clearSelection()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win)
	{
		this->win->getXournal()->clearSelection();
	}
}

void Control::setClipboardHandlerSelection(EditSelection* selection)
{
	XOJ_CHECK_TYPE(Control);

	if (this->clipboardHandler)
	{
		this->clipboardHandler->setSelection(selection);
	}
}

void Control::setCopyPasteEnabled(bool enabled)
{
	XOJ_CHECK_TYPE(Control);

	this->clipboardHandler->setCopyPasteEnabled(enabled);
}

void Control::setToolSize(ToolSize size)
{
	XOJ_CHECK_TYPE(Control);

	EditSelection* sel = NULL;
	if (this->win)
	{
		sel = this->win->getXournal()->getSelection();
	}

	if (sel)
	{
		UndoAction* undo = sel->setSize(size, toolHandler->getToolThickness(TOOL_PEN),
										toolHandler->getToolThickness(TOOL_HILIGHTER),
										toolHandler->getToolThickness(TOOL_ERASER));
		if (undo)
		{
			undoRedo->addUndoAction(undo);
		}
	}
	this->toolHandler->setSize(size);
}

void Control::fontChanged()
{
	XOJ_CHECK_TYPE(Control);

	XojFont font = win->getFontButtonFont();
	settings->setFont(font);

	EditSelection* sel = NULL;
	if (this->win)
	{
		sel = this->win->getXournal()->getSelection();
	}
	if (sel)
	{
		UndoAction* undo = sel->setFont(font);
		if (undo)
		{
			undoRedo->addUndoAction(undo);
		}
	}

	TextEditor* editor = getTextEditor();
	if (editor)
	{
		editor->setFont(font);
	}
}

//The core handler for inserting latex
void Control::runLatex()
{
	XOJ_CHECK_TYPE(Control);

#ifdef ENABLE_MATHTEX
	this->doc->lock();

	int pageNr = getCurrentPageNo();
	if (pageNr == -1)
	{
		return;
	}
	XojPageView* view = win->getXournal()->getViewFor(pageNr);
	if (view == NULL)
	{
		return;
	}
	//we get the selection
	PageRef page = this->doc->getPage(pageNr);
	Layer* layer = page->getSelectedLayer();

	TexImage* img = view->getSelectedTex();

	double imgx = 10;
	double imgy = 10;
	double imgheight = 0;
	double imgwidth = 0;
	string imgTex;
	if (img)
	{
		//this will get the position of the Latex properly
		EditSelection* theSelection = win->getXournal()->getSelection();
		//imgx = img->getX();
		//imgy = img->getY();
		imgx = theSelection->getXOnView();
		imgy = theSelection->getYOnView();

		imgheight = img->getElementHeight();
		imgwidth = img->getElementWidth();
		//fix this typecast:
		imgTex = img->getText();
	}

	//now call the image handlers
	this->doc->unlock();

	//need to do this otherwise we can't remove the image for its replacement
	clearSelectionEndText();

	LatexGlade* mytex = new LatexGlade(this->gladeSearchPath);
	//determine if we should set a specific string
	mytex->setTex(imgTex);
	mytex->show(GTK_WINDOW(this->win->getWindow()));
	string tmp = mytex->getTex();
	delete mytex;
	cout << tmp << endl;

	if (tmp.empty())
	{
		return;
	}
	if (img)
	{
		layer->removeElement((Element*) img, false);
		view->rerenderElement(img);
		delete img;
		img = NULL;
	}

	//now do all the LatexAction stuff
	LatexAction texAction(tmp, imgheight * imgwidth);
	texAction.runCommand();

	this->doc->lock();

	GFile* mygfile = g_file_new_for_path(texAction.getFileName().c_str());
	cout << "About to insert image...";
	GError* err = NULL;
	GFileInputStream* in = g_file_read(mygfile, NULL, &err);
	g_object_unref(mygfile);
	if (err)
	{
		this->doc->unlock();

		cerr << _F("Could not retrieve LaTeX image file: {1}") % err->message << endl;

		g_error_free(err);
		return;
	}

	GdkPixbuf* pixbuf = NULL;
	pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(in), NULL, &err);
	g_input_stream_close(G_INPUT_STREAM(in), NULL, NULL);

	img = new TexImage();
	img->setX(imgx);
	img->setY(imgy);
	img->setImage(pixbuf);
	img->setText(tmp);

	if (imgheight)
	{
		double ratio = (gdouble) gdk_pixbuf_get_width(pixbuf) / gdk_pixbuf_get_height(pixbuf);
		if (ratio == 0)
		{
			if (imgwidth == 0)
			{
				img->setWidth(10);
			}
			else
			{
				img->setWidth(imgwidth);
			}
		}
		else
		{
			img->setWidth(imgheight * ratio);
		}
		img->setHeight(imgheight);
	}
	else
	{
		img->setWidth(gdk_pixbuf_get_width(pixbuf));
		img->setHeight(gdk_pixbuf_get_height(pixbuf));
	}

	layer->addElement(img);
	view->rerenderElement(img);


	cout << "Image inserted!" << endl;

	this->doc->unlock();

	undoRedo->addUndoAction(new InsertUndoAction(page, layer, img));
#else
	cout << "Mathtex is disabled. Recompile with ./configure --enable-mathtex, "
			"ensuring you have the mathtex command on your system." << endl;
#endif // ENABLE_MATHTEX

}

/**
 * GETTER / SETTER
 */

UndoRedoHandler* Control::getUndoRedoHandler()
{
	XOJ_CHECK_TYPE(Control);

	return this->undoRedo;
}

ZoomControl* Control::getZoomControl()
{
	XOJ_CHECK_TYPE(Control);

	return this->zoom;
}

Cursor* Control::getCursor()
{
	XOJ_CHECK_TYPE(Control);

	return this->cursor;
}

RecentManager* Control::getRecentManager()
{
	XOJ_CHECK_TYPE(Control);

	return this->recent;
}

Document* Control::getDocument()
{
	XOJ_CHECK_TYPE(Control);

	return this->doc;
}

ToolHandler* Control::getToolHandler()
{
	XOJ_CHECK_TYPE(Control);

	return this->toolHandler;
}

XournalScheduler* Control::getScheduler()
{
	XOJ_CHECK_TYPE(Control);

	return this->scheduler;
}

MainWindow* Control::getWindow()
{
	XOJ_CHECK_TYPE(Control);

	return this->win;
}

bool Control::isFullscreen()
{
	XOJ_CHECK_TYPE(Control);

	return this->fullscreen;
}

TextEditor* Control::getTextEditor()
{
	XOJ_CHECK_TYPE(Control);

	if (this->win)
	{
		return this->win->getXournal()->getTextEditor();
	}
	return NULL;
}

GladeSearchpath* Control::getGladeSearchPath()
{
	XOJ_CHECK_TYPE(Control);

	return this->gladeSearchPath;
}

Settings* Control::getSettings()
{
	XOJ_CHECK_TYPE(Control);

	return settings;
}

ScrollHandler* Control::getScrollHandler()
{
	XOJ_CHECK_TYPE(Control);

	return this->scrollHandler;
}

MetadataManager* Control::getMetadataManager()
{
	XOJ_CHECK_TYPE(Control);

	return this->metadata;
}

Sidebar* Control::getSidebar()
{
	XOJ_CHECK_TYPE(Control);

	return this->sidebar;
}

SearchBar* Control::getSearchBar()
{
	XOJ_CHECK_TYPE(Control);

	return this->searchBar;
}
