#include "Control.h"

#include <algorithm>  // for max
#include <cstdlib>    // for size_t
#include <exception>  // for exce...
#include <iterator>   // for end
#include <locale>
#include <memory>     // for make...
#include <numeric>    // for accu...
#include <optional>   // for opti...
#include <regex>      // for regex
#include <utility>    // for move

#include "control/AudioController.h"                             // for Audi...
#include "control/ClipboardHandler.h"                            // for Clip...
#include "control/CompassController.h"                           // for Comp...
#include "control/RecentManager.h"                               // for Rece...
#include "control/ScrollHandler.h"                               // for Scro...
#include "control/SetsquareController.h"                         // for Sets...
#include "control/Tool.h"                                        // for Tool
#include "control/ToolHandler.h"                                 // for Tool...
#include "control/jobs/AutosaveJob.h"                            // for Auto...
#include "control/jobs/BaseExportJob.h"                          // for Base...
#include "control/jobs/CustomExportJob.h"                        // for Cust...
#include "control/jobs/PdfExportJob.h"                           // for PdfE...
#include "control/jobs/SaveJob.h"                                // for SaveJob
#include "control/jobs/Scheduler.h"                              // for JOB_...
#include "control/jobs/XournalScheduler.h"                       // for Xour...
#include "control/layer/LayerController.h"                       // for Laye...
#include "control/pagetype/PageTypeHandler.h"                    // for Page...
#include "control/pagetype/PageTypeMenu.h"                       // for Page...
#include "control/settings/ButtonConfig.h"                       // for Butt...
#include "control/settings/MetadataManager.h"                    // for Meta...
#include "control/settings/PageTemplateSettings.h"               // for Page...
#include "control/settings/Settings.h"                           // for Sett...
#include "control/settings/SettingsEnums.h"                      // for Button
#include "control/settings/ViewModes.h"                          // for ViewM..
#include "control/tools/EditSelection.h"                         // for Edit...
#include "control/tools/TextEditor.h"                            // for Text...
#include "control/xojfile/LoadHandler.h"                         // for Load...
#include "control/zoom/ZoomControl.h"                            // for Zoom...
#include "gui/MainWindow.h"                                      // for Main...
#include "gui/PageView.h"                                        // for XojP...
#include "gui/PdfFloatingToolbox.h"                              // for PdfF...
#include "gui/SearchBar.h"                                       // for Sear...
#include "gui/XournalView.h"                                     // for Xour...
#include "gui/XournalppCursor.h"                                 // for Xour...
#include "gui/dialog/AboutDialog.h"                              // for Abou...
#include "gui/dialog/FillOpacityDialog.h"                        // for Fill...
#include "gui/dialog/FormatDialog.h"                             // for Form...
#include "gui/dialog/GotoDialog.h"                               // for Goto...
#include "gui/dialog/PageTemplateDialog.h"                       // for Page...
#include "gui/dialog/SelectBackgroundColorDialog.h"              // for Sele...
#include "gui/dialog/SettingsDialog.h"                           // for Sett...
#include "gui/dialog/ToolbarManageDialog.h"                      // for Tool...
#include "gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"  // for Tool...
#include "gui/inputdevices/CompassInputHandler.h"                // for Comp...
#include "gui/inputdevices/GeometryToolInputHandler.h"           // for Geom...
#include "gui/inputdevices/HandRecognition.h"                    // for Hand...
#include "gui/inputdevices/SetsquareInputHandler.h"              // for Sets...
#include "gui/sidebar/Sidebar.h"                                 // for Sidebar
#include "gui/toolbarMenubar/ToolMenuHandler.h"                  // for Tool...
#include "gui/toolbarMenubar/model/ToolbarData.h"                // for Tool...
#include "gui/toolbarMenubar/model/ToolbarModel.h"               // for Tool...
#include "model/Compass.h"                                       // for Comp...
#include "model/Document.h"                                      // for Docu...
#include "model/DocumentChangeType.h"                            // for DOCU...
#include "model/DocumentListener.h"                              // for Docu...
#include "model/Element.h"                                       // for Element
#include "model/Font.h"                                          // for XojFont
#include "model/Image.h"                                         // for Image
#include "model/Layer.h"                                         // for Layer
#include "model/LineStyle.h"                                     // for Line...
#include "model/PageType.h"                                      // for Page...
#include "model/Setsquare.h"                                     // for Sets...
#include "model/Stroke.h"                                        // for Stroke
#include "model/StrokeStyle.h"                                   // for Stro...
#include "model/TexImage.h"                                      // for TexI...
#include "model/Text.h"                                          // for Text
#include "model/XojPage.h"                                       // for XojPage
#include "pdf/base/XojPdfPage.h"                                 // for XojP...
#include "plugin/PluginController.h"                             // for Plug...
#include "stockdlg/XojOpenDlg.h"                                 // for XojO...
#include "undo/AddUndoAction.h"                                  // for AddU...
#include "undo/InsertDeletePageUndoAction.h"                     // for Inse...
#include "undo/InsertUndoAction.h"                               // for Inse...
#include "undo/MoveSelectionToLayerUndoAction.h"                 // for Move...
#include "undo/UndoAction.h"                                     // for Undo...
#include "util/Color.h"                                          // for oper...
#include "util/PathUtil.h"                                       // for clea...
#include "util/PlaceholderString.h"                              // for Plac...
#include "util/Stacktrace.h"                                     // for Stac...
#include "util/Util.h"                                           // for exec...
#include "util/XojMsgBox.h"                                      // for XojM...
#include "util/glib_casts.h"                                     // for wrap_v
#include "util/i18n.h"                                           // for _, FS
#include "util/serializing/InputStreamException.h"               // for Inpu...
#include "util/serializing/ObjectInputStream.h"                  // for Obje...
#include "view/CompassView.h"                                    // for Comp...
#include "view/SetsquareView.h"                                  // for Sets...
#include "view/overlays/OverlayView.h"                           // for Over...

#include "CrashHandler.h"                    // for emer...
#include "FullscreenHandler.h"               // for Full...
#include "LatexController.h"                 // for Late...
#include "PageBackgroundChangeController.h"  // for Page...
#include "PrintHandler.h"                    // for print
#include "UndoRedoController.h"              // for Undo...
#include "config-dev.h"                      // for SETT...
#include "config.h"                          // for PROJ...

using std::string;

Control::Control(GApplication* gtkApp, GladeSearchpath* gladeSearchPath, bool disableAudio): gtkApp(gtkApp) {
    this->undoRedo = new UndoRedoHandler(this);
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

    this->pageTypes = new PageTypeHandler(gladeSearchPath);
    this->newPageType = std::make_unique<PageTypeMenu>(this->pageTypes, settings, true, true);

    this->audioController =
            (disableAudio || this->settings->isAudioDisabled()) ? nullptr : new AudioController(this->settings, this);

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
    this->initButtonTool();

    /**
     * This is needed to update the previews
     */
    this->changeTimout = g_timeout_add_seconds(5, xoj::util::wrap_v<checkChangedDocument>, this);

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

    deleteLastAutosaveFile();
    this->scheduler->stop();
    this->changedPages.clear();  // can be removed, will be done by implicit destructor

    delete this->pluginController;
    this->pluginController = nullptr;
    delete this->clipboardHandler;
    this->clipboardHandler = nullptr;
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

void Control::setLastAutosaveFile(fs::path newAutosaveFile) {
    try {
        if (!this->lastAutosaveFilename.empty() && !fs::equivalent(newAutosaveFile, this->lastAutosaveFilename) &&
            fs::exists(newAutosaveFile)) {
            deleteLastAutosaveFile();
        }
    } catch (const fs::filesystem_error& e) {
        auto fmtstr = FS(_F("Filesystem error: {1}") % e.what());
        Util::execInUiThread([fmtstr, win = getGtkWindow()]() { XojMsgBox::showErrorToUser(win, fmtstr); });
    }
    this->lastAutosaveFilename = std::move(newAutosaveFile);
}

void Control::deleteLastAutosaveFile() {
    try {
        if (fs::exists(this->lastAutosaveFilename)) {
            fs::remove(this->lastAutosaveFilename);
        }
    } catch (const fs::filesystem_error& e) {
        auto fmtstr = FS(_F("Could not remove old autosave file \"{1}\": {2}") % this->lastAutosaveFilename.string() %
                         e.what());
        Util::execInUiThread([fmtstr, win = getGtkWindow()]() { XojMsgBox::showErrorToUser(win, fmtstr); });
    }
    this->lastAutosaveFilename.clear();
}

auto Control::checkChangedDocument(Control* control) -> bool {
    if (!control->doc->tryLock()) {
        // call again later
        return true;
    }
    for (auto const& page: control->changedPages) {
        auto p = control->doc->indexOf(page);
        if (p != npos) {
            for (DocumentListener* dl: control->changedDocumentListeners) {
                dl->pageChanged(p);
            }
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
    highlighterSizeChanged();
    updateDeletePageButton();
    toolFillChanged();
    toolLineStyleChanged();

    this->clipboardHandler = new ClipboardHandler(this, win->getXournal()->getWidget());

    this->enableAutosave(settings->isAutosaveEnabled());

    win->setFontButtonFont(settings->getFont());

    win->rebindMenubarAccelerators();

    fireActionSelected(GROUP_SNAPPING, settings->isSnapRotation() ? ACTION_ROTATION_SNAPPING : ACTION_NONE);
    fireActionSelected(GROUP_GRID_SNAPPING, settings->isSnapGrid() ? ACTION_GRID_SNAPPING : ACTION_NONE);
    fireActionSelected(GROUP_GEOMETRY_TOOL, ACTION_NONE);
}

auto Control::autosaveCallback(Control* control) -> bool {
    if (!control->undoRedo->isChangedAutosave()) {
        // do nothing, nothing changed
        return true;
    }

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
        auto timeout = guint(settings->getAutosaveTimeout()) * 60U;
        this->autosaveTimeout = g_timeout_add_seconds(timeout, xoj::util::wrap_v<autosaveCallback>, this);
    }
}

void Control::updatePageNumbers(size_t page, size_t pdfPage) {
    if (this->win == nullptr) {
        return;
    }

    this->win->updatePageNumbers(page, this->doc->getPageCount(), pdfPage);
    this->sidebar->selectPageNr(page, pdfPage);

    this->metadata->storeMetadata(this->doc->getEvMetadataFilename(), int(page), getZoomControl()->getZoomReal());

    auto current = getCurrentPageNo();
    auto count = this->doc->getPageCount();

    fireEnableAction(ACTION_GOTO_FIRST, current != 0);
    fireEnableAction(ACTION_GOTO_BACK, current != 0);
    fireEnableAction(ACTION_GOTO_PREVIOUS_ANNOTATED_PAGE, current != 0);

    fireEnableAction(ACTION_GOTO_PAGE, count > 1);

    fireEnableAction(ACTION_GOTO_NEXT, current < count - 1);
    fireEnableAction(ACTION_GOTO_LAST, current < count - 1);
    fireEnableAction(ACTION_GOTO_NEXT_ANNOTATED_PAGE, current < count - 1);
}

void Control::actionPerformed(ActionType type, ActionGroup group, GtkToolButton* toolbutton, bool enabled) {
    if (layerController->actionPerformed(type)) {
        return;
    }

    // If PDF toolbox is currently shown, end the PDF selection unless:
    //   1) Hand tool action
    //   2) zoom action (toolbox will be hidden in ZoomControl::zoomChanged)
    if (getWindow() && getWindow()->getPdfToolbox()->hasSelection()) {
        bool keepPdfToolbox = type == ACTION_TOOL_HAND || type == ACTION_ZOOM_100 || type == ACTION_ZOOM_FIT ||
                              type == ACTION_ZOOM_IN || type == ACTION_ZOOM_OUT;
        if (!keepPdfToolbox) {
            getWindow()->getPdfToolbox()->userCancelSelection();
        }
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
            clearSelectionEndText();
            UndoRedoController::undo(this);
            break;
        case ACTION_REDO:
            clearSelectionEndText();
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
        case ACTION_SELECT_ALL:
            selectAllOnPage();
            break;
        case ACTION_DELETE:
            if (!win->getXournal()->actionDelete()) {
                deleteSelection();
            }
            break;
        case ACTION_SETTINGS:
            showSettings();
            break;
        case ACTION_HIGHLIGHT_POSITION:
            highlightPositionToggle();
            break;

        case ACTION_ARRANGE_BRING_TO_FRONT:
        case ACTION_ARRANGE_BRING_FORWARD:
        case ACTION_ARRANGE_SEND_BACKWARD:
        case ACTION_ARRANGE_SEND_TO_BACK:
            this->reorderSelection(type);
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
        case ACTION_DUPLICATE_PAGE:
            duplicatePage();
            break;
        case ACTION_NEW_PAGE_AFTER:
            insertNewPage(getCurrentPageNo() + 1);
            break;
        case ACTION_APPEND_NEW_PDF_PAGES:
            appendNewPdfPages();
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
        case ACTION_MOVE_SELECTION_LAYER_UP:
            // moveSelectionToLayer takes layer number (layerid - 1) not id
            // therefor the new layer is "layerid - 1 + 1"
            moveSelectionToLayer(getCurrentPage()->getSelectedLayerId());
            break;
        case ACTION_MOVE_SELECTION_LAYER_DOWN:
            if (this->getLayerController()->getCurrentLayerId() >= 2) {
                // moveSelectionToLayer takes layer number (layerid - 1) not id
                // therefor the new layer is "layerid - 1 - 1"
                moveSelectionToLayer(getCurrentPage()->getSelectedLayerId() - 2);
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

        case ACTION_TOOL_HIGHLIGHTER:
            clearSelection();
            if (enabled) {
                selectTool(TOOL_HIGHLIGHTER);
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
        case ACTION_TOOL_SELECT_MULTILAYER_RECT:
            if (enabled) {
                selectTool(TOOL_SELECT_MULTILAYER_RECT);
            }
            break;
        case ACTION_TOOL_SELECT_MULTILAYER_REGION:
            if (enabled) {
                selectTool(TOOL_SELECT_MULTILAYER_REGION);
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
        case ACTION_TOOL_SELECT_PDF_TEXT_LINEAR:
            clearSelection();
            if (enabled) {
                selectTool(TOOL_SELECT_PDF_TEXT_LINEAR);
            }
            break;
        case ACTION_TOOL_SELECT_PDF_TEXT_RECT:
            clearSelection();
            if (enabled) {
                selectTool(TOOL_SELECT_PDF_TEXT_RECT);
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
        case ACTION_SETSQUARE: {
            bool needsNewSetsquare = !this->geometryToolController ||
                                     this->geometryToolController->getType() != GeometryToolType::SETSQUARE;
            resetGeometryTool();
            if (needsNewSetsquare) {
                makeGeometryTool<Setsquare, xoj::view::SetsquareView, SetsquareController, SetsquareInputHandler,
                                 ACTION_SETSQUARE>();
            }
            break;
        }
        case ACTION_COMPASS: {
            bool needsNewCompass = !this->geometryToolController ||
                                   this->geometryToolController->getType() != GeometryToolType::COMPASS;
            resetGeometryTool();
            if (needsNewCompass) {
                makeGeometryTool<Compass, xoj::view::CompassView, CompassController, CompassInputHandler,
                                 ACTION_COMPASS>();
            }
            break;
        }
        case ACTION_TOOL_FLOATING_TOOLBOX:
            if (enabled) {
                selectTool(TOOL_FLOATING_TOOLBOX);
            }
            break;
        case ACTION_TOOL_DRAW_RECT:
        case ACTION_TOOL_DRAW_ELLIPSE:
        case ACTION_TOOL_DRAW_ARROW:
        case ACTION_TOOL_DRAW_DOUBLE_ARROW:
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
        case ACTION_TOOL_PEN_FILL_OPACITY:
            selectFillAlpha(true);
            break;


        case ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE:
            if (enabled) {
                this->toolHandler->setHighlighterSize(TOOL_SIZE_VERY_FINE);
                highlighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HIGHLIGHTER_SIZE_FINE:
            if (enabled) {
                this->toolHandler->setHighlighterSize(TOOL_SIZE_FINE);
                highlighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM:
            if (enabled) {
                this->toolHandler->setHighlighterSize(TOOL_SIZE_MEDIUM);
                highlighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HIGHLIGHTER_SIZE_THICK:
            if (enabled) {
                this->toolHandler->setHighlighterSize(TOOL_SIZE_THICK);
                highlighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK:
            if (enabled) {
                this->toolHandler->setHighlighterSize(TOOL_SIZE_VERY_THICK);
                highlighterSizeChanged();
            }
            break;
        case ACTION_TOOL_HIGHLIGHTER_FILL:
            this->toolHandler->setHighlighterFillEnabled(enabled);
            break;
        case ACTION_TOOL_HIGHLIGHTER_FILL_OPACITY:
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
            setViewFullscreenMode(enabled);
            break;

        case ACTION_TOGGLE_PAIRS_PARITY: {
            int pairsOffset = settings->getPairsOffset();
            bool pairsEnabled = settings->isShowPairedPages();
            if (pairsOffset % 2 == 0) {
                setPairsOffset(pairsOffset + 1);
            } else {
                setPairsOffset(pairsOffset - 1);
            };
            setViewPairedPages(pairsEnabled);
            break;
        }

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
            if (!audioController) {
                g_warning("Audio has been disabled");
                return;
            }
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
            if (!audioController) {
                g_warning("Audio has been disabled");
                return;
            }
            if (enabled) {
                this->getAudioController()->pausePlayback();
            } else {
                this->getAudioController()->continuePlayback();
            }
            break;

        case ACTION_AUDIO_SEEK_FORWARDS:
            if (!audioController) {
                g_warning("Audio has been disabled");
                return;
            }

            this->getAudioController()->seekForwards();
            break;

        case ACTION_AUDIO_SEEK_BACKWARDS:
            if (!audioController) {
                g_warning("Audio has been disabled");
                return;
            }

            this->getAudioController()->seekBackwards();
            break;

        case ACTION_AUDIO_STOP_PLAYBACK:
            if (!audioController) {
                g_warning("Audio has been disabled");
                return;
            }

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

        case ACTION_NONE:
            // do nothing
            break;

        default:
            g_warning("Unhandled action event: %s / %s (%i / %i)", ActionType_toString(type).c_str(),
                      ActionGroup_toString(group).c_str(), type, group);
            Stacktrace::printStacktrace();
    }

    if (type >= ACTION_TOOL_PEN && type <= ACTION_TOOL_HAND) {
        auto at = static_cast<ActionType>(toolHandler->getToolType() - TOOL_PEN + ACTION_TOOL_PEN);
        if (type == at && !enabled) {
            fireActionSelected(GROUP_TOOL, at);
        }
    }
}

template <class ToolClass, class ViewClass, class ControllerClass, class InputHandlerClass, ActionType a>
void Control::makeGeometryTool() {
    const auto view = this->win->getXournal()->getViewFor(getCurrentPageNo());
    const auto* xournal = GTK_XOURNAL(this->win->getXournal()->getWidget());
    auto tool = new ToolClass();
    view->addOverlayView(std::make_unique<ViewClass>(tool, view, zoom));
    this->geometryTool = std::unique_ptr<GeometryTool>(tool);
    this->geometryToolController = std::make_unique<ControllerClass>(view, tool);
    std::unique_ptr<InputHandlerClass> geometryToolInputHandler =
            std::make_unique<InputHandlerClass>(this->win->getXournal(), geometryToolController.get());
    geometryToolInputHandler->registerToPool(tool->getHandlerPool());
    Range range = view->getVisiblePart();
    if (range.isValid()) {
        double originX = (range.minX + range.maxX) * .5;
        double originY = (range.minY + range.maxY) * .5;
        geometryToolController->translate(originX, originY);
    } else {
        geometryToolController->translate(view->getWidth() * .5, view->getHeight() * .5);
    }
    xournal->input->setGeometryToolInputHandler(std::move(geometryToolInputHandler));
    fireActionSelected(GROUP_GEOMETRY_TOOL, a);
    geometryTool->notify();
}

void Control::resetGeometryTool() {
    this->geometryToolController.reset();
    this->geometryTool.reset();
    auto* xournal = GTK_XOURNAL(this->win->getXournal()->getWidget());
    xournal->input->resetGeometryToolInputHandler();
    fireActionSelected(GROUP_GEOMETRY_TOOL, ACTION_NONE);
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
        alpha = toolHandler->getHighlighterFill();
    }

    FillOpacityDialog dlg(gladeSearchPath, alpha, pen);
    dlg.show(getGtkWindow());

    if (dlg.getResultAlpha() == -1) {
        return;
    }

    alpha = dlg.getResultAlpha();

    if (pen) {
        toolHandler->setPenFill(alpha);
    } else {
        toolHandler->setHighlighterFill(alpha);
    }
}

void Control::clearSelectionEndText() {
    clearSelection();
    if (win) {
        win->getXournal()->endTextAllPages();
    }
}

void Control::selectAllOnPage() {
    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
        return;
    }

    this->doc->lock();
    XojPageView* view = win->getXournal()->getViewFor(pageNr);
    if (view == nullptr) {
        this->doc->unlock();
        return;
    }

    PageRef page = this->doc->getPage(pageNr);
    Layer* layer = page->getSelectedLayer();

    win->getXournal()->clearSelection();

    if (layer->getElements().empty()) {
        this->doc->unlock();
        return;
    }

    EditSelection* selection = new EditSelection(this->undoRedo, view, page, layer);
    this->doc->unlock();

    win->getXournal()->setSelection(selection);
}

void Control::reorderSelection(const ActionType type) {
    EditSelection* sel = win->getXournal()->getSelection();
    if (!sel)
        return;

    EditSelection::OrderChange change;
    switch (type) {
        case ACTION_ARRANGE_BRING_TO_FRONT:
            change = EditSelection::OrderChange::BringToFront;
            break;
        case ACTION_ARRANGE_BRING_FORWARD:
            change = EditSelection::OrderChange::BringForward;
            break;
        case ACTION_ARRANGE_SEND_BACKWARD:
            change = EditSelection::OrderChange::SendBackward;
            break;
        case ACTION_ARRANGE_SEND_TO_BACK:
            change = EditSelection::OrderChange::SendToBack;
            break;
        default:
            // Unknown selection order, do nothing.
            return;
    }

    auto undoAction = sel->rearrangeInsertOrder(change);
    this->undoRedo->addUndoAction(std::move(undoAction));
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

    if (auto tbs = this->win->getToolbarModel()->getToolbars();
        std::find(tbs->begin(), tbs->end(), this->win->getSelectedToolbar()) == tbs->end()) {
        // The active toolbar has been deleted!
        assert(!tbs->empty());
        this->win->toolbarSelected(tbs->front());
        XojMsgBox::showErrorToUser(GTK_WINDOW(this->win->getWindow()),
                                   _("You deleted the active toolbar. Falling back to the default toolbar."));
    }

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

void Control::setShapeTool(ActionType type, bool enabled) {

    if (this->toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE && (type != ACTION_TOOL_DRAW_SPLINE || !enabled)) {
        // Shape changed from spline to something else: finish ongoing splines
        if (win) {
            win->getXournal()->endSplineAllPages();
        }
    }

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
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_DOUBLE_ARROW && type == ACTION_TOOL_DRAW_DOUBLE_ARROW) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_COORDINATE_SYSTEM &&
         type == ACTION_TOOL_DRAW_COORDINATE_SYSTEM) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_ELLIPSE && type == ACTION_TOOL_DRAW_ELLIPSE) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE && type == ACTION_TOOL_DRAW_SPLINE) ||
        (this->toolHandler->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER && type == ACTION_SHAPE_RECOGNIZER)) {
        return;
    }

    switch (type) {
        case ACTION_TOOL_DRAW_RECT:
            this->toolHandler->setDrawingType(DRAWING_TYPE_RECTANGLE);
            break;

        case ACTION_TOOL_DRAW_ELLIPSE:
            this->toolHandler->setDrawingType(DRAWING_TYPE_ELLIPSE);
            break;

        case ACTION_TOOL_DRAW_ARROW:
            this->toolHandler->setDrawingType(DRAWING_TYPE_ARROW);
            break;

        case ACTION_TOOL_DRAW_DOUBLE_ARROW:
            this->toolHandler->setDrawingType(DRAWING_TYPE_DOUBLE_ARROW);
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

    // if the current page contains the geometry tool, reset it
    size_t pNr = getCurrentPageNo();
    if (geometryToolController) {
        doc->lock();
        auto page = doc->indexOf(geometryToolController->getPage());
        doc->unlock();
        if (page == pNr) {
            resetGeometryTool();
        }
    }
    // don't allow delete pages if we have less than 2 pages,
    // so we can be (more or less) sure there is at least one page.
    if (this->doc->getPageCount() < 2) {
        return;
    }

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

void Control::duplicatePage() {
    auto page = getCurrentPage();
    if (!page) {
        return;
    }
    auto pageCopy = std::make_shared<XojPage>(*page);

    insertPage(pageCopy, getCurrentPageNo() + 1);
}

void Control::insertNewPage(size_t position, bool shouldScrollToPage) {
    pageBackgroundChangeController->insertNewPage(position, shouldScrollToPage);
}

void Control::appendNewPdfPages() {
    auto pageCount = this->doc->getPageCount();
    // find last page with pdf background and get its pdf page number
    auto currentPdfPageCount = [&]() {
        for (size_t i = pageCount; i != 0; --i) {
            if (auto page = doc->getPage(i - 1); page && page->getBackgroundType().isPdfPage()) {
                return page->getPdfPageNr() + 1;
            }
        }
        return size_t{0U};
    }();

    auto pdfPageCount = this->doc->getPdfPageCount();
    auto insertCount = pdfPageCount - currentPdfPageCount;

    if (insertCount == 0) {
        string msg = FS(_F("No pdf pages available to append. You may need to reopen the document first."));
        XojMsgBox::showErrorToUser(getGtkWindow(), msg);
    }
    for (size_t i = 0; i != insertCount; ++i) {

        doc->lock();
        XojPdfPageSPtr pdf = doc->getPdfPage(currentPdfPageCount + i);
        doc->unlock();

        if (pdf) {
            auto newPage = std::make_shared<XojPage>(pdf->getWidth(), pdf->getHeight());
            newPage->setBackgroundPdfPageNr(currentPdfPageCount + i);
            insertPage(newPage, pageCount + i);
        } else {
            string msg = FS(_F("Unable to retrieve pdf page."));  // should not happen
            XojMsgBox::showErrorToUser(getGtkWindow(), msg);
        }
    }
}

void Control::insertPage(const PageRef& page, size_t position, bool shouldScrollToPage) {
    this->doc->lock();
    this->doc->insertPage(page, position);  // insert the new page to the document and update page numbers
    this->doc->unlock();

    // notify document listeners about the inserted page; this creates the new XojViewPage, recalculates the layout
    // and creates a preview page in the sidebar
    firePageInserted(position);

    getCursor()->updateCursor();

    // make the inserted page fully visible (or at least as much from the top which fits on the screen),
    // and make the page appear selected
    if (shouldScrollToPage) {
        scrollHandler->scrollToPage(position);
        firePageSelected(position);
    }


    updateDeletePageButton();
    undoRedo->addUndoAction(std::make_unique<InsertDeletePageUndoAction>(page, position, true));
}

void Control::gotoPage() {
    auto dlg = GotoDialog(this->gladeSearchPath, int(this->doc->getPageCount()));

    dlg.show(GTK_WINDOW(this->win->getWindow()));
    auto page = dlg.getSelectedPage();

    if (page > 0) {
        this->scrollHandler->scrollToPage(size_t(page - 1), 0);
    }
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
    auto dlg = PageTemplateDialog(this->gladeSearchPath, settings, pageTypes);
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    if (dlg.isSaved()) {
        newPageType->loadDefaultPage();
    }
}

void Control::paperFormat() {
    auto const& page = getCurrentPage();
    if (!page || page->getBackgroundType().isPdfPage()) {
        return;
    }
    clearSelectionEndText();

    auto dlg = FormatDialog(this->gladeSearchPath, settings, page->getWidth(), page->getHeight());
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    double width = dlg.getWidth();
    double height = dlg.getHeight();

    if (width > 0) {
        this->doc->lock();
        Document::setPageSize(page, width, height);
        this->doc->unlock();
    }

    size_t pageNo = doc->indexOf(page);
    if (pageNo != npos && pageNo < doc->getPageCount()) {
        this->firePageSizeChanged(pageNo);
    }
}

void Control::changePageBackgroundColor() {
    auto pNr = getCurrentPageNo();
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
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewFullscreenMode(bool enabled) {
    if (enabled) {
        this->loadViewMode(VIEW_MODE_FULLSCREEN);
    } else {
        this->loadViewMode(VIEW_MODE_DEFAULT);
    }
}

void Control::setViewPresentationMode(bool enabled) {
    if (enabled) {
        this->loadViewMode(VIEW_MODE_PRESENTATION);

        bool success = zoom->updateZoomPresentationValue();
        if (!success) {
            g_warning("Error calculating zoom value");
            fireActionSelected(GROUP_PRESENTATION_MODE, ACTION_NOT_SELECTED);
            return;
        }
    } else {
        this->loadViewMode(VIEW_MODE_DEFAULT);

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
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setPairsOffset(int numOffset) {
    settings->setPairsOffset(numOffset);
    fireActionSelected(GROUP_PAIRED_PAGES, numOffset ? ACTION_SET_PAIRS_OFFSET : ACTION_NOT_SELECTED);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
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

auto Control::getCurrentPageNo() const -> size_t {
    if (this->win) {
        return this->win->getXournal()->getCurrentPage();
    }
    return 0;
}

auto Control::searchTextOnPage(const std::string& text, size_t pageNumber, size_t* occurrences,
                               double* yOfUpperMostMatch) -> bool {
    return getWindow()->getXournal()->searchTextOnPage(text, pageNumber, occurrences, yOfUpperMostMatch);
}

auto Control::getCurrentPage() -> PageRef {
    this->doc->lock();
    PageRef p = this->doc->getPage(getCurrentPageNo());
    this->doc->unlock();

    return p;
}

void Control::undoRedoChanged() {
    fireEnableAction(ACTION_UNDO, undoRedo->canUndo());
    fireEnableAction(ACTION_REDO, undoRedo->canRedo());

    win->setUndoDescription(undoRedo->undoDescription());
    win->setRedoDescription(undoRedo->redoDescription());

    updateWindowTitle();
}

void Control::undoRedoPageChanged(PageRef page) {
    if (std::find(begin(this->changedPages), end(this->changedPages), page) == end(this->changedPages)) {
        this->changedPages.emplace_back(std::move(page));
    }
}

void Control::selectTool(ToolType type) {
    // keep text-selection when switching from text to seletion tool
    auto oldTool = getToolHandler()->getActiveTool();
    if (oldTool && win && isSelectToolType(type) && oldTool->getToolType() == ToolType::TOOL_TEXT &&
        this->win->getXournal()->getTextEditor() && !(this->win->getXournal()->getTextEditor()->bufferEmpty())) {
        auto xournal = this->win->getXournal();
        Text* textobj = xournal->getTextEditor()->getTextElement();
        clearSelectionEndText();

        auto pageNr = getCurrentPageNo();
        XojPageView* view = xournal->getViewFor(pageNr);
        g_assert(view != nullptr);
        this->doc->lock();
        PageRef page = this->doc->getPage(pageNr);
        auto selection = new EditSelection(this->undoRedo, textobj, view, page);
        this->doc->unlock();

        xournal->setSelection(selection);
    }

    toolHandler->selectTool(type);
    toolHandler->fireToolChanged();
}

void Control::selectDefaultTool() {
    ButtonConfig* cfg = settings->getButtonConfig(Button::BUTTON_DEFAULT);
    cfg->applyConfigToToolbarTool(toolHandler);

    if (toolHandler->getToolType() != TOOL_NONE) {
        selectTool(toolHandler->getToolType());
    }
}

void Control::setFontSelected(const XojFont& font) { this->getWindow()->setFontButtonFont(font); }

void Control::toolChanged() {
    ToolType type = toolHandler->getToolType();

    // Convert enum values, enums has to be in the same order!
    auto at = static_cast<ActionType>(type - TOOL_PEN + ACTION_TOOL_PEN);

    fireActionSelected(GROUP_TOOL, at);

    fireEnableAction(ACTION_SELECT_COLOR, toolHandler->hasCapability(TOOL_CAP_COLOR));
    fireEnableAction(ACTION_SELECT_COLOR_CUSTOM, toolHandler->hasCapability(TOOL_CAP_COLOR));

    fireEnableAction(ACTION_RULER, toolHandler->hasCapability(TOOL_CAP_RULER));
    fireEnableAction(ACTION_TOOL_DRAW_RECT, toolHandler->hasCapability(TOOL_CAP_RECTANGLE));
    fireEnableAction(ACTION_TOOL_DRAW_ELLIPSE, toolHandler->hasCapability(TOOL_CAP_ELLIPSE));
    fireEnableAction(ACTION_TOOL_DRAW_ARROW, toolHandler->hasCapability(TOOL_CAP_ARROW));
    fireEnableAction(ACTION_TOOL_DRAW_DOUBLE_ARROW, toolHandler->hasCapability(TOOL_CAP_DOUBLE_ARROW));
    fireEnableAction(ACTION_TOOL_DRAW_COORDINATE_SYSTEM, toolHandler->hasCapability(TOOL_CAP_ARROW));
    fireEnableAction(ACTION_TOOL_DRAW_SPLINE, toolHandler->hasCapability(TOOL_CAP_SPLINE));
    fireEnableAction(ACTION_SHAPE_RECOGNIZER, toolHandler->hasCapability(TOOL_CAP_RECOGNIZER));

    bool enableSize = toolHandler->hasCapability(TOOL_CAP_SIZE);
    fireEnableAction(ACTION_SIZE_MEDIUM, enableSize);
    fireEnableAction(ACTION_SIZE_THICK, enableSize);
    fireEnableAction(ACTION_SIZE_FINE, enableSize);
    fireEnableAction(ACTION_SIZE_VERY_THICK, enableSize);
    fireEnableAction(ACTION_SIZE_VERY_FINE, enableSize);
    if (enableSize) {
        toolSizeChanged();
    }

    bool enableLineStyle = toolHandler->hasCapability(TOOL_CAP_LINE_STYLE);
    fireEnableAction(ACTION_TOOL_LINE_STYLE_PLAIN, enableLineStyle);
    fireEnableAction(ACTION_TOOL_LINE_STYLE_DASH, enableLineStyle);
    fireEnableAction(ACTION_TOOL_LINE_STYLE_DASH_DOT, enableLineStyle);
    fireEnableAction(ACTION_TOOL_LINE_STYLE_DOT, enableLineStyle);
    if (enableLineStyle) {
        toolLineStyleChanged();
    }

    bool enableFill = toolHandler->hasCapability(TOOL_CAP_FILL);
    fireEnableAction(ACTION_TOOL_FILL, enableFill);
    if (enableFill) {
        toolFillChanged();
    }

    // Update color
    if (toolHandler->hasCapability(TOOL_CAP_COLOR)) {
        toolColorChanged();
    }

    if (toolHandler->getToolType() == TOOL_PEN) {
        toolLineStyleChanged();
    }

    ActionType rulerAction = ACTION_NOT_SELECTED;
    if (toolHandler->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        rulerAction = ACTION_SHAPE_RECOGNIZER;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_LINE) {
        rulerAction = ACTION_RULER;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_RECTANGLE) {
        rulerAction = ACTION_TOOL_DRAW_RECT;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_ELLIPSE) {
        rulerAction = ACTION_TOOL_DRAW_ELLIPSE;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_ARROW) {
        rulerAction = ACTION_TOOL_DRAW_ARROW;
    } else if (toolHandler->getDrawingType() == DRAWING_TYPE_DOUBLE_ARROW) {
        rulerAction = ACTION_TOOL_DRAW_DOUBLE_ARROW;
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
    if (toolHandler->getDrawingType() != DRAWING_TYPE_SPLINE) {
        if (win) {
            win->getXournal()->endSplineAllPages();
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

void Control::highlighterSizeChanged() {
    switch (toolHandler->getHighlighterSize()) {
        case TOOL_SIZE_VERY_FINE:
            fireActionSelected(GROUP_HIGHLIGHTER_SIZE, ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_FINE);
            break;
        case TOOL_SIZE_FINE:
            fireActionSelected(GROUP_HIGHLIGHTER_SIZE, ACTION_TOOL_HIGHLIGHTER_SIZE_FINE);
            break;
        case TOOL_SIZE_MEDIUM:
            fireActionSelected(GROUP_HIGHLIGHTER_SIZE, ACTION_TOOL_HIGHLIGHTER_SIZE_MEDIUM);
            break;
        case TOOL_SIZE_THICK:
            fireActionSelected(GROUP_HIGHLIGHTER_SIZE, ACTION_TOOL_HIGHLIGHTER_SIZE_THICK);
            break;
        case TOOL_SIZE_VERY_THICK:
            fireActionSelected(GROUP_HIGHLIGHTER_SIZE, ACTION_TOOL_HIGHLIGHTER_SIZE_VERY_THICK);
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
    } else if (toolHandler->getToolType() == TOOL_HIGHLIGHTER) {
        highlighterSizeChanged();
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
    fireActionSelected(GROUP_HIGHLIGHTER_FILL,
                       toolHandler->getHighlighterFillEnabled() ? ACTION_TOOL_HIGHLIGHTER_FILL : ACTION_NONE);
}

void Control::toolLineStyleChanged() {
    std::optional<string> style = getLineStyleToSelect();

    if (!style) {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_NONE);
    } else if (style == "dash") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DASH);
    } else if (style == "dashdot") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DASH_DOT);
    } else if (style == "dot") {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_DOT);
    } else {
        fireActionSelected(GROUP_LINE_STYLE, ACTION_TOOL_LINE_STYLE_PLAIN);
    }
}

auto Control::getLineStyleToSelect() -> std::optional<string> const {
    const LineStyle& lineStyle = toolHandler->getTool(TOOL_PEN).getLineStyle();
    string style = StrokeStyle::formatStyle(lineStyle);

    if (!win) {
        return style;
    }

    const EditSelection* sel = win->getXournal()->getSelection();
    if (!sel) {
        return style;
    }

    bool isFirstPenStrokeElement = true;
    string previous_style = "none";

    // Todo(cpp20) Replace with std::ranges::filter_view and for_first_then_for_each
    for (const Element* e: sel->getElements()) {
        if (e->getType() == ELEMENT_STROKE) {
            const auto* s = dynamic_cast<const Stroke*>(e);

            if (s->getToolType().hasLineStyle()) {
                style = StrokeStyle::formatStyle(s->getLineStyle());

                if (isFirstPenStrokeElement) {
                    previous_style = style;
                    isFirstPenStrokeElement = false;
                } else {
                    if (style != previous_style) {
                        return std::nullopt;
                    }
                }
            }
        }
    }

    return style;
}

void Control::toolColorChanged() {
    fireActionSelected(GROUP_COLOR, ACTION_SELECT_COLOR);
    getCursor()->updateCursor();
}

void Control::changeColorOfSelection() {
    if (this->win && toolHandler->hasCapability(TOOL_CAP_COLOR)) {
        EditSelection* sel = this->win->getXournal()->getSelection();
        if (sel) {
            undoRedo->addUndoAction(sel->setColor(toolHandler->getColor()));
        }

        TextEditor* edit = getTextEditor();


        if (this->toolHandler->getToolType() == TOOL_TEXT && edit != nullptr) {
            // Todo move into selection
            edit->setColor(toolHandler->getColor());
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
    SidebarNumberingStyle sidebarStyle = settings->getSidebarNumberingStyle();

    auto dlg = SettingsDialog(this->gladeSearchPath, settings, this);
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    // note which settings have changed and act accordingly
    if (selectionColor != settings->getBorderColor()) {
        win->getXournal()->forceUpdatePagenumbers();
    }

    if (verticalSpace != settings->getAddVerticalSpace() || horizontalSpace != settings->getAddHorizontalSpace() ||
        verticalSpaceAmount != settings->getAddVerticalSpaceAmount() ||
        horizontalSpaceAmount != settings->getAddHorizontalSpaceAmount()) {
        win->getXournal()->layoutPages();
        scrollHandler->scrollToPage(getCurrentPageNo());
    }

    if (stylusCursorType != settings->getStylusCursorType() || highlightPosition != settings->isHighlightPosition()) {
        getCursor()->updateCursor();
    }

    win->updateScrollbarSidebarPosition();
    this->updateWindowTitle();

    enableAutosave(settings->isAutosaveEnabled());

    this->zoom->setZoomStep(settings->getZoomStep() / 100.0);
    this->zoom->setZoomStepScroll(settings->getZoomStepScroll() / 100.0);
    this->zoom->setZoom100Value(settings->getDisplayDpi() / Util::DPI_NORMALIZATION_FACTOR);

    if (sidebarStyle != settings->getSidebarNumberingStyle()) {
        getSidebar()->layout();
    }

    getWindow()->getXournal()->getHandRecognition()->reload();
    getWindow()->updateColorscheme();
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
    auto basePath = Util::getCacheSubfolder("");
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

    if (Util::hasPdfFileExt(filepath)) {
        return loadPdf(filepath, scrollToPage);
    }

    LoadHandler loadHandler;
    Document* loadedDocument = loadHandler.loadDocument(filepath);

    if (!loadedDocument) {
        string msg = FS(_F("Error opening file \"{1}\"") % filepath.u8string()) + "\n" + loadHandler.getLastError();
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

    if ((loadedDocument != nullptr && loadHandler.isAttachedPdfMissing()) ||
        !loadHandler.getMissingPdfFilename().empty()) {
        // give the user a second chance to select a new PDF filepath, or to discard the PDF
        promptMissingPdf(loadHandler, filepath);
    }

    return true;
}

auto Control::loadPdf(const fs::path& filepath, int scrollToPage) -> bool {
    LoadHandler loadHandler;

    if (settings->isAutoloadPdfXoj()) {
        Document* tmp;
        const std::vector<std::string> exts = {".xopp", ".xoj", ".pdf.xopp", ".pdf.xoj"};
        for (const std::string& ext: exts) {
            fs::path f = filepath;
            Util::clearExtensions(f, ".pdf");
            f += ext;
            tmp = loadHandler.loadDocument(f);
            if (tmp)
                break;
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

void Control::promptMissingPdf(LoadHandler& loadHandler, const fs::path& filepath) {
    const fs::path missingFilePath = fs::path(loadHandler.getMissingPdfFilename());

    // create error message
    std::string parentFolderPath;
    std::string filename;
#if defined(WIN32)
    parentFolderPath = missingFilePath.parent_path().string();
    filename = missingFilePath.filename().string();
#else
    // since POSIX systems detect the whole Windows path as a filename, this checks whether missingFilePath
    // contains a Windows path
    std::regex regex(R"([A-Z]:\\(?:.*\\)*(.*))");
    std::cmatch matchInfo;

    if (std::regex_match(missingFilePath.filename().string().c_str(), matchInfo, regex) && matchInfo[1].matched) {
        parentFolderPath = missingFilePath.filename().string();
        filename = matchInfo[1].str();
    } else {
        parentFolderPath = missingFilePath.parent_path().string();
        filename = missingFilePath.filename().string();
    }
#endif
    std::string msg;
    if (loadHandler.isAttachedPdfMissing()) {
        msg = FS(_F("The attached background file could not be found. It might have been moved, "
                    "renamed or deleted."));
    } else {
        msg = FS(_F("The background file \"{1}\" could not be found. It might have been moved, renamed or "
                    "deleted.\nIt was last seen at: \"{2}\"") %
                 filename % parentFolderPath);
    }

    // try to find file in current directory
    auto proposedPdfFilepath = filepath.parent_path() / filename;
    bool proposePdfFile = !loadHandler.isAttachedPdfMissing() && !filename.empty() && fs::exists(proposedPdfFilepath) &&
                          !fs::is_directory(proposedPdfFilepath);
    if (proposePdfFile) {
        msg += FS(_F("\nProposed replacement file: \"{1}\"") % proposedPdfFilepath.string());
    }

    // create the dialog
    GtkWidget* dialog = gtk_message_dialog_new(getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                               "%s", msg.c_str());

    enum dialogOptions { USE_PROPOSED, SELECT_OTHER, REMOVE, CANCEL };

    if (proposePdfFile) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), _("Use proposed PDF"), USE_PROPOSED);
    }
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another PDF"), SELECT_OTHER);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Remove PDF Background"), REMOVE);
    gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), CANCEL);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));
    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    switch (res) {
        case USE_PROPOSED:
            if (!proposedPdfFilepath.empty()) {
                this->pageBackgroundChangeController->changePdfPagesBackground(proposedPdfFilepath, false);
            }
            break;
        case SELECT_OTHER: {
            bool attachToDocument = false;
            XojOpenDlg dlg(getGtkWindow(), this->settings);
            auto pdfFilename = dlg.showOpenDialog(true, attachToDocument);
            if (!pdfFilename.empty()) {
                this->pageBackgroundChangeController->changePdfPagesBackground(pdfFilename, attachToDocument);
            }
        } break;
        case REMOVE:
            this->pageBackgroundChangeController->changeAllPagesBackground(PageType(PageTypeFormat::Plain));
            break;
        default:
            break;
    }
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
    // Do not call again!
    return false;
}

void Control::loadMetadata(MetadataEntry md) {
    auto* data = new MetadataCallbackData();
    data->md = std::move(md);
    data->ctrl = this;

    g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, xoj::util::wrap_v<loadMetadataCallback>, data,
                    &xoj::util::destroy_cb<MetadataCallbackData>);
}

auto Control::annotatePdf(fs::path filepath, bool /*attachPdf*/, bool attachToDocument) -> bool {
    if (!this->close(false)) {
        return false;
    }

    // Prompt the user for a path if none is provided.
    if (filepath.empty()) {
        XojOpenDlg dlg(getGtkWindow(), this->settings);
        filepath = dlg.showOpenDialog(true, attachToDocument);
        if (filepath.empty()) {
            return false;
        }
    }

    // First, we create a dummy document and load the PDF into it.
    // We do NOT reset the current document yet because loading could fail.
    getCursor()->setCursorBusy(true);
    auto newDoc = std::make_unique<Document>(this);
    newDoc->setFilepath("");

    const bool res = newDoc->readPdf(filepath, /*initPages=*/true, attachToDocument);
    getCursor()->setCursorBusy(false);

    if (!res) {
        // Loading failed, so display the error to the user.
        newDoc->lock();
        std::string errMsg = newDoc->getLastErrorMsg();
        newDoc->unlock();

        std::string msg = FS(_F("Error annotate PDF file \"{1}\"\n{2}") % filepath.u8string() % errMsg);
        XojMsgBox::showErrorToUser(getGtkWindow(), msg);
        return false;
    }

    // Success, so we can close the current document.
    this->closeDocument();
    // Then we overwrite the global document with the new document.
    // FIXME: there could potentially be a data race if a job requires the old document but runs after it is closed
    {
        std::lock_guard<Document> lg(*doc);
        // TODO: allow Document to be moved
        *doc = *newDoc;
    }

    // Trigger callbacks and update UI
    fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
    fileLoaded();

    return true;
}

void Control::print() {
    this->doc->lock();
    PrintHandler::print(this->doc, getCurrentPageNo(), this->getGtkWindow());
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

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFilename(suggested_folder).c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), Util::toGFilename(suggested_name).c_str());
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),
                                         Util::toGFilename(this->settings->getLastOpenPath()).c_str(), nullptr);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), false);  // handled below

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->getWindow()->getWindow()));

    while (true) {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return false;
        }

        auto fileTmp = Util::fromGFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        Util::clearExtensions(fileTmp);
        fileTmp += ".xopp";
        // Since we add the extension after the OK button, we have to check manually on existing files
        if (askToReplace(fileTmp, GTK_WINDOW(dialog))) {
            break;
        }
    }

    auto filename = Util::fromGFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
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

            if (settings->isFilepathInTitlebarShown()) {
                title += ("[" + doc->getPdfFilepath().parent_path().u8string() + "] - " +
                          doc->getPdfFilepath().filename().u8string());
            } else {
                title += doc->getPdfFilepath().filename().u8string();
            }
        }
    } else {
        if (undoRedo->isChanged()) {
            title += "*";
        }

        if (settings->isFilepathInTitlebarShown()) {
            title += ("[" + doc->getFilepath().parent_path().u8string() + "] - " +
                      doc->getFilepath().filename().u8string());
        } else {
            title += doc->getFilepath().filename().u8string();
        }
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

    // no lock needed, this is an uncritical operation
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

    if (audioController) {
        audioController->stopRecording();
    }
    this->scheduler->lock();
    this->scheduler->removeAllJobs();
    this->scheduler->unlock();
    this->scheduler->stop();  // Finish current task. Must be called to finish pending saves.
    this->closeDocument();    // Must be done after all jobs has finished (Segfault on save/export)
    settings->save();
    g_application_quit(G_APPLICATION(gtkApp));
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
    resetGeometryTool();
    return true;
}

void Control::closeDocument() {
    this->undoRedo->clearContents();

    this->doc->lock();
    this->doc->clearDocument(true);
    this->doc->unlock();

    this->undoRedoChanged();
}

void Control::applyPreferredLanguage() {
    auto const& lang = this->settings->getPreferredLocale();
#ifdef _WIN32
    _putenv_s("LANGUAGE", lang.c_str());
#else
    setenv("LANGUAGE", lang.c_str(), 1);
#endif
}

void Control::initButtonTool() {
    std::vector<Button> buttons{Button::BUTTON_ERASER,       Button::BUTTON_STYLUS_ONE,  Button::BUTTON_STYLUS_TWO,
                                Button::BUTTON_MOUSE_MIDDLE, Button::BUTTON_MOUSE_RIGHT, Button::BUTTON_TOUCH};
    ButtonConfig* cfg;
    for (auto b: buttons) {
        cfg = settings->getButtonConfig(b);
        cfg->initButton(this->toolHandler, b);
    }
}

auto Control::askToReplace(fs::path const& filepath, GtkWindow* parent) const -> bool {
    if (fs::exists(filepath)) {
        std::string msg = FS(FORMAT_STR("The file {1} already exists! Do you want to replace it?") %
                             filepath.filename().u8string());
        int res = XojMsgBox::replaceFileQuestion(parent, msg);
        return res == GTK_RESPONSE_OK;
    }
    return true;
}

void Control::showAbout() {
    AboutDialog dlg(this->gladeSearchPath);
    dlg.show(GTK_WINDOW(this->win->getWindow()));
}

auto Control::loadViewMode(ViewModeId mode) -> bool {
    if (!settings->loadViewMode(mode)) {
        return false;
    }
    this->win->setMenubarVisible(settings->isMenubarVisible());
    this->win->setToolbarVisible(settings->isToolbarVisible());
    this->win->setSidebarVisible(settings->isSidebarVisible());
    setFullscreen(settings->isFullscreen());
    return false;
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
    t->setColor(toolHandler->getTool(TOOL_TEXT).getColor());

    clipboardPaste(t);
}

void Control::clipboardPasteImage(GdkPixbuf* img) {
    auto image = new Image();
    image->setImage(img);

    auto width =
            static_cast<double>(gdk_pixbuf_get_width(img)) / settings->getDisplayDpi() * Util::DPI_NORMALIZATION_FACTOR;
    auto height = static_cast<double>(gdk_pixbuf_get_height(img)) / settings->getDisplayDpi() *
                  Util::DPI_NORMALIZATION_FACTOR;

    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
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
    auto pageNr = getCurrentPageNo();
    if (pageNr == npos) {
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

void Control::registerPluginToolButtons(ToolMenuHandler* toolMenuHandler) {
    pluginController->registerToolButtons(toolMenuHandler);
}

void Control::clipboardPasteXournal(ObjectInputStream& in) {
    auto pNr = getCurrentPageNo();
    if (pNr == npos && win != nullptr) {
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
            selection->addElement(element.release(), Element::InvalidIndex);
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
    } catch (const std::exception& e) {
        g_warning("could not paste, Exception occurred: %s", e.what());
        Stacktrace::printStacktrace();
        if (selection) {
            for (Element* el: selection->getElements()) {
                delete el;
            }
            delete selection;
        }
    }
}

void Control::moveSelectionToLayer(size_t layerNo) {
    PageRef currentP = getCurrentPage();
    if (layerNo >= currentP->getLayerCount()) {
        return;
    }
    auto selection = getWindow()->getXournal()->getSelection();
    if (!selection) {
        return;
    }

    auto* oldLayer = currentP->getSelectedLayer();
    auto* newLayer = currentP->getLayers()->at(layerNo);
    auto moveSelUndo = std::make_unique<MoveSelectionToLayerUndoAction>(currentP, getLayerController(), oldLayer,
                                                                        currentP->getSelectedLayerId() - 1, layerNo);
    for (auto* e: selection->getElements()) {
        moveSelUndo->addElement(newLayer, e, newLayer->indexOf(e));
    }
    undoRedo->addUndoAction(std::move(moveSelUndo));

    getLayerController()->switchToLay(layerNo + 1, /*hideShow=*/false, /*clearSelection=*/false);
}

void Control::deleteSelection() {
    if (win) {
        win->getXournal()->deleteSelection();
    }
}

void Control::clearSelection() {
    if (this->win) {
        this->win->getXournal()->clearSelection();
        this->win->getPdfToolbox()->userCancelSelection();
    }
}

void Control::setClipboardHandlerSelection(EditSelection* selection) {
    if (this->clipboardHandler) {
        this->clipboardHandler->setSelection(selection);
    }
}

void Control::addChangedDocumentListener(DocumentListener* dl) { this->changedDocumentListeners.push_back(dl); }

void Control::removeChangedDocumentListener(DocumentListener* dl) { this->changedDocumentListeners.remove(dl); }

void Control::setCopyCutEnabled(bool enabled) { this->clipboardHandler->setCopyCutEnabled(enabled); }

void Control::setFill(bool fill) {
    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }

    if (sel) {
        undoRedo->addUndoAction(UndoActionPtr(
                sel->setFill(fill ? toolHandler->getPenFill() : -1, fill ? toolHandler->getHighlighterFill() : -1)));
    }
    toolHandler->setFillEnabled(fill, true);
}

void Control::setLineStyle(const string& style) {
    LineStyle stl = StrokeStyle::parseStyle(style);

    EditSelection* sel = nullptr;
    if (this->win) {
        sel = this->win->getXournal()->getSelection();
    }

    if (sel) {
        undoRedo->addUndoAction(sel->setLineStyle(stl));
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
                                                           toolHandler->getToolThickness(TOOL_HIGHLIGHTER),
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

auto Control::getUndoRedoHandler() const -> UndoRedoHandler* { return this->undoRedo; }

auto Control::getZoomControl() const -> ZoomControl* { return this->zoom; }

auto Control::getCursor() const -> XournalppCursor* { return this->cursor; }

auto Control::getDocument() const -> Document* { return this->doc; }

auto Control::getToolHandler() const -> ToolHandler* { return this->toolHandler; }

auto Control::getScheduler() const -> XournalScheduler* { return this->scheduler; }

auto Control::getWindow() const -> MainWindow* { return this->win; }

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

void Control::highlightPositionToggle() {
    settings->setHighlightPosition(!settings->isHighlightPosition());
    fireActionSelected(GROUP_HIGHLIGHT_POSITION,
                       settings->isHighlightPosition() ? ACTION_HIGHLIGHT_POSITION : ACTION_NONE);
}

auto Control::getTextEditor() -> TextEditor* {
    if (this->win) {
        return this->win->getXournal()->getTextEditor();
    }
    return nullptr;
}

auto Control::getGladeSearchPath() const -> GladeSearchpath* { return this->gladeSearchPath; }

auto Control::getSettings() const -> Settings* { return settings; }

auto Control::getScrollHandler() const -> ScrollHandler* { return this->scrollHandler; }

auto Control::getMetadataManager() const -> MetadataManager* { return this->metadata; }

auto Control::getSidebar() const -> Sidebar* { return this->sidebar; }

auto Control::getSearchBar() const -> SearchBar* { return this->searchBar; }

auto Control::getAudioController() const -> AudioController* { return this->audioController; }

auto Control::getPageTypes() const -> PageTypeHandler* { return this->pageTypes; }

auto Control::getNewPageType() const -> PageTypeMenu* { return this->newPageType.get(); }

auto Control::getPageBackgroundChangeController() const -> PageBackgroundChangeController* {
    return this->pageBackgroundChangeController;
}

auto Control::getLayerController() const -> LayerController* { return this->layerController; }

auto Control::getPluginController() const -> PluginController* { return this->pluginController; }
