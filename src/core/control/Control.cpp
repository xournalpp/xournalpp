#include "Control.h"

#include <algorithm>   // for max
#include <cstdlib>     // for size_t
#include <exception>   // for exce...
#include <functional>  // for bind
#include <iterator>   // for end
#include <locale>
#include <memory>    // for make...
#include <numeric>   // for accu...
#include <optional>  // for opti...
#include <regex>     // for regex
#include <utility>   // for move

#include "control/AudioController.h"                             // for Audi...
#include "control/ClipboardHandler.h"                            // for Clip...
#include "control/CompassController.h"                           // for Comp...
#include "control/RecentManager.h"                               // for Rece...
#include "control/ScrollHandler.h"                               // for Scro...
#include "control/SetsquareController.h"                         // for Sets...
#include "control/Tool.h"                                        // for Tool
#include "control/ToolHandler.h"                                 // for Tool...
#include "control/actions/ActionDatabase.h"                      // for Acti...
#include "control/jobs/AutosaveJob.h"                            // for Auto...
#include "control/jobs/BaseExportJob.h"                          // for Base...
#include "control/jobs/CustomExportJob.h"                        // for Cust...
#include "control/jobs/PdfExportJob.h"                           // for PdfE...
#include "control/jobs/SaveJob.h"                                // for SaveJob
#include "control/jobs/Scheduler.h"                              // for JOB_...
#include "control/jobs/XournalScheduler.h"                       // for Xour...
#include "control/layer/LayerController.h"                       // for Laye...
#include "control/pagetype/PageTypeHandler.h"                    // for Page...
#include "control/settings/ButtonConfig.h"                       // for Butt...
#include "control/settings/MetadataManager.h"                    // for Meta...
#include "control/settings/PageTemplateSettings.h"               // for Page...
#include "control/settings/Settings.h"                           // for Sett...
#include "control/settings/SettingsEnums.h"                      // for Button
#include "control/settings/ViewModes.h"                          // for ViewM..
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
#include "gui/dialog/FormatDialog.h"                             // for Form...
#include "gui/dialog/GotoDialog.h"                               // for Goto...
#include "gui/dialog/PageTemplateDialog.h"                       // for Page...
#include "gui/dialog/SelectBackgroundColorDialog.h"              // for Sele...
#include "gui/dialog/SelectOpacityDialog.h"                      // for Opac...
#include "gui/dialog/SettingsDialog.h"                           // for Sett...
#include "gui/dialog/ToolbarManageDialog.h"                      // for Tool...
#include "gui/dialog/toolbarCustomize/ToolbarDragDropHandler.h"  // for Tool...
#include "gui/inputdevices/CompassInputHandler.h"                // for Comp...
#include "gui/inputdevices/GeometryToolInputHandler.h"           // for Geom...
#include "gui/inputdevices/HandRecognition.h"                    // for Hand...
#include "gui/inputdevices/SetsquareInputHandler.h"              // for Sets...
#include "gui/menus/menubar/Menubar.h"                           // for Menubar
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
#include "util/Assert.h"                                         // for xoj_assert
#include "util/Color.h"                                          // for oper...
#include "util/PathUtil.h"                                       // for clea...
#include "util/PlaceholderString.h"                              // for Plac...
#include "util/PopupWindowWrapper.h"                             // for PopupWindowWrapper
#include "util/Stacktrace.h"                                     // for Stac...
#include "util/Util.h"                                           // for exec...
#include "util/XojMsgBox.h"                                      // for XojM...
#include "util/glib_casts.h"                                     // for wrap_v
#include "util/i18n.h"                                           // for _, FS
#include "util/safe_casts.h"                                     // for as_unsigned
#include "util/serializing/InputStreamException.h"               // for Inpu...
#include "util/serializing/ObjectInputStream.h"                  // for Obje...
#include "view/CompassView.h"                                    // for Comp...
#include "view/SetsquareView.h"                                  // for Sets...
#include "view/overlays/OverlayView.h"                           // for Over...

#include "CrashHandler.h"                    // for emer...
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

    auto name = Util::getConfigFile(SETTINGS_XML_FILE);
    this->settings = new Settings(std::move(name));
    this->settings->load();

    this->pageTypes = new PageTypeHandler(gladeSearchPath);

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

    this->toolHandler = new ToolHandler(this, this->actionDB.get(), this->settings);
    this->toolHandler->loadSettings();
    this->initButtonTool();

    /**
     * This is needed to update the previews
     */
    this->changeTimout = g_timeout_add_seconds(5, xoj::util::wrap_v<checkChangedDocument>, this);

    this->pageBackgroundChangeController = std::make_unique<PageBackgroundChangeController>(this);

    this->layerController = new LayerController(this);
    this->layerController->registerListener(this);

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
    delete this->layerController;
    this->layerController = nullptr;
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
    this->win = win;

    this->actionDB = std::make_unique<ActionDatabase>(this);

    selectTool(toolHandler->getToolType());
    this->sidebar = new Sidebar(win, this);

    XojMsgBox::setDefaultWindow(getGtkWindow());

    updatePageNumbers(0, npos);

    // toolHandler->eraserTypeChanged();

    this->searchBar = new SearchBar(this);

    if (settings->isPresentationMode()) {
        setViewPresentationMode(true);
    } else if (settings->isViewFixedRows()) {
        setViewRows(settings->getViewRows());
    } else {
        setViewColumns(settings->getViewColumns());
    }

    setViewLayoutType(settings->getViewLayoutType());
    setViewLayoutVert(settings->getViewLayoutVert());
    setViewLayoutR2L(settings->getViewLayoutR2L());
    setViewLayoutB2T(settings->getViewLayoutB2T());

    setViewPairedPages(settings->isShowPairedPages());

    toolLineStyleChanged();

    this->clipboardHandler = new ClipboardHandler(this, win->getXournal()->getWidget());

    this->enableAutosave(settings->isAutosaveEnabled());
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

    this->metadata->storeMetadata(this->doc->getEvMetadataFilename(), static_cast<int>(page),
                                  getZoomControl()->getZoomReal());
    if (settings->isPageNumberInTitlebarShown()) {
        this->updateWindowTitle();
    }

    auto current = getCurrentPageNo();
    auto count = this->doc->getPageCount();

    this->actionDB->enableAction(Action::GOTO_FIRST, current != 0);
    this->actionDB->enableAction(Action::GOTO_PREVIOUS, current != 0);
    this->actionDB->enableAction(Action::GOTO_PREVIOUS_ANNOTATED_PAGE, current != 0);
    this->actionDB->enableAction(Action::GOTO_PAGE, count > 1);
    this->actionDB->enableAction(Action::GOTO_NEXT, current < count - 1);
    this->actionDB->enableAction(Action::GOTO_LAST, current < count - 1);
    this->actionDB->enableAction(Action::GOTO_NEXT_ANNOTATED_PAGE, current < count - 1);
}

bool Control::toggleCompass() {
    return toggleGeometryTool<Compass, xoj::view::CompassView, CompassController, CompassInputHandler,
                              GeometryToolType::COMPASS>();
}
bool Control::toggleSetsquare() {
    return toggleGeometryTool<Setsquare, xoj::view::SetsquareView, SetsquareController, SetsquareInputHandler,
                              GeometryToolType::SETSQUARE>();
}

template <class ToolClass, class ViewClass, class ControllerClass, class InputHandlerClass, GeometryToolType toolType>
bool Control::toggleGeometryTool() {
    bool needsNewGeometryTool = !this->geometryToolController || this->geometryToolController->getType() != toolType;
    this->resetGeometryTool();
    if (!needsNewGeometryTool) {
        return false;
    }
    const auto view = this->win->getXournal()->getViewFor(getCurrentPageNo());
    const auto* xournal = GTK_XOURNAL(this->win->getXournal()->getWidget());
    auto tool = new ToolClass();
    view->addOverlayView(std::make_unique<ViewClass>(tool, view, zoom));
    this->geometryTool = std::unique_ptr<GeometryTool>(tool);
    this->geometryToolController = std::make_unique<ControllerClass>(view, tool);
    std::unique_ptr<InputHandlerClass> geometryToolInputHandler =
            std::make_unique<InputHandlerClass>(this->win->getXournal(), geometryToolController.get());
    geometryToolInputHandler->registerToPool(tool->getHandlerPool());
    xournal->input->setGeometryToolInputHandler(std::move(geometryToolInputHandler));
    geometryTool->notify();
    return true;
}

void Control::resetGeometryTool() {
    this->geometryToolController.reset();
    this->geometryTool.reset();
    auto* xournal = GTK_XOURNAL(this->win->getXournal()->getWidget());
    xournal->input->resetGeometryToolInputHandler();
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

void Control::selectAlpha(OpacityFeature feature) {
    int alpha = 0;

    switch (feature) {
        case OPACITY_FILL_PEN:
            alpha = this->toolHandler->getPenFill();
            break;
        case OPACITY_FILL_HIGHLIGHTER:
            alpha = this->toolHandler->getHighlighterFill();
            break;
        case OPACITY_SELECT_PDF_TEXT_MARKER:
            alpha = this->toolHandler->getSelectPDFTextMarkerOpacity();
            break;
        default:
            g_warning("Unhandled OpacityFeature for selectAlpha event: %s", opacityFeatureToString(feature).c_str());
            Stacktrace::printStracktrace();
            break;
    }
    auto dlg = xoj::popup::PopupWindowWrapper<xoj::popup::SelectOpacityDialog>(
            gladeSearchPath, alpha, feature, [&th = *toolHandler](int alpha, OpacityFeature feature) {
                switch (feature) {
                    case OPACITY_FILL_PEN:
                        th.setPenFill(alpha);
                        break;
                    case OPACITY_FILL_HIGHLIGHTER:
                        th.setHighlighterFill(alpha);
                        break;
                    case OPACITY_SELECT_PDF_TEXT_MARKER:
                        th.setSelectPDFTextMarkerOpacity(alpha);
                        break;
                    default:
                        g_warning("Unhandled OpacityFeature for callback of SelectOpacityDialog: %s",
                                  opacityFeatureToString(feature).c_str());
                        Stacktrace::printStracktrace();
                        break;
                }
            });
    dlg.show(getGtkWindow());
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

    auto elements = layer->clearNoFree();
    this->doc->unlock();

    if (!elements.empty()) {
        InsertionOrder insertionOrder;
        insertionOrder.reserve(elements.size());
        Element::Index n = 0;
        for (auto&& e: elements) {
            insertionOrder.emplace_back(std::move(e), n++);
        }
        auto [sel, rg] =
                SelectionFactory::createFromFloatingElements(this, page, layer, view, std::move(insertionOrder));

        page->fireRangeChanged(rg);

        win->getXournal()->setSelection(sel.release());
    }
}

void Control::reorderSelection(EditSelection::OrderChange change) {
    EditSelection* sel = win->getXournal()->getSelection();
    if (!sel) {
        return;
    }

    auto undoAction = sel->rearrangeInsertionOrder(change);
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
    xoj::popup::PopupWindowWrapper<ToolbarManageDialog> dlg(this->gladeSearchPath, this->win->getToolbarModel(),
                                                            [win = this->win]() {
                                                                win->updateToolbarMenu();
                                                                auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
                                                                win->getToolbarModel()->save(filepath);
                                                            });
    dlg.show(GTK_WINDOW(this->win->getWindow()));

    if (auto tbs = this->win->getToolbarModel()->getToolbars();
        std::find(tbs->begin(), tbs->end(), this->win->getSelectedToolbar()) == tbs->end()) {
        // The active toolbar has been deleted!
        xoj_assert(!tbs->empty());
        this->win->toolbarSelected(tbs->front());
        XojMsgBox::showErrorToUser(GTK_WINDOW(this->win->getWindow()),
                                   _("You deleted the active toolbar. Falling back to the default toolbar."));
    }

    this->win->updateToolbarMenu();
    auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
    this->win->getToolbarModel()->save(filepath);
}

void Control::customizeToolbars() {
    xoj_assert(this->win != nullptr);

    if (this->win->getSelectedToolbar()->isPredefined()) {
        enum { YES = 1, NO };
        XojMsgBox::askQuestion(getGtkWindow(),
                               FC(_F("The Toolbarconfiguration \"{1}\" is predefined, "
                                     "would you create a copy to edit?") %
                                  this->win->getSelectedToolbar()->getName()),
                               std::string(), {{_("Yes"), YES}, {_("No"), NO}}, [ctrl = this](int response) {
                                   if (response == YES) {
                                       auto* data = new ToolbarData(*ctrl->win->getSelectedToolbar());
                                       ToolbarModel* model = ctrl->win->getToolbarModel();
                                       model->initCopyNameId(data);
                                       model->add(data);
                                       ctrl->win->toolbarSelected(data);
                                       ctrl->win->updateToolbarMenu();

                                       xoj_assert(!ctrl->win->getSelectedToolbar()->isPredefined());
                                       ctrl->customizeToolbars();
                                   }
                               });
    } else {
        if (!this->dragDropHandler) {
            this->dragDropHandler = new ToolbarDragDropHandler(this);
        }
        this->dragDropHandler->configure();
    }
}

void Control::setToolDrawingType(DrawingType type) {
    if (this->toolHandler->getDrawingType() != type) {

        if (this->toolHandler->getDrawingType() == DRAWING_TYPE_SPLINE) {
            // Shape changed from spline to something else: finish ongoing splines
            if (win) {
                win->getXournal()->endSplineAllPages();
            }
        }
        this->toolHandler->setDrawingType(type);
    }
}

void Control::setFullscreen(bool enabled) {
    win->setFullscreen(enabled);
    actionDB->setActionState(Action::FULLSCREEN, enabled);
}

void Control::setShowSidebar(bool enabled) {
    win->setSidebarVisible(enabled);
    actionDB->setActionState(Action::SHOW_SIDEBAR, enabled);

    if (settings->isSidebarVisible() != enabled &&
        settings->getActiveViewMode() == PresetViewModeIds::VIEW_MODE_DEFAULT) {
        settings->setSidebarVisible(enabled);
        ViewMode viewMode = settings->getViewModes()[PresetViewModeIds::VIEW_MODE_DEFAULT];
        viewMode.showSidebar = enabled;
        settings->setViewMode(PresetViewModeIds::VIEW_MODE_DEFAULT, viewMode);
    }
}

void Control::setShowToolbar(bool enabled) {
    win->setToolbarVisible(enabled);
    actionDB->setActionState(Action::SHOW_TOOLBAR, enabled);

    if (settings->isToolbarVisible() != enabled &&
        settings->getActiveViewMode() == PresetViewModeIds::VIEW_MODE_DEFAULT) {
        settings->setToolbarVisible(enabled);
        ViewMode viewMode = settings->getViewModes()[PresetViewModeIds::VIEW_MODE_DEFAULT];
        viewMode.showToolbar = enabled;
        settings->setViewMode(PresetViewModeIds::VIEW_MODE_DEFAULT, viewMode);
    }
}

void Control::setShowMenubar(bool enabled) {
    win->setMenubarVisible(enabled);
    actionDB->setActionState(Action::SHOW_MENUBAR, enabled);

    if (settings->isMenubarVisible() != enabled &&
        settings->getActiveViewMode() == PresetViewModeIds::VIEW_MODE_DEFAULT) {
        settings->setMenubarVisible(enabled);
        ViewMode viewMode = settings->getViewModes()[PresetViewModeIds::VIEW_MODE_DEFAULT];
        viewMode.showMenubar = enabled;
        settings->setViewMode(PresetViewModeIds::VIEW_MODE_DEFAULT, viewMode);
    }
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
    this->actionDB->enableAction(Action::DELETE_PAGE, this->doc->getPageCount() > 1);
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

    scrollHandler->scrollToPage(pNr);
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
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::GotoDialog>(
            this->gladeSearchPath, this->getCurrentPageNo(), this->doc->getPageCount(),
            [scroll = this->scrollHandler](size_t pageNumber) {
                xoj_assert(pageNumber != 0);
                scroll->scrollToPage(pageNumber - 1);
            });
    popup.show(GTK_WINDOW(this->win->getWindow()));
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

    PageType bg = p->getBackgroundType();
    this->actionDB->enableAction(Action::PAPER_BACKGROUND_COLOR, !bg.isSpecial());
    // PDF page size is defined, you cannot change it
    this->actionDB->enableAction(Action::PAPER_FORMAT, !bg.isPdfPage());
}

void Control::paperTemplate() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::PageTemplateDialog>(this->gladeSearchPath, settings,
                                                                                win->getToolMenuHandler(), pageTypes);
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::paperFormat() {
    auto const& page = getCurrentPage();
    if (!page || page->getBackgroundType().isPdfPage()) {
        return;
    }
    clearSelectionEndText();

    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::FormatDialog>(
            this->gladeSearchPath, settings, page->getWidth(), page->getHeight(),
            [ctrl = this, page](double width, double height) {
                ctrl->doc->lock();
                Document::setPageSize(page, width, height);
                size_t pageNo = ctrl->doc->indexOf(page);
                size_t pageCount = ctrl->doc->getPageCount();
                ctrl->doc->unlock();
                if (pageNo != npos && pageNo < pageCount) {
                    ctrl->firePageSizeChanged(pageNo);
                }
            });
    popup.show(GTK_WINDOW(this->win->getWindow()));
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

    xoj::popup::PopupWindowWrapper<SelectBackgroundColorDialog> dlg(this, [p, pNr, ctrl = this](Color color) {
        p->setBackgroundColor(color);
        ctrl->firePageChanged(pNr);
    });
    dlg.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::setViewPairedPages(bool enabled) {
    settings->setShowPairedPages(enabled);
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
            return;
        }
    } else {
        this->loadViewMode(VIEW_MODE_DEFAULT);

        if (settings->isViewFixedRows()) {
            setViewRows(settings->getViewRows());
        } else {
            setViewColumns(settings->getViewColumns());
        }

        setViewLayoutType(settings->getViewLayoutType());
        setViewLayoutVert(settings->getViewLayoutVert());
        setViewLayoutR2L(settings->getViewLayoutR2L());
        setViewLayoutB2T(settings->getViewLayoutB2T());
    }
    zoom->setZoomPresentationMode(enabled);
    settings->setPresentationMode(enabled);

    // Disable Zoom
    this->actionDB->enableAction(Action::ZOOM_IN, !enabled);
    this->actionDB->enableAction(Action::ZOOM_OUT, !enabled);
    this->actionDB->enableAction(Action::ZOOM_FIT, !enabled);
    this->actionDB->enableAction(Action::ZOOM_100, !enabled);

    // TODO Figure out how to replace this
    // fireEnableAction(ACTION_FOOTER_ZOOM_SLIDER, !enabled);

    this->actionDB->enableAction(Action::SET_LAYOUT_BOTTOM_TO_TOP, !enabled);
    this->actionDB->enableAction(Action::SET_LAYOUT_RIGHT_TO_LEFT, !enabled);
    this->actionDB->enableAction(Action::SET_LAYOUT_VERTICAL, !enabled);
    this->actionDB->enableAction(Action::SET_COLUMNS_OR_ROWS, !enabled);

    // disable selection of scroll hand tool
    // TODO Figure out how to replace this
    // fireEnableAction(ACTION_TOOL_HAND, !enabled);

    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setPairsOffset(int numOffset) {
    settings->setPairsOffset(numOffset);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewColumns(int numColumns) {
    settings->setViewColumns(numColumns);
    settings->setViewFixedRows(false);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewRows(int numRows) {
    settings->setViewRows(numRows);
    settings->setViewFixedRows(true);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewLayoutType(LayoutType type) {
    settings->setViewLayoutType(type);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewLayoutVert(bool vert) {
    settings->setViewLayoutVert(vert);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewLayoutR2L(bool r2l) {
    settings->setViewLayoutR2L(r2l);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

void Control::setViewLayoutB2T(bool b2t) {
    settings->setViewLayoutB2T(b2t);
    win->getXournal()->layoutPages();
    scrollHandler->scrollToPage(getCurrentPageNo());
}

auto Control::getCurrentPageNo() const -> size_t {
    if (this->win) {
        return this->win->getXournal()->getCurrentPage();
    }
    return 0;
}

auto Control::searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                               XojPdfRectangle* matchRect) -> bool {
    return getWindow()->getXournal()->searchTextOnPage(text, pageNumber, index, occurrences, matchRect);
}

auto Control::getCurrentPage() -> PageRef {
    this->doc->lock();
    PageRef p = this->doc->getPage(getCurrentPageNo());
    this->doc->unlock();

    return p;
}

void Control::undoRedoChanged() {
    this->actionDB->enableAction(Action::UNDO, undoRedo->canUndo());
    this->actionDB->enableAction(Action::REDO, undoRedo->canRedo());

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
        xoj_assert(view != nullptr);
        PageRef page = view->getPage();
        auto sel = SelectionFactory::createFromElementOnActiveLayer(this, page, view, textobj);

        xournal->setSelection(sel.release());
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

void Control::setFontSelected(const XojFont& font) {
    this->actionDB->setActionState(Action::FONT, font.asString().c_str());
}

void Control::toolChanged() {
    ToolType type = toolHandler->getToolType();

    this->actionDB->setActionState(Action::SELECT_TOOL, type);

    this->actionDB->enableAction(Action::TOOL_DRAW_LINE, toolHandler->hasCapability(TOOL_CAP_RULER));
    this->actionDB->enableAction(Action::TOOL_DRAW_RECTANGLE, toolHandler->hasCapability(TOOL_CAP_RECTANGLE));
    this->actionDB->enableAction(Action::TOOL_DRAW_ELLIPSE, toolHandler->hasCapability(TOOL_CAP_ELLIPSE));
    this->actionDB->enableAction(Action::TOOL_DRAW_ARROW, toolHandler->hasCapability(TOOL_CAP_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_DOUBLE_ARROW, toolHandler->hasCapability(TOOL_CAP_DOUBLE_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_COORDINATE_SYSTEM, toolHandler->hasCapability(TOOL_CAP_ARROW));
    this->actionDB->enableAction(Action::TOOL_DRAW_SPLINE, toolHandler->hasCapability(TOOL_CAP_SPLINE));
    this->actionDB->enableAction(Action::TOOL_DRAW_SHAPE_RECOGNIZER, toolHandler->hasCapability(TOOL_CAP_RECOGNIZER));

    DrawingType dt = toolHandler->getDrawingType();
    this->actionDB->setActionState(Action::TOOL_DRAW_LINE, dt == DRAWING_TYPE_LINE);
    this->actionDB->setActionState(Action::TOOL_DRAW_RECTANGLE, dt == DRAWING_TYPE_RECTANGLE);
    this->actionDB->setActionState(Action::TOOL_DRAW_ELLIPSE, dt == DRAWING_TYPE_ELLIPSE);
    this->actionDB->setActionState(Action::TOOL_DRAW_ARROW, dt == DRAWING_TYPE_ARROW);
    this->actionDB->setActionState(Action::TOOL_DRAW_DOUBLE_ARROW, dt == DRAWING_TYPE_DOUBLE_ARROW);
    this->actionDB->setActionState(Action::TOOL_DRAW_COORDINATE_SYSTEM, dt == DRAWING_TYPE_COORDINATE_SYSTEM);
    this->actionDB->setActionState(Action::TOOL_DRAW_SPLINE, dt == DRAWING_TYPE_SPLINE);
    this->actionDB->setActionState(Action::TOOL_DRAW_SHAPE_RECOGNIZER, dt == DRAWING_TYPE_SHAPE_RECOGNIZER);

    bool enableSize = toolHandler->hasCapability(TOOL_CAP_SIZE);
    this->actionDB->enableAction(Action::TOOL_SIZE, enableSize);
    if (enableSize) {
        toolSizeChanged();
    }

    // Set or reset the lineStyle
    toolLineStyleChanged();

    bool enableFill = toolHandler->hasCapability(TOOL_CAP_FILL);
    this->actionDB->enableAction(Action::TOOL_FILL, enableFill);
    this->actionDB->enableAction(Action::TOOL_FILL_OPACITY, enableFill);
    if (enableFill) {
        toolFillChanged();
    }

    bool enableColor = toolHandler->hasCapability(TOOL_CAP_COLOR);
    this->actionDB->enableAction(Action::TOOL_COLOR, enableColor);
    this->actionDB->enableAction(Action::SELECT_COLOR, enableColor);
    if (enableColor) {
        toolColorChanged();
    }

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
    this->actionDB->setActionState(Action::TOOL_ERASER_SIZE, toolHandler->getEraserSize());
}

void Control::penSizeChanged() { this->actionDB->setActionState(Action::TOOL_PEN_SIZE, toolHandler->getPenSize()); }

void Control::highlighterSizeChanged() {
    this->actionDB->setActionState(Action::TOOL_HIGHLIGHTER_SIZE, toolHandler->getHighlighterSize());
}

void Control::toolSizeChanged() {
    if (toolHandler->getToolType() == TOOL_PEN) {
        penSizeChanged();
    } else if (toolHandler->getToolType() == TOOL_ERASER) {
        eraserSizeChanged();
    } else if (toolHandler->getToolType() == TOOL_HIGHLIGHTER) {
        highlighterSizeChanged();
    }

    this->actionDB->setActionState(Action::TOOL_SIZE, toolHandler->getSize());

    getCursor()->updateCursor();
}

void Control::toolFillChanged() {
    this->actionDB->setActionState(Action::TOOL_FILL, toolHandler->getFill() != -1);
    this->actionDB->setActionState(Action::TOOL_PEN_FILL, toolHandler->getPenFillEnabled());
    this->actionDB->setActionState(Action::TOOL_HIGHLIGHTER_FILL, toolHandler->getHighlighterFillEnabled());
}

void Control::toolLineStyleChanged() {
    std::optional<string> style = getLineStyleToSelect();
    this->actionDB->setActionState(Action::TOOL_PEN_LINE_STYLE, style ? style->c_str() : "none");
}

auto Control::getLineStyleToSelect() -> std::optional<string> const {
    std::optional<std::string> style;
    if (auto* tool = toolHandler->getActiveTool(); tool->getToolType() == TOOL_PEN) {
        style = StrokeStyle::formatStyle(tool->getLineStyle());
    }

    if (!win) {
        return style;
    }

    const EditSelection* sel = win->getXournal()->getSelection();
    if (!sel) {
        return style;
    }

    bool isFirstPenStrokeElement = true;
    std::optional<std::string> previous_style;

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
    this->actionDB->setActionState(Action::TOOL_COLOR, getToolHandler()->getColorMaskAlpha());
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

void Control::showSettings() {
    // take note of some settings before to compare with after
    struct {
        Color selectionColor;
        bool verticalSpace;
        int verticalSpaceAmountAbove;
        int verticalSpaceAmountBelow;
        bool horizontalSpace;
        int horizontalSpaceAmountRight;
        int horizontalSpaceAmountLeft;
        bool unlimitedScrolling;
        StylusCursorType stylusCursorType;
        bool highlightPosition;
        SidebarNumberingStyle sidebarStyle;
    } callbackData = {settings->getBorderColor(),
                      settings->getAddVerticalSpace(),
                      settings->getAddVerticalSpaceAmountAbove(),
                      settings->getAddVerticalSpaceAmountBelow(),
                      settings->getAddHorizontalSpace(),
                      settings->getAddHorizontalSpaceAmountRight(),
                      settings->getAddHorizontalSpaceAmountLeft(),
                      settings->getUnlimitedScrolling(),
                      settings->getStylusCursorType(),
                      settings->isHighlightPosition(),
                      settings->getSidebarNumberingStyle()};

    auto dlg = xoj::popup::PopupWindowWrapper<SettingsDialog>(
            this->gladeSearchPath, settings, this, [ctrl = this, callbackData]() {
                Settings* settings = ctrl->getSettings();
                MainWindow* win = ctrl->win;
                XournalView* xournal = win->getXournal();
                // note which settings have changed and act accordingly
                if (callbackData.selectionColor != settings->getBorderColor()) {
                    xournal->forceUpdatePagenumbers();
                }

                if (!settings->getUnlimitedScrolling() &&
                    (callbackData.verticalSpace != settings->getAddVerticalSpace() ||
                     callbackData.horizontalSpace != settings->getAddHorizontalSpace() ||
                     callbackData.verticalSpaceAmountAbove != settings->getAddVerticalSpaceAmountAbove() ||
                     callbackData.horizontalSpaceAmountRight != settings->getAddHorizontalSpaceAmountRight() ||
                     callbackData.verticalSpaceAmountBelow != settings->getAddVerticalSpaceAmountBelow() ||
                     callbackData.horizontalSpaceAmountLeft != settings->getAddHorizontalSpaceAmountLeft())) {
                    xournal->layoutPages();
                    double const xChange =
                            (settings->getAddHorizontalSpace() ? settings->getAddHorizontalSpaceAmountLeft() : 0) -
                            (callbackData.horizontalSpace ? callbackData.horizontalSpaceAmountLeft : 0);
                    const double yChange =
                            (settings->getAddVerticalSpace() ? settings->getAddVerticalSpaceAmountAbove() : 0) -
                            (callbackData.verticalSpace ? callbackData.verticalSpaceAmountAbove : 0);

                    win->getLayout()->scrollRelative(xChange, yChange);
                }

                if (callbackData.unlimitedScrolling != settings->getUnlimitedScrolling()) {
                    const int xUnlimited = static_cast<int>(win->getLayout()->getVisibleRect().width);
                    const int yUnlimited = static_cast<int>(win->getLayout()->getVisibleRect().height);
                    const double xChange =
                            callbackData.unlimitedScrolling ?
                                    -xUnlimited + (callbackData.horizontalSpace ?
                                                           callbackData.horizontalSpaceAmountLeft :
                                                           0) :
                                    xUnlimited -
                                            (callbackData.horizontalSpace ? callbackData.horizontalSpaceAmountLeft : 0);
                    const double yChange =
                            callbackData.unlimitedScrolling ?
                                    -yUnlimited +
                                            (callbackData.verticalSpace ? callbackData.verticalSpaceAmountAbove : 0) :
                                    yUnlimited -
                                            (callbackData.verticalSpace ? callbackData.verticalSpaceAmountAbove : 0);

                    xournal->layoutPages();
                    win->getLayout()->scrollRelative(xChange, yChange);
                }

                if (callbackData.stylusCursorType != settings->getStylusCursorType() ||
                    callbackData.highlightPosition != settings->isHighlightPosition()) {
                    ctrl->getCursor()->updateCursor();
                }

                ctrl->getSidebar()->saveSize();
                ctrl->win->updateScrollbarSidebarPosition();
                ctrl->updateWindowTitle();

                ctrl->enableAutosave(settings->isAutosaveEnabled());

                ctrl->zoom->setZoomStep(settings->getZoomStep() / 100.0);
                ctrl->zoom->setZoomStepScroll(settings->getZoomStepScroll() / 100.0);
                ctrl->zoom->setZoom100Value(settings->getDisplayDpi() / Util::DPI_NORMALIZATION_FACTOR);

                if (callbackData.sidebarStyle != settings->getSidebarNumberingStyle()) {
                    ctrl->getSidebar()->layout();
                }

                xournal->getHandRecognition()->reload();
                ctrl->win->updateColorscheme();
            });
    dlg.show(GTK_WINDOW(this->win->getWindow()));
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

    bool an = annotatePdf(filepath, false);
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

enum class MissingPdfDialogOptions : gint { USE_PROPOSED, SELECT_OTHER, REMOVE, CANCEL };

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

    // show the dialog
    std::vector<XojMsgBox::Button> buttons = {
            {_("Select another PDF"), static_cast<int>(MissingPdfDialogOptions::SELECT_OTHER)},
            {_("Remove PDF Background"), static_cast<int>(MissingPdfDialogOptions::REMOVE)},
            {_("Cancel"), static_cast<int>(MissingPdfDialogOptions::CANCEL)}};
    if (proposePdfFile) {
        buttons.insert(buttons.begin(),
                       {_("Use proposed PDF"), static_cast<int>(MissingPdfDialogOptions::USE_PROPOSED)});
    }
    XojMsgBox::askQuestion(
            this->getGtkWindow(), _("Missing PDF background file"), msg, buttons,
            std::bind(&Control::missingPdfDialogResponseHandler, this, proposedPdfFilepath, std::placeholders::_1));
}

void Control::missingPdfDialogResponseHandler(const fs::path& proposedPdfFilepath, int responseId) {
    switch (static_cast<MissingPdfDialogOptions>(responseId)) {
        case MissingPdfDialogOptions::USE_PROPOSED:
            if (!proposedPdfFilepath.empty()) {
                this->pageBackgroundChangeController->changePdfPagesBackground(proposedPdfFilepath, false);
            }
            break;
        case MissingPdfDialogOptions::SELECT_OTHER: {
            bool attachToDocument = false;
            XojOpenDlg dlg(this->getGtkWindow(), this->settings);
            auto pdfFilename = dlg.showOpenDialog(true, attachToDocument);
            if (!pdfFilename.empty()) {
                this->pageBackgroundChangeController->changePdfPagesBackground(pdfFilename, attachToDocument);
            }
        } break;
        case MissingPdfDialogOptions::REMOVE:
            this->pageBackgroundChangeController->applyBackgroundToAllPages(PageType(PageTypeFormat::Plain));
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
    data->ctrl->scrollHandler->scrollToPage(as_unsigned(data->md.page));
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

auto Control::annotatePdf(fs::path filepath, bool attachToDocument) -> bool {
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
    win->setDynamicallyGeneratedSubmenuDisabled(true);
    actionDB->disableAll();
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

    this->win->setDynamicallyGeneratedSubmenuDisabled(false);
    actionDB->resetEnableStatus();
    getCursor()->setCursorBusy(false);
    disableSidebarTmp(false);

    gtk_widget_hide(this->statusbar);

    this->isBlocking = false;
}

void Control::setMaximumState(size_t max) { this->maxState = max; }

void Control::setCurrentState(size_t state) {
    Util::execInUiThread([=]() {
        gtk_progress_bar_set_fraction(this->pgState,
                                      static_cast<gdouble>(state) / static_cast<gdouble>(this->maxState));
    });
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

    GtkFileFilter* filterXoj = gtk_file_filter_new();
    gtk_file_filter_set_name(filterXoj, _("Xournal++ files"));
    gtk_file_filter_add_mime_type(filterXoj, "application/x-xopp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);

    this->doc->lock();
    auto suggested_folder = this->doc->createSaveFolder(this->settings->getLastSavePath());
    auto suggested_name = this->doc->createSaveFilename(Document::XOPP, this->settings->getDefaultSaveName());
    this->doc->unlock();

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(suggested_folder), nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), Util::toGFilename(suggested_name).c_str());
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(this->settings->getLastOpenPath()),
                                         nullptr);

    while (true) {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return false;
        }

        auto fileTmp = Util::fromGFile(
                xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)), xoj::util::adopt)
                        .get());
        Util::clearExtensions(fileTmp);
        fileTmp += ".xopp";
        // Since we add the extension after the OK button, we have to check manually on existing files
        if (askToReplace(fileTmp)) {
            break;
        }
    }

    auto file = Util::fromGFile(
            xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)), xoj::util::adopt).get());
    settings->setLastSavePath(file.parent_path());
    gtk_widget_destroy(dialog);

    this->doc->lock();
    this->doc->setFilepath(file);
    this->doc->unlock();

    return true;
}

void Control::showFontDialog() {
    this->actionDB->enableAction(Action::SELECT_FONT, false);  // Only one dialog
    auto* dlg = gtk_font_chooser_dialog_new(_("Select font"), GTK_WINDOW(this->win->getWindow()));
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dlg), settings->getFont().asString().c_str());

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(GTK_DIALOG(dlg), [this, dlg](int response) {
        if (response == GTK_RESPONSE_OK) {
            auto font = xoj::util::OwnedCString::assumeOwnership(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg)));
            this->actionDB->fireChangeActionState(Action::FONT, font.get());
        }
        this->actionDB->enableAction(Action::SELECT_FONT, true);
    });
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::showColorChooserDialog() {
    this->actionDB->enableAction(Action::SELECT_COLOR, false);  // Only one dialog
    auto* dlg = gtk_color_chooser_dialog_new(_("Select color"), GTK_WINDOW(this->win->getWindow()));
    GdkRGBA c = Util::argb_to_GdkRGBA(toolHandler->getColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &c);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dlg), false);

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(GTK_DIALOG(dlg), [this, dlg](int response) {
        if (response == GTK_RESPONSE_OK) {
            GdkRGBA c;
            gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &c);
            this->actionDB->fireChangeActionState(Action::TOOL_COLOR, Util::GdkRGBA_to_rgb(c));
        }
        this->actionDB->enableAction(Action::SELECT_COLOR, true);
    });
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::updateWindowTitle() {
    string title{};

    this->doc->lock();
    if (doc->getFilepath().empty()) {
        if (doc->getPdfFilepath().empty()) {
            title = _("Unsaved Document");
        } else {
            if (settings->isPageNumberInTitlebarShown()) {
                title += ("[" + std::to_string(getCurrentPageNo() + 1) + "/" + std::to_string(doc->getPageCount()) +
                          "]  ");
            }
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
        if (settings->isPageNumberInTitlebarShown()) {
            title += ("[" + std::to_string(getCurrentPageNo() + 1) + "/" + std::to_string(doc->getPageCount()) + "]  ");
        }
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

    auto* job = new PdfExportJob(this);
    job->showFileChooser(
            [ctrl = this, job]() {
                ctrl->scheduler->addJob(job, JOB_PRIORITY_NONE);
                job->unref();
            },
            [ctrl = this, job]() {
                // The job blocked, so we have to unblock, because the job unblocks only after
                ctrl->unblock();
                job->unref();
            });
}

void Control::exportAs() {
    this->clearSelectionEndText();
    auto* job = new CustomExportJob(this);
    job->showDialogAndRun();
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

void Control::initButtonTool() {
    std::vector<Button> buttons{Button::BUTTON_ERASER,       Button::BUTTON_STYLUS_ONE,  Button::BUTTON_STYLUS_TWO,
                                Button::BUTTON_MOUSE_MIDDLE, Button::BUTTON_MOUSE_RIGHT, Button::BUTTON_TOUCH};
    ButtonConfig* cfg;
    for (auto b: buttons) {
        cfg = settings->getButtonConfig(b);
        cfg->initButton(this->toolHandler, b);
    }
}

auto Control::askToReplace(fs::path const& filepath) const -> bool {
    if (fs::exists(filepath)) {
        std::string msg = FS(FORMAT_STR("The file {1} already exists! Do you want to replace it?") %
                             filepath.filename().u8string());
        int res = XojMsgBox::replaceFileQuestion(getGtkWindow(), msg);
        return res == GTK_RESPONSE_OK;
    }
    return true;
}

void Control::showAbout() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::AboutDialog>(this->gladeSearchPath);
    popup.show(GTK_WINDOW(this->win->getWindow()));
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
    this->actionDB->enableAction(Action::CUT, enabled);
    this->actionDB->enableAction(Action::COPY, enabled);
}

void Control::clipboardPasteEnabled(bool enabled) { this->actionDB->enableAction(Action::PASTE, enabled); }

void Control::clipboardPasteText(string text) {
    auto t = std::make_unique<Text>();
    t->setText(text);
    t->setFont(settings->getFont());
    t->setColor(toolHandler->getTool(TOOL_TEXT).getColor());

    clipboardPaste(std::move(t));
}

void Control::clipboardPasteImage(GdkPixbuf* img) {
    auto image = std::make_unique<Image>();
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

    clipboardPaste(std::move(image));
}

void Control::clipboardPaste(ElementPtr e) {
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
    this->doc->unlock();

    win->getXournal()->getPasteTarget(x, y);

    double width = e->getElementWidth();
    double height = e->getElementHeight();

    x = std::max(0.0, x - width / 2);
    y = std::max(0.0, y - height / 2);

    e->setX(x);
    e->setY(y);

    undoRedo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, e.get()));
    auto sel = SelectionFactory::createFromFloatingElement(this, page, layer, view, std::move(e));

    win->getXournal()->setSelection(sel.release());
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

    auto selection = std::make_unique<EditSelection>(this, page, page->getSelectedLayer(), view);
    this->doc->unlock();

    try {
        ElementPtr element;
        std::string version = in.readString();
        if (version != PROJECT_STRING) {
            g_warning("Paste from Xournal Version %s to Xournal Version %s", version.c_str(), PROJECT_STRING);
        }

        selection->readSerialized(in);

        // document lock not needed anymore, because we don't change the document, we only change the selection

        int count = in.readInt();
        auto pasteAddUndoAction = std::make_unique<AddUndoAction>(page, false);
        // this will undo a group of elements that are inserted

        for (int i = 0; i < count; i++) {
            std::string name = in.getNextObjectName();
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
            selection->addElement(std::move(element), std::numeric_limits<Element::Index>::max());
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

        win->getXournal()->setSelection(selection.release());
    } catch (const std::exception& e) {
        g_warning("could not paste, Exception occurred: %s", e.what());
        Stacktrace::printStracktrace();
        if (selection) {
            for (Element* el: selection->getElements()) {
                delete el;
            }
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
    toolHandler->setFillEnabled(fill);
}

void Control::setLineStyle(const string& style) {
    LineStyle stl = StrokeStyle::parseStyle(style);

    if (this->win && this->win->getXournal()->getSelection()) {
        undoRedo->addUndoAction(this->win->getXournal()->getSelection()->setLineStyle(stl));
    } else if (this->toolHandler->getActiveTool()->getToolType() != TOOL_PEN) {
        this->selectTool(TOOL_PEN);
    }
    this->toolHandler->setLineStyle(stl);
}

void Control::setEraserType(EraserType type) {
    if (type != ERASER_TYPE_NONE) {
        if (this->toolHandler->getActiveTool()->getToolType() != TOOL_ERASER) {
            this->selectTool(TOOL_ERASER);
        }
    }
    this->toolHandler->setEraserType(type);
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

void Control::fontChanged(const XojFont& font) {
    settings->setFont(font);

    if (this->win) {
        if (EditSelection* sel = this->win->getXournal()->getSelection(); sel) {
            undoRedo->addUndoAction(UndoActionPtr(sel->setFont(font)));
        }
    }

    if (TextEditor* editor = getTextEditor(); editor) {
        editor->setFont(font);
    }
}

/**
 * The core handler for inserting latex
 */
void Control::runLatex() {
    /*
     * LatexController::run() will open a non-blocking dialog.
     */
    LatexController::run(this);
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

void Control::setRotationSnapping(bool enable) {
    settings->setSnapRotation(enable);
    this->actionDB->setActionState(Action::ROTATION_SNAPPING, enable);
}

void Control::setGridSnapping(bool enable) {
    settings->setSnapGrid(enable);
    this->actionDB->setActionState(Action::GRID_SNAPPING, enable);
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

auto Control::getPageBackgroundChangeController() const -> PageBackgroundChangeController* {
    return this->pageBackgroundChangeController.get();
}

auto Control::getLayerController() const -> LayerController* { return this->layerController; }

auto Control::getPluginController() const -> PluginController* { return this->pluginController; }
