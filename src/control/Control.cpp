#include "Control.h"

#include <ctime>
#include <fstream>
#include <memory>
#include <numeric>
#include <sstream>
#include <utility>

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include "gui/TextEditor.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "gui/dialog/AboutDialog.h"
#include "gui/dialog/FillTransparencyDialog.h"
#include "gui/dialog/FormatDialog.h"
#include "gui/dialog/GotoDialog.h"
#include "gui/dialog/PageTemplateDialog.h"
#include "gui/dialog/SelectBackgroundColorDialog.h"
#include "gui/dialog/SettingsDialog.h"
#include "gui/dialog/ToolbarManageDialog.h"
#include "gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"
#include "gui/inputdevices/HandRecognition.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"
#include "jobs/AutosaveJob.h"
#include "jobs/BlockingJob.h"
#include "jobs/CustomExportJob.h"
#include "jobs/PdfExportJob.h"
#include "jobs/SaveJob.h"
#include "layer/LayerController.h"
#include "model/BackgroundImage.h"
#include "model/FormatDefinitions.h"
#include "model/StrokeStyle.h"
#include "model/XojPage.h"
#include "pagetype/PageTypeHandler.h"
#include "pagetype/PageTypeMenu.h"
#include "plugin/PluginController.h"
#include "serializing/ObjectInputStream.h"
#include "settings/ButtonConfig.h"
#include "stockdlg/XojOpenDlg.h"
#include "undo/AddUndoAction.h"
#include "undo/DeleteUndoAction.h"
#include "undo/InsertDeletePageUndoAction.h"
#include "undo/InsertUndoAction.h"
#include "view/DocumentView.h"
#include "view/TextView.h"
#include "xojfile/LoadHandler.h"

#include "CrashHandler.h"
#include "FullscreenHandler.h"
#include "LatexController.h"
#include "PageBackgroundChangeController.h"
#include "PathUtil.h"
#include "PrintHandler.h"
#include "Stacktrace.h"
#include "StringUtils.h"
#include "UndoRedoController.h"
#include "Util.h"
#include "XojMsgBox.h"
#include "config-dev.h"
#include "config-features.h"
#include "config.h"
#include "i18n.h"


Control::Control(GladeSearchpath* gladeSearchPath) {
    this->recent = new RecentManager();
    this->undoRedo = new UndoRedoHandler(this);
    this->recent->addListener(this);
    this->undoRedo->addUndoRedoListener(this);
    this->isBlocking = false;

    this->gladeSearchPath = gladeSearchPath;

    this->metadata = new MetadataManager();
    this->cursor = new XournalppCursor(this);

    this->lastAction = ACTION_NONE;
    this->lastGroup = GROUP_NOGROUP;
    this->lastEnabled = false;

    auto name = Util::getConfigFile(SETTINGS_XML_FILE);
    this->settings = new Settings(std::move(name));
    this->settings->load();

    this->applyPreferredLanguage();

    TextView::setDpi(settings->getDisplayDpi());

    this->pageTypes = new PageTypeHandler(gladeSearchPath);
    this->newPageType = new PageTypeMenu(this->pageTypes, settings, true, true);

    this->audioController = new AudioController(this->settings, this);

    this->scrollHandler = new ScrollHandler(this);

    this->scheduler = new XournalScheduler();

    this->doc = new Document(this);

    // for crashhandling
    setEmergencyDocument(this->doc);

    this->zoom = new ZoomControl();
    this->zoom->setZoomStep(this->settings->getZoomStep() / 100.0);
    this->zoom->setZoomStepScroll(this->settings->getZoomStepScroll() / 100.0);
    this->zoom->setZoom100Value(this->settings->getDisplayDpi() / Util::DPI_NORMALIZATION_FACTOR);

    this->toolHandler = new ToolHandler(this, this, this->settings);
    this->toolHandler->loadSettings();

    /**
     * This is needed to update the previews
     */
    this->changeTimout = g_timeout_add_seconds(5, reinterpret_cast<GSourceFunc>(checkChangedDocument), this);

    this->pageBackgroundChangeController = new PageBackgroundChangeController(this);

    this->layerController = new LayerController(this);
    this->layerController->registerListener(this);

    this->fullscreenHandler = new FullscreenHandler(settings);

    this->pluginController = new PluginController(this);
    this->pluginController->registerToolbar();
}

Control::~Control() {
    g_source_remove(this->changeTimout);
    this->enableAutosave(false);

    deleteLastAutosaveFile("");

    this->scheduler->stop();

    this->changedPages.clear();  // can be removed, will be done by implicit destructor

    delete this->pluginController;
    this->pluginController = nullptr;
    delete this->clipboardHandler;
    this->clipboardHandler = nullptr;
    delete this->recent;
    this->recent = nullptr;
    delete this->undoRedo;
    this->undoRedo = nullptr;
    delete this->settings;
    this->settings = nullptr;
    delete this->toolHandler;
    this->toolHandler = nullptr;
    delete this->sidebar;
    this->sidebar = nullptr;
    delete this->doc;
    this->doc = nullptr;
    delete this->searchBar;
    this->searchBar = nullptr;
    delete this->scrollHandler;
    this->scrollHandler = nullptr;
    delete this->newPageType;
    this->newPageType = nullptr;
    delete this->pageTypes;
    this->pageTypes = nullptr;
    delete this->metadata;
    this->metadata = nullptr;
    delete this->cursor;
    this->cursor = nullptr;
    delete this->zoom;
    this->zoom = nullptr;
    delete this->scheduler;
    this->scheduler = nullptr;
    delete this->dragDropHandler;
    this->dragDropHandler = nullptr;
    delete this->audioController;
    this->audioController = nullptr;
    delete this->pageBackgroundChangeController;
    this->pageBackgroundChangeController = nullptr;
    delete this->layerController;
    this->layerController = nullptr;
    delete this->fullscreenHandler;
    this->fullscreenHandler = nullptr;
}

void Control::renameLastAutosaveFile() {
    if (this->lastAutosaveFilename.empty()) {
        return;
    }

    auto const& filename = this->lastAutosaveFilename;
    auto renamed = Util::getAutosaveFilepath();
    Util::clearExtensions(renamed);
    if (!filename.empty() && filename.string().front() != '.') {
        // This file must be a fresh, unsaved document. Since this file is
        // already in ~/.xournalpp/autosave/, we need to change the renamed filename.
        renamed += ".old.autosave.xopp";
    } else {
        // The file is a saved document with the form ".<filename>.autosave.xopp"
        renamed += filename.filename();
    }

    g_message("%s", FS(_F("Autosave renamed from {1} to {2}") % this->lastAutosaveFilename.string() % renamed.string())
                            .c_str());

    if (!fs::exists(filename)) {
        this->save(false);
    }

    std::vector<string> errors;
    try {
        Util::safeRenameFile(filename, renamed);
    } catch (fs::filesystem_error const& e) {
        auto fmtstr = _F("Could not rename autosave file from \"{1}\" to \"{2}\": {3}");
        errors.emplace_back(FS(fmtstr % filename.string() % renamed.string() % e.what()));
    }


    if (!errors.empty()) {
        string error = std::accumulate(errors.begin() + 1, errors.end(), *errors.begin(),
                                       [](const string& e1, const string& e2) { return e1 + "\n" + e2; });
        Util::execInUiThread([=]() {
            string msg = FS(_F("Autosave failed with an error: {1}") % error);
            XojMsgBox::showErrorToUser(getGtkWindow(), msg);
        });
    }
}

void Control::setLastAutosaveFile(fs::path newAutosaveFile) { this->lastAutosaveFilename = std::move(newAutosaveFile); }

void Control::deleteLastAutosaveFile(fs::path newAutosaveFile) {
    fs::remove(this->lastAutosaveFilename);
    this->lastAutosaveFilename = std::move(newAutosaveFile);
}

auto Control::checkChangedDocument(Control* control) -> bool {
    if (!control->doc->tryLock()) {
        // call again later
        return true;
    }
    for (auto const& page: control->changedPages) {
        int p = control->doc->indexOf(page);
        if (p != -1) {
            control->firePageChanged(p);
        }
    }
    control->changedPages.clear();
    control->doc->unlock();

    // Call again
    return true;
}

void Control::saveSettings() {
    this->toolHandler->saveSettings();

    gint width = 0;
    gint height = 0;
    gtk_window_get_size(getGtkWindow(), &width, &height);

    if (!this->win->isMaximized()) {
        this->settings->setMainWndSize(width, height);
    }
    this->settings->setMainWndMaximized(this->win->isMaximized());

    this->sidebar->saveSize();
}

void Control::initWindow(MainWindow* win) {
    win->setRecentMenu(recent->getMenu());
    selectTool(toolHandler->getToolType());
    this->win = win;
    this->sidebar = new Sidebar(win, this);

    XojMsgBox::setDefaultWindow(getGtkWindow());

    updatePageNumbers(0, npos);

    toolHandler->eraserTypeChanged();

    this->searchBar = new SearchBar(this);

    // Disable undo buttons
    undoRedoChanged();

    if (settings->isPresentationMode()) {
        setViewPresentationMode(true);
    } else if (settings->isViewFixedRows()) {
        setViewRows(settings->getViewRows());
    } else {
        setViewColumns(settings->getViewColumns());
    }

    setViewLayoutVert(settings->getViewLayoutVert());
    setViewLayoutR2L(settings->getViewLayoutR2L());
    setViewLayoutB2T(settings->getViewLayoutB2T());

    setViewPairedPages(settings->isShowPairedPages());

    penSizeChanged();
    eraserSizeChanged();
    hilighterSizeChanged();
    updateDeletePageButton();
    toolFillChanged();
    toolLineStyleChanged();

    this->clipboardHandler = new ClipboardHandler(this, win->getXournal()->getWidget());

    this->enableAutosave(settings->isAutosaveEnabled());

    win->setFontButtonFont(settings->getFont());

    this->pluginController->registerMenu();

    fireActionSelected(GROUP_SNAPPING, settings->isSnapRotation() ? ACTION_ROTATION_SNAPPING : ACTION_NONE);
    fireActionSelected(GROUP_GRID_SNAPPING, settings->isSnapGrid() ? ACTION_GRID_SNAPPING : ACTION_NONE);
}

auto Control::autosaveCallback(Control* control) -> bool {
    if (!control->undoRedo->isChangedAutosave()) {
        // do nothing, nothing changed
        return true;
    }


    g_message("Info: autosave document...");


    auto* job = new AutosaveJob(control);
    control->scheduler->addJob(job, JOB_PRIORITY_NONE);
    job->unref();

    return true;
}

void Control::enableAutosave(bool enable) {
    if (this->autosaveTimeout) {
        g_source_remove(this->autosaveTimeout);
        this->autosaveTimeout = 0;
    }

    if (enable) {
        int timeout = settings->getAutosaveTimeout() * 60;
        this->autosaveTimeout = g_timeout_add_seconds(timeout, reinterpret_cast<GSourceFunc>(autosaveCallback), this);
    }
}

void Control::updatePageNumbers(size_t page, size_t pdfPage) {
    if (this->win == nullptr) {
        return;
    }

    this->win->updatePageNumbers(page, this->doc->getPageCount(), pdfPage);
    this->sidebar->selectPageNr(page, pdfPage);

    this->metadata->storeMetadata(this->doc->getEvMetadataFilename(), page, getZoomControl()->getZoomReal());

    int current = getCurrentPageNo();
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
                              GtkToolButton* toolbutton, bool enabled) {
    if (layerController->actionPerformed(type)) {
        return;
    }

    switch (type) {
            // Menu File
        case ACTION_NEW:
            clearSelectionEndText();
            newFile();
            break;
        case ACTION_OPEN:
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
            UndoRedoController::undo(this);
            break;
        case ACTION_REDO:
            UndoRedoController::redo(this);
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
        case ACTION_PAPER_FORMAT:
            paperFormat();
            break;
        case ACTION_CONFIGURE_PAGE_TEMPLATE:
            paperTemplate();
            break;
        case ACTION_PAPER_BACKGROUND_COLOR:
            changePageBackgroundColor();
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
        case ACTION_TOOL_PLAY_OBJECT:
            if (enabled) {
                selectTool(TOOL_PLAY_OBJECT);
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
        case ACTION_TOOL_FLOATING_TOOLBOX:
            if (enabled) {
                selectTool(TOOL_FLOATING_TOOLBOX);
            }
            break;
        case ACTION_TOOL_DRAW_RECT:
        case ACTION_TOOL_DRAW_CIRCLE:
        case ACTION_TOOL_DRAW_ARROW:
        case ACTION_TOOL_DRAW_COORDINATE_SYSTEM:
        case ACTION_RULER:
        case ACTION_TOOL_DRAW_SPLINE:
        case ACTION_SHAPE_RECOGNIZER:
            setShapeTool(type, enabled);
            break;

        case ACTION_TOOL_DEFAULT:
            if (enabled) {
                selectDefaultTool();
            }
            break;
        case ACTION_TOOL_FILL:
            setFill(enabled);
            break;

        case ACTION_SIZE_VERY_FINE:
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

        case ACTION_TOOL_LINE_STYLE_PLAIN:
            setLineStyle("plain");
            break;
        case ACTION_TOOL_LINE_STYLE_DASH:
            setLineStyle("dash");
            break;
        case ACTION_TOOL_LINE_STYLE_DASH_DOT:
            setLineStyle("dashdot");
            break;
        case ACTION_TOOL_LINE_STYLE_DOT:
            setLineStyle("dot");
            break;

        case ACTION_TOOL_ERASER_SIZE_VERY_FINE:
            if (enabled) {
                this->toolHandler->setEraserSize(TOOL_SIZE_VERY_FINE);
                eraserSizeChanged();
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
        case ACTION_TOOL_ERASER_SIZE_VERY_THICK:
            if (enabled) {
                this->toolHandler->setEraserSize(TOOL_SIZE_VERY_THICK);
                eraserSizeChanged();
            }
            break;
        case ACTION_TOOL_PEN_SIZE_VERY_FINE:
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
        case ACTION_TOOL_PEN_FILL:
            this->toolHandler->setPenFillEnabled(enabled);
            break;
        case ACTION_TOOL_PEN_FILL_TRANSPARENCY:
            selectFillAlpha(true);
            break;


        case ACTION_TOOL_HILIGHTER_SIZE_VERY_FINE:
            if (enabled) {
                this->toolHandler->setHilighterSize(TOOL_SIZE_VERY_FINE);
                hilighterSizeChanged();
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
        case ACTION_TOOL_HILIGHTER_SIZE_VERY_THICK:
            if (enabled) {
                this->toolHandler->setHilighterSize(TOOL_SIZE_VERY_THICK);
                hilighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HILIGHTER_FILL:
            this->toolHandler->setHilighterFillEnabled(enabled);
            break;
        case ACTION_TOOL_HILIGHTER_FILL_TRANSPARENCY:
            selectFillAlpha(false);
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
        case ACTION_TEX:
            runLatex();
            break;

            // Menu View
        case ACTION_ZOOM_100:
        case ACTION_ZOOM_FIT:
        case ACTION_ZOOM_IN:
        case ACTION_ZOOM_OUT:
            Util::execInUiThread([=]() { zoomCallback(type, enabled); });
            break;

        case ACTION_VIEW_PAIRED_PAGES:
            setViewPairedPages(enabled);
            break;

        case ACTION_VIEW_PRESENTATION_MODE:
            setViewPresentationMode(enabled);
            break;

        case ACTION_MANAGE_TOOLBAR:
            manageToolbars();
            break;

        case ACTION_CUSTOMIZE_TOOLBAR:
            customizeToolbars();
            break;

        case ACTION_FULLSCREEN:
            setFullscreen(enabled);
            break;

        case ACTION_SET_COLUMNS_1:
            setViewColumns(1);
            break;

        case ACTION_SET_COLUMNS_2:
            setViewColumns(2);
            break;

        case ACTION_SET_COLUMNS_3:
            setViewColumns(3);
            break;

        case ACTION_SET_COLUMNS_4:
            setViewColumns(4);
            break;

        case ACTION_SET_COLUMNS_5:
            setViewColumns(5);
            break;

        case ACTION_SET_COLUMNS_6:
            setViewColumns(6);
            break;

        case ACTION_SET_COLUMNS_7:
            setViewColumns(7);
            break;

        case ACTION_SET_COLUMNS_8:
            setViewColumns(8);
            break;

        case ACTION_SET_ROWS_1:
            setViewRows(1);
            break;

        case ACTION_SET_ROWS_2:
            setViewRows(2);
            break;

        case ACTION_SET_ROWS_3:
            setViewRows(3);
            break;

        case ACTION_SET_ROWS_4:
            setViewRows(4);
            break;

        case ACTION_SET_ROWS_5:
            setViewRows(5);
            break;

        case ACTION_SET_ROWS_6:
            setViewRows(6);
            break;

        case ACTION_SET_ROWS_7:
            setViewRows(7);
            break;

        case ACTION_SET_ROWS_8:
            setViewRows(8);
            break;

        case ACTION_SET_LAYOUT_HORIZONTAL:
            setViewLayoutVert(false);
            break;

        case ACTION_SET_LAYOUT_VERTICAL:
            setViewLayoutVert(true);
            break;

        case ACTION_SET_LAYOUT_L2R:
            setViewLayoutR2L(false);
            break;

        case ACTION_SET_LAYOUT_R2L:
            setViewLayoutR2L(true);
            break;

        case ACTION_SET_LAYOUT_T2B:
            setViewLayoutB2T(false);
            break;

        case ACTION_SET_LAYOUT_B2T:
            setViewLayoutB2T(true);
            break;


        case ACTION_AUDIO_RECORD: {
            bool result = false;
            if (enabled) {
                result = audioController->startRecording();
            } else {
                result = audioController->stopRecording();
            }

            if (!result) {
                Util::execInUiThread([=]() {
                    gtk_toggle_tool_button_set_active(reinterpret_cast<GtkToggleToolButton*>(toolbutton), !enabled);
                    string msg = _("Recorder could not be started.");
                    g_warning("%s", msg.c_str());
                    XojMsgBox::showErrorToUser(Control::getGtkWindow(), msg);
                });
            }
            break;
        }

        case ACTION_AUDIO_PAUSE_PLAYBACK:
            if (enabled) {
                this->getAudioController()->pausePlayback();
            } else {
                this->getAudioController()->continuePlayback();
            }
            break;

        case ACTION_AUDIO_SEEK_FORWARDS:
            this->getAudioController()->seekForwards();
            break;

        case ACTION_AUDIO_SEEK_BACKWARDS:
            this->getAudioController()->seekBackwards();
            break;

        case ACTION_AUDIO_STOP_PLAYBACK:
            this->getAudioController()->stopPlayback();
            break;

        case ACTION_ROTATION_SNAPPING:
            rotationSnappingToggle();
            break;

        case ACTION_GRID_SNAPPING:
            gridSnappingToggle();
            break;

            // Footer, not really an action, but need an identifier to
        case ACTION_FOOTER_PAGESPIN:
        case ACTION_FOOTER_ZOOM_SLIDER:
            // nothing to do here
            break;


            // Plugin menu
        case ACTION_PLUGIN_MANAGER:
            this->pluginController->showPluginManager();
            break;


            // Menu Help
        case ACTION_HELP:
            XojMsgBox::showHelp(getGtkWindow());
            break;
        case ACTION_ABOUT:
            showAbout();
            break;

        default:
            g_warning("Unhandled action event: %s / %s (%i / %i)", ActionType_toString(type).c_str(),
                      ActionGroup_toString(group).c_str(), type, group);
            Stacktrace::printStracktrace();
    }

    if (type >= ACTION_TOOL_PEN && type <= ACTION_TOOL_HAND) {
        auto at = static_cast<ActionType>(toolHandler->getToolType() - TOOL_PEN + ACTION_TOOL_PEN);
        if (type == at && !enabled) {
            fireActionSelected(GROUP_TOOL, at);
        }
    }
}

auto Control::copy() -> bool {
    if (this->win && this->win->getXournal()->copy()) {
        return true;
    }
    return this->clipboardHandler->copy();
}

auto Control::cut() -> bool {
    if (this->win && this->win->getXournal()->cut()) {
        return true;
    }
    return this->clipboardHandler->cut();
}

auto Control::paste() -> bool {
    if (this->win && this->win->getXournal()->paste()) {
        return true;
    }
    return this->clipboardHandler->paste();
}

void Control::selectFillAlpha(bool pen) {
    int alpha = 0;

    if (pen) {
        alpha = toolHandler->getPenFill();
    } else {
        alpha = toolHandler->getHilighterFill();
    }

    FillTransparencyDialog dlg(gladeSearchPath, alpha);
    dlg.show(getGtkWindow());

    if (dlg.getResultAlpha() == -1) {
        return;
    }

    alpha = dlg.getResultAlpha();

    if (pen) {
        toolHandler->setPenFill(alpha);
    } else {
        toolHandler->setHilighterFill(alpha);
    }
}

void Control::clearSelectionEndText() {
    clearSelection();
    if (win) {
        win->getXournal()->endTextAllPages();
    }
}

/**
 * Fire page selected, but first check if the page Number is valid
 *
 * @return the page ID or size_t_npos if the page is not found
 */
auto Control::firePageSelected(const PageRef& page) -> size_t {
    this->doc->lock();
    size_t pageId = this->doc->indexOf(page);
    this->doc->unlock();
    if (pageId == npos) {
        return npos;
    }

    DocumentHandler::firePageSelected(pageId);
    return pageId;
}

void Control::firePageSelected(size_t page) { DocumentHandler::firePageSelected(page); }

void Control::manageToolbars() {
    ToolbarManageDialog dlg(this->gladeSearchPath, this->win->getToolbarModel());
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    this->win->updateToolbarMenu();

    auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
    this->win->getToolbarModel()->save(filepath);
}

void Control::customizeToolbars() {
    g_return_if_fail(this->win != nullptr);

    if (this->win->getSelectedToolbar()->isPredefined()) {
        GtkWidget* dialog =
                gtk_message_dialog_new(getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s",
                                       FC(_F("The Toolbarconfiguration \"{1}\" is predefined, "
                                             "would you create a copy to edit?") %
                                          this->win->getSelectedToolbar()->getName()));

        gtk_window_set_transient_for(GTK_WINDOW(dialog), getGtkWindow());
        int res = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (res == -8)  // Yes
        {
            auto* data = new ToolbarData(*this->win->getSelectedToolbar());

            ToolbarModel* model = this->win->getToolbarModel();
            model->initCopyNameId(data);
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
    if (!this->dragDropHandler) {
        return;
    }

    this->dragDropHandler->clearToolbarsFromDragAndDrop();
}

void Control::startDragDropToolbar() {
    if (!this->dragDropHandler) {
        return;
    }

    this->dragDropHandler->prepareToolbarsForDragAndDrop();
}

auto Control::isInDragAndDropToolbar() -> bool {
    if (!this->dragDropHandler) {
        return false;
    }

    return this->dragDropHandler->isInDragAndDrop();
}

void Control::setShapeTool(ActionType type, bool enabled) {
    if (!enabled) {
        // Disable all entries
        this->toolHandler->setDrawingType(DRAWING_TYPE_DEFAULT);

        // fire disabled and return
        fireActionSelected(GROUP_RULER, ACTION_NONE);
        return;
    }

    // Check for nothing changed, and return in this case
    if ((this->toolHandler->getDrawingType() == DRAWING_TYPE_LINE && type == ACTION_RULER) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_RECTANGLE && type == ACTION_TOOL_DRAW_RECT) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_ARROW && type == ACTION_TOOL_DRAW_ARROW) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM &&
         type == ACTION_TOOL_DRAW_COORDINATE_SYSTEM) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_CIRCLE && type == ACTION_TOOL_DRAW_CIRCLE) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE && type == ACTION_TOOL_DRAW_SPLINE) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER && type == ACTION_SHAPE_RECOGNIZER)) {
        return;
    }

    switch (type) {
        case ACTION_TOOL_DRAW_RECT:
            this->toolHandler->setDrawingType(DRAWING_TYPE_RECTANGLE);
            break;

        case ACTION_TOOL_DRAW_CIRCLE:
            this->toolHandler->setDrawingType(DRAWING_TYPE_CIRCLE);
            break;

        case ACTION_TOOL_DRAW_ARROW:
            this->toolHandler->setDrawingType(DRAWING_TYPE_ARROW);
            break;

        case ACTION_TOOL_DRAW_COORDINATE_SYSTEM:
            this->toolHandler->setDrawingType(DRAWING_TYPE_COORDINATE_SYSTEM);
            break;

        case ACTION_RULER:
            this->toolHandler->setDrawingType(DRAWING_TYPE_LINE);
            break;

        case ACTION_TOOL_DRAW_SPLINE:
            this->toolHandler->setDrawingType(DRAWING_TYPE_SPLINE);
            break;

        case ACTION_SHAPE_RECOGNIZER:
            this->toolHandler->setDrawingType(DRAWING_TYPE_STROKE_RECOGNIZER);
            this->resetShapeRecognizer();
            break;

        default:
            g_warning("Invalid type for setShapeTool: %i", type);
            break;
    }

    fireActionSelected(GROUP_RULER, type);
}

void Control::setFullscreen(bool enabled) {
    fullscreenHandler->setFullscreen(win, enabled);

    fireActionSelected(GROUP_FULLSCREEN, enabled ? ACTION_FULLSCREEN : ACTION_NONE);
}

void Control::disableSidebarTmp(bool disabled) { this->sidebar->setTmpDisabled(disabled); }

void Control::addDefaultPage(string pageTemplate) {
    if (pageTemplate.empty()) {
        pageTemplate = settings->getPageTemplate();
    }

    PageTemplateSettings model;
    model.parse(pageTemplate);

    auto page = std::make_shared<XojPage>(model.getPageWidth(), model.getPageHeight());
    page->setBackgroundColor(model.getBackgroundColor());
    page->setBackgroundType(model.getBackgroundType());

    this->doc->lock();
    this->doc->addPage(std::move(page));
    this->doc->unlock();

    updateDeletePageButton();
}

void Control::updateDeletePageButton() {
    if (this->win) {
        GtkWidget* w = this->win->get("menuDeletePage");
        gtk_widget_set_sensitive(w, this->doc->getPageCount() > 1);
    }
}

void Control::deletePage() {
    clearSelectionEndText();
    // don't allow delete pages if we have less than 2 pages,
    // so we can be (more or less) sure there is at least one page.
    if (this->doc->getPageCount() < 2) {
        return;
    }

    size_t pNr = getCurrentPageNo();
    if (pNr == npos || pNr > this->doc->getPageCount()) {
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
    this->undoRedo->addUndoAction(std::make_unique<InsertDeletePageUndoAction>(page, pNr, false));

    if (pNr >= this->doc->getPageCount()) {
        pNr = this->doc->getPageCount() - 1;
    }

    scrollHandler->scrollToPage(pNr, 0);
    this->win->getXournal()->forceUpdatePagenumbers();
}

void Control::insertNewPage(size_t position) { pageBackgroundChangeController->insertNewPage(position); }

void Control::insertPage(const PageRef& page, size_t position) {
    this->doc->lock();
    this->doc->insertPage(page, position);
    this->doc->unlock();
    firePageInserted(position);

    getCursor()->updateCursor();

    int visibleHeight = 0;
    scrollHandler->isPageVisible(position, &visibleHeight);

    if (visibleHeight < 10) {
        Util::execInUiThread([=]() { scrollHandler->scrollToPage(position); });
    }
    firePageSelected(position);

    updateDeletePageButton();
    undoRedo->addUndoAction(std::make_unique<InsertDeletePageUndoAction>(page, position, true));
}

void Control::gotoPage() {
    auto* dlg = new GotoDialog(this->gladeSearchPath, this->doc->getPageCount());

    dlg->show(GTK_WINDOW(this->win->getWindow()));
    int page = dlg->getSelectedPage();

    if (page != -1) {
        this->scrollHandler->scrollToPage(page - 1, 0);
    }

    delete dlg;
}

void Control::updateBackgroundSizeButton() {
    if (this->win == nullptr) {
        return;
    }

    // Update paper color button
    auto const& p = getCurrentPage();
    if (!p || this->win == nullptr) {
        return;
    }
    GtkWidget* paperColor = win->get("menuJournalPaperColor");
    GtkWidget* pageSize = win->get("menuJournalPaperFormat");

    PageType bg = p->getBackgroundType();
    gtk_widget_set_sensitive(paperColor, !bg.isSpecial());

    // PDF page size is defined, you cannot change it
    gtk_widget_set_sensitive(pageSize, !bg.isPdfPage());
}

void Control::paperTemplate() {
    auto* dlg = new PageTemplateDialog(this->gladeSearchPath, settings, pageTypes);
    dlg->show(GTK_WINDOW(this->win->getWindow()));

    if (dlg->isSaved()) {
        newPageType->loadDefaultPage();
    }

    delete dlg;
}

void Control::paperFormat() {
    auto const& page = getCurrentPage();
    if (!page || page->getBackgroundType().isPdfPage()) {
        return;
    }
    clearSelectionEndText();

    auto* dlg = new FormatDialog(this->gladeSearchPath, settings, page->getWidth(), page->getHeight());
    dlg->show(GTK_WINDOW(this->win->getWindow()));

    double width = dlg->getWidth();
    double height = dlg->getHeight();

    if (width > 0) {
        this->doc->lock();
        Document::setPageSize(page, width, height);
        this->doc->unlock();
    }

    size_t pageNo = doc->indexOf(page);
    if (pageNo != npos && pageNo < doc->getPageCount()) {
        this->firePageSizeChanged(pageNo);
    }

    delete dlg;
}

void Control::changePageBackgroundColor() {
    int pNr = getCurrentPageNo();
    this->doc->lock();
    auto const& p = this->doc->getPage(pNr);
    this->doc->unlock();

    if (!p) {
        return;
    }

    clearSelectionEndText();

    PageType bg = p->getBackgroundType();
    if (bg.isSpecial()) {
        return;
    }

    SelectBackgroundColorDialog dlg(this);
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    if (auto optColor = dlg.getSelectedColor(); optColor) {
        p->setBackgroundColor(*optColor);
        firePageChanged(pNr);
    }
}

void Control::setViewPairedPages(bool enabled) {
    settings->setShowPairedPages(enabled);
    fireActionSelected(GROUP_PAIRED_PAGES, enabled ? ACTION_VIEW_PAIRED_PAGES : ACTION_NOT_SELECTED);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewPresentationMode(bool enabled) {
    if (enabled) {
        bool success = zoom->updateZoomPresentationValue();
        if (!success) {
            g_warning("Error calculating zoom value");
            fireActionSelected(GROUP_PRESENTATION_MODE, ACTION_NOT_SELECTED);
            return;
        }
    } else {
        if (settings->isViewFixedRows()) {
            setViewRows(settings->getViewRows());
        } else {
            setViewColumns(settings->getViewColumns());
        }

        setViewLayoutVert(settings->getViewLayoutVert());
        setViewLayoutR2L(settings->getViewLayoutR2L());
        setViewLayoutB2T(settings->getViewLayoutB2T());
    }
    zoom->setZoomPresentationMode(enabled);
    settings->setPresentationMode(enabled);

    // Disable Zoom
    fireEnableAction(ACTION_ZOOM_IN, !enabled);
    fireEnableAction(ACTION_ZOOM_OUT, !enabled);
    fireEnableAction(ACTION_ZOOM_FIT, !enabled);
    fireEnableAction(ACTION_ZOOM_100, !enabled);
    fireEnableAction(ACTION_FOOTER_ZOOM_SLIDER, !enabled);

    gtk_widget_set_sensitive(win->get("menuitemLayout"), !enabled);
    gtk_widget_set_sensitive(win->get("menuitemViewDimensions"), !enabled);

    // disable selection of scroll hand tool
    fireEnableAction(ACTION_TOOL_HAND, !enabled);
    fireActionSelected(GROUP_PRESENTATION_MODE, enabled ? ACTION_VIEW_PRESENTATION_MODE : ACTION_NOT_SELECTED);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setPairsOffset(int numOffset) {
    settings->setPairsOffset(numOffset);
    fireActionSelected(GROUP_PAIRED_PAGES, numOffset ? ACTION_SET_PAIRS_OFFSET : ACTION_NOT_SELECTED);
    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewColumns(int numColumns) {
    settings->setViewColumns(numColumns);
    settings->setViewFixedRows(false);

    ActionType action{};

    switch (numColumns) {
        case 1:
            action = ACTION_SET_COLUMNS_1;
            break;
        case 2:
            action = ACTION_SET_COLUMNS_2;
            break;
        case 3:
            action = ACTION_SET_COLUMNS_3;
            break;
        case 4:
            action = ACTION_SET_COLUMNS_4;
            break;
        case 5:
            action = ACTION_SET_COLUMNS_5;
            break;
        case 6:
            action = ACTION_SET_COLUMNS_6;
            break;
        case 7:
            action = ACTION_SET_COLUMNS_7;
            break;
        case 8:
            action = ACTION_SET_COLUMNS_8;
            break;
        default:
            action = ACTION_SET_COLUMNS;
    }

    fireActionSelected(GROUP_FIXED_ROW_OR_COLS, action);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewRows(int numRows) {
    settings->setViewRows(numRows);
    settings->setViewFixedRows(true);

    ActionType action{};

    switch (numRows) {
        case 1:
            action = ACTION_SET_ROWS_1;
            break;
        case 2:
            action = ACTION_SET_ROWS_2;
            break;
        case 3:
            action = ACTION_SET_ROWS_3;
            break;
        case 4:
            action = ACTION_SET_ROWS_4;
            break;
        case 5:
            action = ACTION_SET_ROWS_5;
            break;
        case 6:
            action = ACTION_SET_ROWS_6;
            break;
        case 7:
            action = ACTION_SET_ROWS_7;
            break;
        case 8:
            action = ACTION_SET_ROWS_8;
            break;
        default:
            action = ACTION_SET_ROWS;
    }

    fireActionSelected(GROUP_FIXED_ROW_OR_COLS, action);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewLayoutVert(bool vert) {
    settings->setViewLayoutVert(vert);

    ActionType action{};

    if (vert) {
        action = ACTION_SET_LAYOUT_VERTICAL;
    } else {
        action = ACTION_SET_LAYOUT_HORIZONTAL;
    }

    fireActionSelected(GROUP_LAYOUT_HORIZONTAL, action);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewLayoutR2L(bool r2l) {
    settings->setViewLayoutR2L(r2l);

    ActionType action{};

    if (r2l) {
        action = ACTION_SET_LAYOUT_R2L;
    } else {
        action = ACTION_SET_LAYOUT_L2R;
    }

    fireActionSelected(GROUP_LAYOUT_LR, action);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

void Control::setViewLayoutB2T(bool b2t) {
    settings->setViewLayoutB2T(b2t);

    ActionType action{};

    if (b2t) {
        action = ACTION_SET_LAYOUT_B2T;
    } else {
        action = ACTION_SET_LAYOUT_T2B;
    }

    fireActionSelected(GROUP_LAYOUT_TB, action);

    int currentPage = getCurrentPageNo();
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(currentPage);
}

/**
 * This callback is used by used to be called later in the UI Thread
 * On slower machine this feels more fluent, therefore this will not
 * be removed
 */
void Control::zoomCallback(ActionType type, bool enabled) {
    switch (type) {
        case ACTION_ZOOM_100:
            zoom->zoom100();
            break;
        case ACTION_ZOOM_FIT:
            if (enabled) {
                zoom->updateZoomFitValue();
            }
            // enable/disable ZoomFit
            zoom->setZoomFitMode(enabled);
            break;
        case ACTION_ZOOM_IN:
            zoom->zoomOneStep(ZOOM_IN);
            break;
        case ACTION_ZOOM_OUT:
            zoom->zoomOneStep(ZOOM_OUT);
            break;
        default:
            break;
    }
}

auto Control::getCurrentPageNo() -> size_t {
    if (this->win) {
        return this->win->getXournal()->getCurrentPage();
    }
    return 0;
}

auto Control::searchTextOnPage(string text, int p, int* occures, double* top) -> bool {
    return getWindow()->getXournal()->searchTextOnPage(std::move(text), p, occures, top);
}

auto Control::getCurrentPage() -> PageRef {
    this->doc->lock();
    PageRef p = this->doc->getPage(getCurrentPageNo());
    this->doc->unlock();

    return p;
}

void Control::fileOpened(fs::path const& path) { openFile(path); }

void Control::undoRedoChanged() {
    fireEnableAction(ACTION_UNDO, undoRedo->canUndo());
    fireEnableAction(ACTION_REDO, undoRedo->canRedo());

    win->setUndoDescription(undoRedo->undoDescription());
    win->setRedoDescription(undoRedo->redoDescription());

    updateWindowTitle();
}

void Control::undoRedoPageChanged(PageRef page) {
    if (std::find(begin(this->changedPages), end(this->changedPages), page) == end(this->changedPages)) {
        return;
    }

    this->changedPages.emplace_back(std::move(page));
}

void Control::selectTool(ToolType type) {
    toolHandler->selectTool(type);

    if (win) {
        (win->getXournal()->getViewFor(getCurrentPageNo()))->rerenderPage();
    }
}

void Control::selectDefaultTool() {
    ButtonConfig* cfg = settings->getDefaultButtonConfig();
    cfg->acceptActions(toolHandler);
}

void Control::toolChanged() {
    ToolType type = toolHandler->getToolType();

    // Convert enum values, enums has to be in the same order!
    auto at = static_cast<ActionType>(type - TOOL_PEN + ACTION_TOOL_PEN);

    fireActionSelected(GROUP_TOOL, at);

    fireEnableAction(ACTION_SELECT_COLOR, toolHandler->hasCapability(TOOL_CAP_COLOR));
    fireEnableAction(ACTION_SELECT_COLOR_CUSTOM, toolHandler->hasCapability(TOOL_CAP_COLOR));

    fireEnableAction(ACTION_RULER, toolHandler->hasCapability(TOOL_CAP_RULER));
    fireEnableAction(ACTION_TOOL_DRAW_RECT, toolHandler->hasCapability(TOOL_CAP_RECTANGLE));
    fireEnableAction(ACTION_TOOL_DRAW_CIRCLE, toolHandler->hasCapability(TOOL_CAP_CIRCLE));
    fireEnableAction(ACTION_TOOL_DRAW_ARROW, toolHandler->hasCapability(TOOL_CAP_ARROW));
    fireEnableAction(ACTION_TOOL_DRAW_COORDINATE_SYSTEM, toolHandler->hasCapability(TOOL_CAP_ARROW));
    fireEnableAction(ACTION_TOOL_DRAW_SPLINE, toolHandler->hasCapability(TOOL_CAP_SPLINE));
    fireEnableAction(ACTION_SHAPE_RECOGNIZER, toolHandler->hasCapability(TOOL_CAP_RECOGNIZER));

    bool enableSize = toolHandler->hasCapability(TOOL_CAP_SIZE);

    fireEnableAction(ACTION_SIZE_MEDIUM, enableSize);
    fireEnableAction(ACTION_SIZE_THICK, enableSize);
    fireEnableAction(ACTION_SIZE_FINE, enableSize);
    fireEnableAction(ACTION_SIZE_VERY_THICK, enableSize);
    fireEnableAction(ACTION_SIZE_VERY_FINE, enableSize);

    bool enableFill = toolHandler->hasCapability(TOOL_CAP_FILL);

    fireEnableAction(ACTION_TOOL_FILL, enableFill);


    if (enableSize) {
        toolSizeChanged();
    }

    // Update color
    if (toolHandler->hasCapability(TOOL_CAP_COLOR)) {
        toolColorChanged(false);
    }

    ActionType rulerAction = ACTION_NOT_SELECTED;
    if (toolHandler->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        rulerAction = ACTION_SHAPE_RECOGNIZER;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_LINE) {
        rulerAction = ACTION_RULER;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_RECTANGLE) {
        rulerAction = ACTION_TOOL_DRAW_RECT;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_CIRCLE) {
        rulerAction = ACTION_TOOL_DRAW_CIRCLE;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_ARROW) {
        rulerAction = ACTION_TOOL_DRAW_ARROW;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM) {
        rulerAction = ACTION_TOOL_DRAW_COORDINATE_SYSTEM;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE) {
        rulerAction = ACTION_TOOL_DRAW_SPLINE;
    }

    fireActionSelected(GROUP_RULER, rulerAction);

    getCursor()->updateCursor();

    if (type != TOOL_TEXT) {
        if (win) {
            win->getXournal()->endTextAllPages();
        }
    }
}

void Control::eraserSizeChanged() {
    switch (toolHandler->getEraserSize()) {
        case TOOL_SIZE_VERY_FINE:
            fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_VERY_FINE);
            break;
        case TOOL_SIZE_FINE:
            fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_FINE);
            break;
        case TOOL_SIZE_MEDIUM:
            fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_MEDIUM);
            break;
        case TOOL_SIZE_THICK:
            fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_THICK);
            break;
        case TOOL_SIZE_VERY_THICK:
            fireActionSelected(GROUP_ERASER_SIZE, ACTION_TOOL_ERASER_SIZE_VERY_THICK);
            break;
        default:
            break;
    }
}

void Control::penSizeChanged() {
    switch (toolHandler->getPenSize()) {
        case TOOL_SIZE_VERY_FINE:
            fireActionSelected(GROUP_PEN_SIZE, ACTION_TOOL_PEN_SIZE_VERY_FINE);
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
        default:
            break;
    }
}

void Control::hilighterSizeChanged() {
    switch (toolHandler->getHilighterSize()) {
        case TOOL_SIZE_VERY_FINE:
            fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_VERY_FINE);
            break;
        case TOOL_SIZE_FINE:
            fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_FINE);
            break;
        case TOOL_SIZE_MEDIUM:
            fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_MEDIUM);
            break;
        case TOOL_SIZE_THICK:
            fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_THICK);
            break;
        case TOOL_SIZE_VERY_THICK:
            fireActionSelected(GROUP_HILIGHTER_SIZE, ACTION_TOOL_HILIGHTER_SIZE_VERY_THICK);
            break;
        default:
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
            fireActionSelected(GROUP_SIZE, ACTION_SIZE_VERY_FINE);
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
            fireActionSelected(GROUP_SIZE, ACTION_SIZE_VERY_THICK);
            break;
    }

    getCursor()->updateCursor();
}

void Control::toolFillChanged() {
    fireActionSelected(GROUP_FILL, toolHandler->getFill() != -1 ? ACTION_TOOL_FILL : ACTION_NONE);
    fireActionSelected(GROUP_PEN_FILL, toolHandler->getPenFillEnabled() ? ACTION_TOOL_PEN_FILL : ACTION_NONE);
    fireActionSelected(GROUP_HILIGHTER_FILL,
                       toolHandler->getHilighterFillEnabled() ? ACTION_TOOL_HILIGHTER_FILL : ACTION_NONE);
}

void Control::toolLineStyleChanged() {
    const LineStyle& lineStyle = toolHandler->getTool(TOOL_PEN).getLineStyle();
    string style = StrokeStyle::formatStyle(lineStyle);

    if (style == "dash") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DASH);
    } else if (style == "dashdot") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DASH_DOT);
    } else if (style == "dot") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DOT);
    } else {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_PLAIN);
    }
}

/**
 * Select the color for the tool
 *
 * @param userSelection
 * 			true if the user selected the color
 * 			false if the color is selected by a tool change
 * 			and therefore should not be applied to a selection
 */
void Control::toolColorChanged(bool userSelection) {
    fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR);
    getCursor()->updateCursor();

    if (userSelection && this->win && toolHandler->getColor() != Color(-1)) {
        EditSelection* sel = this->win->getXournal()->getSelection();
        if (sel) {
            UndoAction* undo = sel->setColor(toolHandler->getColor());
            // move into selection
            undoRedo->addUndoAction(UndoActionPtr(undo));
        }

        TextEditor* edit = getTextEditor();


        if (this->toolHandler->getToolType() == TOOL_TEXT && edit != nullptr) {
            // Todo move into selection
            undoRedo->addUndoAction(UndoActionPtr(edit->setColor(toolHandler->getColor())));
        }
    }
}

void Control::setCustomColorSelected() { fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR_CUSTOM); }

void Control::showSettings() {
    // take note of some settings before to compare with after
    auto selectionColor = settings->getBorderColor();
    bool verticalSpace = settings->getAddVerticalSpace();
    int verticalSpaceAmount = settings->getAddVerticalSpaceAmount();
    bool horizontalSpace = settings->getAddHorizontalSpace();
    int horizontalSpaceAmount = settings->getAddHorizontalSpaceAmount();
    StylusCursorType stylusCursorType = settings->getStylusCursorType();
    bool highlightPosition = settings->isHighlightPosition();

    auto* dlg = new SettingsDialog(this->gladeSearchPath, settings, this);
    dlg->show(GTK_WINDOW(this->win->getWindow()));

    // note which settings have changed and act accordingly
    if (selectionColor != settings->getBorderColor()) {
        win->getXournal()->forceUpdatePagenumbers();
    }

    if (verticalSpace != settings->getAddVerticalSpace() || horizontalSpace != settings->getAddHorizontalSpace() ||
        verticalSpaceAmount != settings->getAddVerticalSpaceAmount() ||
        horizontalSpaceAmount != settings->getAddHorizontalSpaceAmount()) {
        int currentPage = getCurrentPageNo();
        win->getXournal()->layoutPages();
        scrollHandler->scrollToPage(currentPage);
    }

    if (stylusCursorType != settings->getStylusCursorType() || highlightPosition != settings->isHighlightPosition()) {
        getCursor()->updateCursor();
    }

    win->updateScrollbarSidebarPosition();

    enableAutosave(settings->isAutosaveEnabled());

    this->zoom->setZoomStep(settings->getZoomStep() / 100.0);
    this->zoom->setZoomStepScroll(settings->getZoomStepScroll() / 100.0);
    this->zoom->setZoom100Value(settings->getDisplayDpi() / Util::DPI_NORMALIZATION_FACTOR);

    getWindow()->getXournal()->getHandRecognition()->reload();

    TextView::setDpi(settings->getDisplayDpi());

    delete dlg;
}

auto Control::newFile(string pageTemplate, fs::path filepath) -> bool {
    if (!this->close(true)) {
        return false;
    }

    Document newDoc(this);

    this->doc->lock();
    *doc = newDoc;
    if (!filepath.empty()) {
        this->doc->setFilepath(std::move(filepath));
    }
    this->doc->unlock();

    addDefaultPage(std::move(pageTemplate));

    fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

    fileLoaded();

    return true;
}

/**
 * Check if this is an autosave file, return false in this case and display a user instruction
 */
auto Control::shouldFileOpen(fs::path const& filepath) const -> bool {
    auto basePath = Util::getConfigSubfolder("");
    auto isChild = Util::isChildOrEquivalent(filepath, basePath);
    if (isChild) {
        string msg = FS(_F("Do not open Autosave files. They may will be overwritten!\n"
                           "Copy the files to another folder.\n"
                           "Files from Folder {1} cannot be opened.") %
                        basePath.u8string());
        XojMsgBox::showErrorToUser(getGtkWindow(), msg);
    }
    return !isChild;
}

auto Control::openFile(fs::path filepath, int scrollToPage, bool forceOpen) -> bool {
    if (filepath.empty()) {
        bool attachPdf = false;
        XojOpenDlg dlg(getGtkWindow(), this->settings);
        filepath = dlg.showOpenDialog(false, attachPdf);
        g_message("%s", (_F("file: {1}") % filepath.string()).c_str());
    }

    if (filepath.empty() || (!forceOpen && !shouldFileOpen(filepath))) {
        return false;
    }

    if (!this->close(false)) {
        return false;
    }

    // Read template file
    if (filepath.extension() == ".xopt") {
        return loadXoptTemplate(filepath);
    }

    if (filepath.extension() == ".pdf") {
        return loadPdf(filepath, scrollToPage);
    }

    LoadHandler loadHandler;
    Document* loadedDocument = loadHandler.loadDocument(filepath);
    if ((loadedDocument != nullptr && loadHandler.isAttachedPdfMissing()) ||
        !loadHandler.getMissingPdfFilename().empty()) {
        // give the user a second chance to select a new PDF filepath, or to discard the PDF

        GtkWidget* dialog = gtk_message_dialog_new(
                getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, "%s",
                loadHandler.isAttachedPdfMissing() ? _("The attached background PDF could not be found.") :
                                                     _("The background PDF could not be found."));

        gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another PDF"), 1);
        gtk_dialog_add_button(GTK_DIALOG(dialog), _("Remove PDF Background"), 2);
        gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 3);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
        int res = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (res == 2)  // remove PDF background
        {
            loadHandler.removePdfBackground();
            loadedDocument = loadHandler.loadDocument(filepath);
        } else if (res == 1)  // select another PDF background
        {
            bool attachToDocument = false;
            XojOpenDlg dlg(getGtkWindow(), this->settings);
            auto pdfFilename = dlg.showOpenDialog(true, attachToDocument);
            if (!pdfFilename.empty()) {
                loadHandler.setPdfReplacement(pdfFilename, attachToDocument);
                loadedDocument = loadHandler.loadDocument(filepath);
            }
        }
    }

    if (!loadedDocument) {
        string msg = FS(_F("Error opening file \"{1}\"") % filepath.string()) + "\n" + loadHandler.getLastError();
        XojMsgBox::showErrorToUser(getGtkWindow(), msg);

        fileLoaded(scrollToPage);
        return false;
    } else if (loadHandler.getFileVersion() > FILE_FORMAT_VERSION) {
        GtkWidget* dialog = gtk_message_dialog_new(
                getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s",
                _("The file being loaded has a file format version newer than the one currently supported by this "
                  "version of Xournal++, so it may not load properly. Open anyways?"));
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (response != GTK_RESPONSE_YES) {
            loadedDocument->clearDocument();
            return false;
        }
    }

    this->closeDocument();

    this->doc->lock();
    this->doc->clearDocument();
    *this->doc = *loadedDocument;
    this->doc->unlock();

    // Set folder as last save path, so the next save will be at the current document location
    // This is important because of the new .xopp format, where Xournal .xoj handled as import,
    // not as file to load
    settings->setLastSavePath(filepath.parent_path());


    fileLoaded(scrollToPage);
    return true;
}

auto Control::loadPdf(const fs::path& filepath, int scrollToPage) -> bool {
    LoadHandler loadHandler;

    if (settings->isAutloadPdfXoj()) {
        fs::path f = filepath;
        Util::clearExtensions(f);
        f += ".xopp";
        Document* tmp = loadHandler.loadDocument(f);

        if (tmp == nullptr) {
            f = filepath;
            Util::clearExtensions(f);
            f += ".xoj";
            tmp = loadHandler.loadDocument(f);
        }

        if (tmp) {
            this->doc->lock();
            this->doc->clearDocument();
            *this->doc = *tmp;
            this->doc->unlock();

            fileLoaded(scrollToPage);
            return true;
        }
    }

    bool an = annotatePdf(filepath, false, false);
    fileLoaded(scrollToPage);
    return an;
}

auto Control::loadXoptTemplate(fs::path const& filepath) -> bool {
    auto contents = Util::readString(filepath);
    if (!contents.has_value()) {
        return false;
    }

    newFile(*contents);
    return true;
}

void Control::fileLoaded(int scrollToPage) {
    this->doc->lock();
    auto filepath = this->doc->getEvMetadataFilename();
    this->doc->unlock();

    if (!filepath.empty()) {
        MetadataEntry md = MetadataManager::getForFile(filepath);
        if (!md.valid) {
            md.zoom = -1;
            md.page = 0;
        }

        if (scrollToPage >= 0) {
            md.page = scrollToPage;
        }

        loadMetadata(md);
        RecentManager::addRecentFileFilename(filepath);
    } else {
        zoom->updateZoomFitValue();
        zoom->setZoomFitMode(true);
    }

    updateWindowTitle();
    win->getXournal()->forceUpdatePagenumbers();
    getCursor()->updateCursor();
    updateDeletePageButton();
}

class MetadataCallbackData {
public:
    Control* ctrl{};
    MetadataEntry md;
};

/**
 * Load the data after processing the document...
 */
auto Control::loadMetadataCallback(MetadataCallbackData* data) -> bool {
    if (!data->md.valid) {
        delete data;
        return false;
    }
    ZoomControl* zoom = data->ctrl->zoom;
    if (zoom->isZoomPresentationMode()) {
        data->ctrl->setViewPresentationMode(true);
    } else if (zoom->isZoomFitMode()) {
        zoom->updateZoomFitValue();
        zoom->setZoomFitMode(true);
    } else {
        zoom->setZoomFitMode(false);
        zoom->setZoom(data->md.zoom * zoom->getZoom100Value());
    }
    data->ctrl->scrollHandler->scrollToPage(data->md.page);

    delete data;

    // Do not call again!
    return false;
}

void Control::loadMetadata(MetadataEntry md) {
    auto* data = new MetadataCallbackData();
    data->md = std::move(md);
    data->ctrl = this;

    g_idle_add(reinterpret_cast<GSourceFunc>(loadMetadataCallback), data);
}

auto Control::annotatePdf(fs::path filepath, bool /*attachPdf*/, bool attachToDocument) -> bool {
    if (!this->close(false)) {
        return false;
    }

    if (filepath.empty()) {
        XojOpenDlg dlg(getGtkWindow(), this->settings);
        filepath = dlg.showOpenDialog(true, attachToDocument);
        if (filepath.empty()) {
            return false;
        }
    }

    this->closeDocument();

    getCursor()->setCursorBusy(true);

    this->doc->setFilepath("");
    bool res = this->doc->readPdf(filepath, true, attachToDocument);

    if (res) {
        RecentManager::addRecentFileFilename(filepath.c_str());

        this->doc->lock();
        auto filepath = this->doc->getEvMetadataFilename();
        this->doc->unlock();
        MetadataEntry md = MetadataManager::getForFile(filepath);
        loadMetadata(md);
    } else {
        this->doc->lock();
        string errMsg = doc->getLastErrorMsg();
        this->doc->unlock();

        string msg = FS(_F("Error annotate PDF file \"{1}\"\n{2}") % filepath.string() % errMsg);
        XojMsgBox::showErrorToUser(getGtkWindow(), msg);
    }
    getCursor()->setCursorBusy(false);

    fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);

    getCursor()->updateCursor();

    return true;
}

void Control::print() {
    PrintHandler print;
    this->doc->lock();
    print.print(this->doc, getCurrentPageNo());
    this->doc->unlock();
}

void Control::block(const string& name) {
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

    gtk_label_set_text(this->lbState, name.c_str());
    gtk_widget_show(this->statusbar);

    this->maxState = 100;
    this->isBlocking = true;
}

void Control::unblock() {
    if (!this->isBlocking) {
        return;
    }

    this->win->setControlTmpDisabled(false);
    getCursor()->setCursorBusy(false);
    disableSidebarTmp(false);

    gtk_widget_hide(this->statusbar);

    this->isBlocking = false;
}

void Control::setMaximumState(int max) { this->maxState = max; }

void Control::setCurrentState(int state) {
    Util::execInUiThread([=]() { gtk_progress_bar_set_fraction(this->pgState, gdouble(state) / this->maxState); });
}

auto Control::save(bool synchron) -> bool {
    // clear selection before saving
    clearSelectionEndText();

    this->doc->lock();
    fs::path filepath = this->doc->getFilepath();
    this->doc->unlock();

    if (filepath.empty()) {
        if (!showSaveDialog()) {
            return false;
        }
    }

    auto* job = new SaveJob(this);
    bool result = true;
    if (synchron) {
        result = job->save();
        unblock();
        this->resetSavedStatus();
    } else {
        this->scheduler->addJob(job, JOB_PRIORITY_URGENT);
    }
    job->unref();

    return result;
}

auto Control::showSaveDialog() -> bool {
    GtkWidget* dialog =
            gtk_file_chooser_dialog_new(_("Save File"), getGtkWindow(), GTK_FILE_CHOOSER_ACTION_SAVE, _("_Cancel"),
                                        GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_OK, nullptr);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ files"));
    gtk_file_filter_add_pattern(filterXoj, "*.xopp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

    this->doc->lock();
    auto suggested_folder = this->doc->createSaveFolder(this->settings->getLastSavePath());
    auto suggested_name = this->doc->createSaveFilename(Document::XOPP, this->settings->getDefaultSaveName());
    this->doc->unlock();

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), suggested_folder.string().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), suggested_name.string().c_str());
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), this->settings->getLastOpenPath().string().c_str(),
                                         nullptr);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), false);  // handled below

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));

    while (true) {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return false;
        }

        auto fileTmp = Util::fromGtkFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        Util::clearExtensions(fileTmp);
        fileTmp += ".xopp";
        // Since we add the extension after the OK button, we have to check manually on existing files
        if (askToReplace(fileTmp)) {
            break;
        }
    }

    auto filename = Util::fromGtkFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    settings->setLastSavePath(filename.parent_path());
    gtk_widget_destroy(dialog);

    this->doc->lock();

    this->doc->setFilepath(filename);
    this->doc->unlock();

    return true;
}

void Control::updateWindowTitle() {
    string title{};

    this->doc->lock();
    if (doc->getFilepath().empty()) {
        if (doc->getPdfFilepath().empty()) {
            title = _("Unsaved Document");
        } else {
            if (undoRedo->isChanged()) {
                title += "*";
            }
            title += doc->getPdfFilepath().filename().string();
        }
    } else {
        if (undoRedo->isChanged()) {
            title += "*";
        }

        title += doc->getFilepath().filename().string();
    }
    this->doc->unlock();

    title += " - Xournal++";

    gtk_window_set_title(getGtkWindow(), title.c_str());
}

void Control::exportAsPdf() {
    this->clearSelectionEndText();
    exportBase(new PdfExportJob(this));
}

void Control::exportAs() {
    this->clearSelectionEndText();
    exportBase(new CustomExportJob(this));
}

void Control::exportBase(BaseExportJob* job) {
    if (job->showFilechooser()) {
        this->scheduler->addJob(job, JOB_PRIORITY_NONE);
    } else {
        // The job blocked, so we have to unblock, because the job unblocks only after run
        unblock();
    }
    job->unref();
}

auto Control::saveAs() -> bool {
    if (!showSaveDialog()) {
        return false;
    }
    this->doc->lock();
    auto filepath = doc->getFilepath();
    this->doc->unlock();

    if (filepath.empty()) {
        return false;
    }

    // no lock needed, this is an uncritically operation
    this->doc->setCreateBackupOnSave(false);
    return save();
}

void Control::resetSavedStatus() {
    this->doc->lock();
    auto filepath = this->doc->getFilepath();
    this->doc->unlock();

    this->undoRedo->documentSaved();
    RecentManager::addRecentFileFilename(filepath);
    this->updateWindowTitle();
}

void Control::quit(bool allowCancel) {
    if (!this->close(false, allowCancel)) {
        if (!allowCancel) {
            // Cancel is not allowed, and the user close or did not save
            // This is probably called from macOS, where the Application
            // now will be killed - therefore do an emergency save.
            emergencySave();
        }

        return;
    }

    this->closeDocument();

    this->scheduler->lock();

    audioController->stopRecording();
    settings->save();

    this->scheduler->removeAllJobs();
    this->scheduler->unlock();
    gtk_main_quit();
}

auto Control::close(const bool allowDestroy, const bool allowCancel) -> bool {
    clearSelectionEndText();
    metadata->documentChanged();

    bool discard = false;
    const bool fileRemoved = !doc->getFilepath().empty() && !fs::exists(this->doc->getFilepath());
    if (undoRedo->isChanged()) {
        const auto message = fileRemoved ? _("Document file was removed.") : _("This document is not saved yet.");
        const auto saveLabel = fileRemoved ? _("Save As...") : _("Save");
        GtkWidget* dialog = gtk_message_dialog_new(getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
                                                   GTK_BUTTONS_NONE, "%s", message);

        gtk_dialog_add_button(GTK_DIALOG(dialog), saveLabel, GTK_RESPONSE_ACCEPT);
        gtk_dialog_add_button(GTK_DIALOG(dialog), _("Discard"), GTK_RESPONSE_REJECT);

        if (allowCancel) {
            gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), GTK_RESPONSE_CANCEL);
        }

        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
        const auto dialogResponse = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        switch (dialogResponse) {
            case GTK_RESPONSE_ACCEPT:
                if (fileRemoved) {
                    return this->saveAs();
                } else {
                    return this->save(true);
                }
                break;
            case GTK_RESPONSE_REJECT:
                discard = true;
                break;
            default:
                return false;
                break;
        }
    }

    if (allowDestroy && discard) {
        this->closeDocument();
    }
    return true;
}

auto Control::closeAndDestroy(bool allowCancel) -> bool {
    // We don't want to "double close", so disallow it first.
    auto retval = this->close(false, allowCancel);
    this->closeDocument();
    return retval;
}

void Control::closeDocument() {
    this->undoRedo->clearContents();

    this->doc->lock();
    this->doc->clearDocument(true);
    this->doc->unlock();

    this->undoRedoChanged();
}

void Control::applyPreferredLanguage() {
#ifdef _WIN32
    _putenv_s("LANGUAGE", this->settings->getPreferredLocale().c_str());
#else
    setenv("LANGUAGE", this->settings->getPreferredLocale().c_str(), 1);
#endif
}

auto Control::askToReplace(fs::path const& filepath) const -> bool {
    if (fs::exists(filepath)) {
        string msg = FS(FORMAT_STR("The file {1} already exists! Do you want to replace it?") %
                        filepath.filename().string());
        int res = XojMsgBox::replaceFileQuestion(getGtkWindow(), msg);
        return res == GTK_RESPONSE_OK;
    }
    return true;
}

void Control::resetShapeRecognizer() {
    if (this->win) {
        this->win->getXournal()->resetShapeRecognizer();
    }
}

void Control::showAbout() {
    AboutDialog dlg(this->gladeSearchPath);
    dlg.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::clipboardCutCopyEnabled(bool enabled) {
    fireEnableAction(ACTION_CUT, enabled);
    fireEnableAction(ACTION_COPY, enabled);
}

void Control::clipboardPasteEnabled(bool enabled) { fireEnableAction(ACTION_PASTE, enabled); }

void Control::clipboardPasteText(string text) {
    Text* t = new Text();
    t->setText(text);
    t->setFont(settings->getFont());
    t->setColor(toolHandler->getColor());

    clipboardPaste(t);
}

void Control::clipboardPasteImage(GdkPixbuf* img) {
    auto image = new Image();
    image->setImage(img);

    auto width =
            static_cast<double>(gdk_pixbuf_get_width(img)) / settings->getDisplayDpi() * Util::DPI_NORMALIZATION_FACTOR;
    auto height = static_cast<double>(gdk_pixbuf_get_height(img)) / settings->getDisplayDpi() *
                  Util::DPI_NORMALIZATION_FACTOR;

    int pageNr = getCurrentPageNo();
    if (pageNr == -1) {
        return;
    }

    this->doc->lock();
    PageRef page = this->doc->getPage(pageNr);
    auto pageWidth = page->getWidth();
    auto pageHeight = page->getHeight();
    this->doc->unlock();

    // Size: 3/4 of the page size
    pageWidth = pageWidth * 3.0 / 4.0;
    pageHeight = pageHeight * 3.0 / 4.0;

    auto scaledWidth = width;
    auto scaledHeight = height;

    if (width > pageWidth) {
        scaledWidth = pageWidth;
        scaledHeight = (scaledWidth * height) / width;
    }

    if (scaledHeight > pageHeight) {
        scaledHeight = pageHeight;
        scaledWidth = (scaledHeight * width) / height;
    }

    image->setWidth(scaledWidth);
    image->setHeight(scaledHeight);

    clipboardPaste(image);
}

void Control::clipboardPaste(Element* e) {
    double x = 0;
    double y = 0;
    int pageNr = getCurrentPageNo();
    if (pageNr == -1) {
        return;
    }

    XojPageView* view = win->getXournal()->getViewFor(pageNr);
    if (view == nullptr) {
        return;
    }

    this->doc->lock();
    PageRef page = this->doc->getPage(pageNr);
    Layer* layer = page->getSelectedLayer();
    win->getXournal()->getPasteTarget(x, y);

    double width = e->getElementWidth();
    double height = e->getElementHeight();

    x = std::max(0.0, x - width / 2);
    y = std::max(0.0, y - height / 2);

    e->setX(x);
    e->setY(y);
    layer->addElement(e);

    this->doc->unlock();

    undoRedo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, e));
    auto* selection = new EditSelection(this->undoRedo, e, view, page);

    win->getXournal()->setSelection(selection);
}

void Control::clipboardPasteXournal(ObjectInputStream& in) {
    int pNr = getCurrentPageNo();
    if (pNr == -1 && win != nullptr) {
        return;
    }

    this->doc->lock();
    PageRef page = this->doc->getPage(pNr);
    Layer* layer = page->getSelectedLayer();

    XojPageView* view = win->getXournal()->getViewFor(pNr);

    if (!view || !page) {
        this->doc->unlock();
        return;
    }

    EditSelection* selection = nullptr;
    try {
        std::unique_ptr<Element> element;
        string version = in.readString();
        if (version != PROJECT_STRING) {
            g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PROJECT_STRING);
        }

        selection = new EditSelection(this->undoRedo, page, view);
        selection->readSerialized(in);

        // document lock not needed anymore, because we don't change the document, we only change the selection
        this->doc->unlock();

        int count = in.readInt();
        auto pasteAddUndoAction = std::make_unique<AddUndoAction>(page, false);
        // this will undo a group of elements that are inserted

        for (int i = 0; i < count; i++) {
            string name = in.getNextObjectName();
            element.reset();

            if (name == "Stroke") {
                element = std::make_unique<Stroke>();
            } else if (name == "Image") {
                element = std::make_unique<Image>();
            } else if (name == "TexImage") {
                element = std::make_unique<TexImage>();
            } else if (name == "Text") {
                element = std::make_unique<Text>();
            } else {
                throw InputStreamException(FS(FORMAT_STR("Get unknown object {1}") % name), __FILE__, __LINE__);
            }

            element->readSerialized(in);

            pasteAddUndoAction->addElement(layer, element.get(), layer->indexOf(element.get()));
            // Todo: unique_ptr
            selection->addElement(element.release(), Layer::InvalidElementIndex);
        }
        undoRedo->addUndoAction(std::move(pasteAddUndoAction));

        double x = 0;
        double y = 0;
        // calculate x/y of paste target, see clipboardPaste(Element* e)
        win->getXournal()->getPasteTarget(x, y);

        x = std::max(0.0, x - selection->getWidth() / 2);
        y = std::max(0.0, y - selection->getHeight() / 2);

        // calculate difference between current selection position and destination
        auto dx = x - selection->getXOnView();
        auto dy = y - selection->getYOnView();

        selection->moveSelection(dx, dy);
        // update all Elements (same procedure as moving a element selection by hand and releasing the mouse button)
        selection->mouseUp();

        win->getXournal()->setSelection(selection);
    } catch (std::exception& e) {
        g_warning("could not paste, Exception occurred: %s", e.what());
        Stacktrace::printStracktrace();
        if (selection) {
            for (Element* e: *selection->getElements()) {
                delete e;
            }
            delete selection;
        }
    }
}

void Control::deleteSelection() {
    if (win) {
        win->getXournal()->deleteSelection();
    }
}

void Control::clearSelection() {
    if (this->win) {
        this->win->getXournal()->clearSelection();
    }
}

void Control::setClipboardHandlerSelection(EditSelection* selection) {
    if (this->clipboardHandler) {
        this->clipboardHandler->setSelection(selection);
    }
}

void Control::setCopyPasteEnabled(bool enabled) { this->clipboardHandler->setCopyPasteEnabled(enabled); }

void Control::setFill(bool fill) {
    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }

    if (sel) {
        undoRedo->addUndoAction(UndoActionPtr(
                sel->setFill(fill ? toolHandler->getPenFill() : -1, fill ? toolHandler->getHilighterFill() : -1)));
    }

    if (toolHandler->getToolType() == TOOL_PEN) {
        fireActionSelected(GROUP_PEN_FILL, fill ? ACTION_TOOL_PEN_FILL : ACTION_NONE);
        this->toolHandler->setPenFillEnabled(fill, false);
    } else if (toolHandler->getToolType() == TOOL_HILIGHTER) {
        fireActionSelected(GROUP_HILIGHTER_FILL, fill ? ACTION_TOOL_HILIGHTER_FILL : ACTION_NONE);
        this->toolHandler->setHilighterFillEnabled(fill, false);
    }
}

void Control::setLineStyle(const string& style) {
    LineStyle stl = StrokeStyle::parseStyle(style.c_str());

    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }

    // TODO(fabian): allow to change selection
    if (sel) {
        //		UndoAction* undo = sel->setSize(size, toolHandler->getToolThickness(TOOL_PEN),
        //										toolHandler->getToolThickness(TOOL_HILIGHTER),
        //										toolHandler->getToolThickness(TOOL_ERASER));
        //		undoRedo->addUndoAction(undo);
    }

    this->toolHandler->setLineStyle(stl);
}

void Control::setToolSize(ToolSize size) {
    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }

    if (sel) {
        undoRedo->addUndoAction(UndoActionPtr(sel->setSize(size, toolHandler->getToolThickness(TOOL_PEN),
                                                           toolHandler->getToolThickness(TOOL_HILIGHTER),
                                                           toolHandler->getToolThickness(TOOL_ERASER))));
    }
    this->toolHandler->setSize(size);
}

void Control::fontChanged() {
    XojFont font = win->getFontButtonFont();
    settings->setFont(font);

    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }
    if (sel) {
        undoRedo->addUndoAction(UndoActionPtr(sel->setFont(font)));
    }

    TextEditor* editor = getTextEditor();
    if (editor) {
        editor->setFont(font);
    }
}

/**
 * The core handler for inserting latex
 */
void Control::runLatex() {
    LatexController latex(this);
    latex.run();
}

/**
 * GETTER / SETTER
 */

auto Control::getUndoRedoHandler() -> UndoRedoHandler* { return this->undoRedo; }

auto Control::getZoomControl() -> ZoomControl* { return this->zoom; }

auto Control::getCursor() -> XournalppCursor* { return this->cursor; }

auto Control::getRecentManager() -> RecentManager* { return this->recent; }

auto Control::getDocument() -> Document* { return this->doc; }

auto Control::getToolHandler() -> ToolHandler* { return this->toolHandler; }

auto Control::getScheduler() -> XournalScheduler* { return this->scheduler; }

auto Control::getWindow() -> MainWindow* { return this->win; }

auto Control::getGtkWindow() const -> GtkWindow* { return GTK_WINDOW(this->win->getWindow()); }

auto Control::isFullscreen() -> bool { return this->fullscreenHandler->isFullscreen(); }

void Control::rotationSnappingToggle() {
    settings->setSnapRotation(!settings->isSnapRotation());
    fireActionSelected(GROUP_SNAPPING, settings->isSnapRotation() ? ACTION_ROTATION_SNAPPING : ACTION_NONE);
}

void Control::gridSnappingToggle() {
    settings->setSnapGrid(!settings->isSnapGrid());
    fireActionSelected(GROUP_GRID_SNAPPING, settings->isSnapGrid() ? ACTION_GRID_SNAPPING : ACTION_NONE);
}

auto Control::getTextEditor() -> TextEditor* {
    if (this->win) {
        return this->win->getXournal()->getTextEditor();
    }
    return nullptr;
}

auto Control::getGladeSearchPath() -> GladeSearchpath* { return this->gladeSearchPath; }

auto Control::getSettings() -> Settings* { return settings; }

auto Control::getScrollHandler() -> ScrollHandler* { return this->scrollHandler; }

auto Control::getMetadataManager() -> MetadataManager* { return this->metadata; }

auto Control::getSidebar() -> Sidebar* { return this->sidebar; }

auto Control::getSearchBar() -> SearchBar* { return this->searchBar; }

auto Control::getAudioController() -> AudioController* { return this->audioController; }

auto Control::getPageTypes() -> PageTypeHandler* { return this->pageTypes; }

auto Control::getNewPageType() -> PageTypeMenu* { return this->newPageType; }

auto Control::getPageBackgroundChangeController() -> PageBackgroundChangeController* {
    return this->pageBackgroundChangeController;
}

auto Control::getLayerController() -> LayerController* { return this->layerController; }
