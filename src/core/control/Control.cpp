#include "Control.h"

#include <algorithm>   // for max
#include <cstdlib>     // for size_t
#include <exception>   // for exce...
#include <functional>  // for bind
#include <iterator>    // for end
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
#include "gui/dialog/XojOpenDlg.h"                               // for XojO...
#include "gui/dialog/XojSaveDlg.h"                               // for XojS...
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
#include "settings/RecolorParameters.h"                          // for RecolorParameters
#include "undo/AddUndoAction.h"                                  // for AddU...
#include "undo/InsertDeletePageUndoAction.h"                     // for Inse...
#include "undo/InsertUndoAction.h"                               // for Inse...
#include "undo/MoveSelectionToLayerUndoAction.h"                 // for Move...
#include "undo/PageSizeChangeUndoAction.h"                       // for PageSizeChangeUndoAction
#include "undo/SwapUndoAction.h"                                 // for SwapUndoAction
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
    this->loadPaletteFromSettings();

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

struct Control::MissingPdfData {
    bool wasPdfAttached;
    std::string missingFileName;
};

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

    int width = 0;
    int height = 0;
    gtk_window_get_default_size(getGtkWindow(), &width, &height);

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
    Range range = view->getVisiblePart();
    if (range.isValid()) {
        double originX = (range.minX + range.maxX) * .5;
        double originY = (range.minY + range.maxY) * .5;
        geometryToolController->translate(originX, originY);
    } else {
        geometryToolController->translate(view->getWidth() * .5, view->getHeight() * .5);
    }
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
            Stacktrace::printStacktrace();
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
                        Stacktrace::printStacktrace();
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

    firePageSelected(pageId);
    return pageId;
}

void Control::firePageSelected(size_t page) { DocumentHandler::firePageSelected(page); }

void Control::manageToolbars() {
    xoj::popup::PopupWindowWrapper<ToolbarManageDialog> dlg(
            this->gladeSearchPath, this->win->getToolbarModel(), [win = this->win]() {
                if (const auto& tbs = win->getToolbarModel()->getToolbars();
                    std::none_of(tbs.begin(), tbs.end(),
                                 [tb = win->getSelectedToolbar()](const auto& t) { return t.get() == tb; })) {
                    // The active toolbar has been deleted!
                    xoj_assert(!tbs.empty());
                    win->toolbarSelected(tbs.front().get());
                    XojMsgBox::showErrorToUser(
                            GTK_WINDOW(win->getWindow()),
                            _("You deleted the active toolbar. Falling back to the default toolbar."));
                }

                win->updateToolbarMenu();
                auto filepath = Util::getConfigFile(TOOLBAR_CONFIG);
                win->getToolbarModel()->save(filepath);
            });
    dlg.show(GTK_WINDOW(this->win->getWindow()));

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
                                       auto data = std::make_unique<ToolbarData>(*ctrl->win->getSelectedToolbar());
                                       ToolbarModel* model = ctrl->win->getToolbarModel();
                                       model->initCopyNameId(data.get());
                                       ctrl->win->toolbarSelected(model->add(std::move(data)));
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
}

void Control::disableSidebarTmp(bool disabled) { this->sidebar->setTmpDisabled(disabled); }

void Control::addDefaultPage(const std::optional<std::string>& pageTemplate, Document* doc) {
    const std::string& templ = pageTemplate.value_or(settings->getPageTemplate());
    PageTemplateSettings model;
    model.parse(templ);

    auto page = std::make_shared<XojPage>(model.getPageWidth(), model.getPageHeight());
    page->setBackgroundColor(model.getBackgroundColor());
    page->setBackgroundType(model.getBackgroundType());

    if (doc == nullptr || doc == this->doc) {
        // Apply to the curent document
        this->doc->lock();
        this->doc->addPage(std::move(page));
        this->doc->unlock();
    } else {
        this->doc->lock();
        doc->addPage(std::move(page));
        this->doc->unlock();
    }
}

void Control::updatePageActions() {
    auto currentPage = getCurrentPageNo();
    auto nbPages = this->doc->getPageCount();
    this->actionDB->enableAction(Action::DELETE_PAGE, nbPages > 1);
    this->actionDB->enableAction(Action::MOVE_PAGE_TOWARDS_BEGINNING, currentPage != 0);
    this->actionDB->enableAction(Action::MOVE_PAGE_TOWARDS_END, currentPage < nbPages - 1);
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

void Control::movePageTowardsBeginning() {
    auto currentPageNo = this->getCurrentPageNo();
    if (currentPageNo < 1) {
        g_warning("Control::movePageTowardsBeginning() called on the first page");
        return;
    }
    if (currentPageNo == npos) {
        g_warning("Control::movePageTowardsBeginning() called with current page selected");
        return;
    }

    auto lock = std::unique_lock(*this->doc);
    PageRef page = this->doc->getPage(currentPageNo);
    PageRef otherPage = doc->getPage(currentPageNo - 1);
    if (!page || !otherPage) {
        g_warning("Control::movePageTowardsBeginning() called but no page %zu or %zu", currentPageNo,
                  currentPageNo - 1);
        return;
    }

    doc->deletePage(currentPageNo);
    doc->insertPage(page, currentPageNo - 1);
    lock.unlock();

    UndoRedoHandler* undo = this->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<SwapUndoAction>(currentPageNo - 1, true, page, otherPage));

    this->firePageDeleted(currentPageNo);
    this->firePageInserted(currentPageNo - 1);
    this->firePageSelected(currentPageNo - 1);

    this->getScrollHandler()->scrollToPage(currentPageNo - 1);
}


void Control::movePageTowardsEnd() {
    auto currentPageNo = this->getCurrentPageNo();
    if (currentPageNo == npos) {
        g_warning("Control::movePageTowardsEnd() called with current page selected");
        return;
    }

    auto lock = std::unique_lock(*this->doc);
    auto nbPage = this->doc->getPageCount();
    if (currentPageNo >= nbPage - 1) {
        g_warning("Control::movePageTowardsEnd() called on last page");
        return;
    }

    PageRef page = this->doc->getPage(currentPageNo);
    PageRef otherPage = doc->getPage(currentPageNo + 1);
    if (!page || !otherPage) {
        g_warning("Control::movePageTowardsEnd() called but no page %zu or %zu", currentPageNo, currentPageNo + 1);
        return;
    }

    doc->deletePage(currentPageNo);
    doc->insertPage(page, currentPageNo + 1);
    lock.unlock();

    this->undoRedo->addUndoAction(std::make_unique<SwapUndoAction>(currentPageNo, false, page, otherPage));

    this->firePageDeleted(currentPageNo);
    this->firePageInserted(currentPageNo + 1);
    this->firePageSelected(currentPageNo + 1);

    this->getScrollHandler()->scrollToPage(currentPageNo + 1);
}

/// Remove mnemonic indicators in menu labels
static std::string removeMnemonics(std::string orig) {
    std::regex reg("_(.)");
    return std::regex_replace(orig, reg, "$1");
}

void Control::askInsertPdfPage(size_t pdfPage) {
    using Responses = enum { CANCEL = 1, AFTER = 2, END = 3 };
    std::vector<XojMsgBox::Button> buttons = {{_("Cancel"), Responses::CANCEL},
                                              {_("Insert after current page"), Responses::AFTER},
                                              {_("Insert at end"), Responses::END}};

    // Must match the labels in main.glade and PageTypeHandler.cpp
    std::string pathToMenuEntry = removeMnemonics(_("_Journal") + std::string(" → ") + _("Paper B_ackground") +
                                                  std::string(" → ") + _("With PDF background"));

    XojMsgBox::askQuestion(this->getGtkWindow(),
                           FC(_F("Your current document does not contain PDF Page no {1}\n"
                                 "Would you like to insert this page?\n\n"
                                 "Tip: You can select {2} to insert a PDF page.") %
                              static_cast<int64_t>(pdfPage + 1) % pathToMenuEntry),
                           "", buttons, [ctrl = this, pdfPage](int response) {
                               if (response == Responses::AFTER || response == Responses::END) {
                                   Document* doc = ctrl->getDocument();

                                   doc->lock();
                                   size_t position = response == Responses::AFTER ? ctrl->getCurrentPageNo() + 1 :
                                                                                    doc->getPageCount();
                                   XojPdfPageSPtr pdf = doc->getPdfPage(pdfPage);
                                   doc->unlock();

                                   if (pdf) {
                                       auto page = std::make_shared<XojPage>(pdf->getWidth(), pdf->getHeight());
                                       page->setBackgroundPdfPageNr(pdfPage);
                                       ctrl->insertPage(page, position);
                                   }
                               }
                           });
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

    updatePageActions();
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
                double oldW = page->getWidth();
                double oldH = page->getHeight();
                Document::setPageSize(page, width, height);
                size_t pageNo = ctrl->doc->indexOf(page);
                size_t pageCount = ctrl->doc->getPageCount();
                ctrl->doc->unlock();
                ctrl->undoRedo->addUndoAction(std::make_unique<PageSizeChangeUndoAction>(page, oldW, oldH));
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
    this->actionDB->enableAction(Action::ZOOM, !enabled);

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
        std::optional<std::filesystem::path> colorPaletteSetting;
        RecolorParameters recolorParameters;
    } settingsBeforeDialog = {
            settings->getBorderColor(),
            settings->getAddVerticalSpace(),
            settings->getAddVerticalSpaceAmountAbove(),
            settings->getAddVerticalSpaceAmountBelow(),
            settings->getAddHorizontalSpace(),
            settings->getAddHorizontalSpaceAmountRight(),
            settings->getAddHorizontalSpaceAmountLeft(),
            settings->getUnlimitedScrolling(),
            settings->getStylusCursorType(),
            settings->isHighlightPosition(),
            settings->getSidebarNumberingStyle(),
            settings->getColorPaletteSetting(),
            settings->getRecolorParameters(),
    };

    auto dlg = xoj::popup::PopupWindowWrapper<SettingsDialog>(
            this->gladeSearchPath, settings, this,
            std::vector<fs::path>{Util::getBuiltInPaletteDirectoryPath(), Util::getCustomPaletteDirectoryPath()},
            [ctrl = this, settingsBeforeDialog]() {
                Settings* settings = ctrl->getSettings();
                MainWindow* win = ctrl->win;
                XournalView* xournal = win->getXournal();
                // note which settings have changed and act accordingly
                if (settingsBeforeDialog.selectionColor != settings->getBorderColor()) {
                    xournal->forceUpdatePagenumbers();
                }

                if (!settings->getUnlimitedScrolling() &&
                    (settingsBeforeDialog.verticalSpace != settings->getAddVerticalSpace() ||
                     settingsBeforeDialog.horizontalSpace != settings->getAddHorizontalSpace() ||
                     settingsBeforeDialog.verticalSpaceAmountAbove != settings->getAddVerticalSpaceAmountAbove() ||
                     settingsBeforeDialog.horizontalSpaceAmountRight != settings->getAddHorizontalSpaceAmountRight() ||
                     settingsBeforeDialog.verticalSpaceAmountBelow != settings->getAddVerticalSpaceAmountBelow() ||
                     settingsBeforeDialog.horizontalSpaceAmountLeft != settings->getAddHorizontalSpaceAmountLeft())) {
                    xournal->layoutPages();
                    double const xChange =
                            (settings->getAddHorizontalSpace() ? settings->getAddHorizontalSpaceAmountLeft() : 0) -
                            (settingsBeforeDialog.horizontalSpace ? settingsBeforeDialog.horizontalSpaceAmountLeft : 0);
                    const double yChange =
                            (settings->getAddVerticalSpace() ? settings->getAddVerticalSpaceAmountAbove() : 0) -
                            (settingsBeforeDialog.verticalSpace ? settingsBeforeDialog.verticalSpaceAmountAbove : 0);

                    win->getLayout()->scrollRelative(xChange, yChange);
                }

                if (settingsBeforeDialog.unlimitedScrolling != settings->getUnlimitedScrolling()) {
                    const int xUnlimited = static_cast<int>(win->getLayout()->getVisibleRect().width);
                    const int yUnlimited = static_cast<int>(win->getLayout()->getVisibleRect().height);
                    const double xChange =
                            settingsBeforeDialog.unlimitedScrolling ?
                                    -xUnlimited + (settingsBeforeDialog.horizontalSpace ?
                                                           settingsBeforeDialog.horizontalSpaceAmountLeft :
                                                           0) :
                                    xUnlimited - (settingsBeforeDialog.horizontalSpace ?
                                                          settingsBeforeDialog.horizontalSpaceAmountLeft :
                                                          0);
                    const double yChange =
                            settingsBeforeDialog.unlimitedScrolling ?
                                    -yUnlimited + (settingsBeforeDialog.verticalSpace ?
                                                           settingsBeforeDialog.verticalSpaceAmountAbove :
                                                           0) :
                                    yUnlimited - (settingsBeforeDialog.verticalSpace ?
                                                          settingsBeforeDialog.verticalSpaceAmountAbove :
                                                          0);

                    xournal->layoutPages();
                    win->getLayout()->scrollRelative(xChange, yChange);
                }

                if (settingsBeforeDialog.stylusCursorType != settings->getStylusCursorType() ||
                    settingsBeforeDialog.highlightPosition != settings->isHighlightPosition()) {
                    ctrl->getCursor()->updateCursor();
                }


                bool reloadToolbars = false;
                if (settingsBeforeDialog.colorPaletteSetting.has_value() &&
                    settingsBeforeDialog.colorPaletteSetting.value() != settings->getColorPaletteSetting()) {
                    ctrl->loadPaletteFromSettings();
                    ctrl->getWindow()->getToolMenuHandler()->updateColorToolItems(ctrl->getPalette());
                    reloadToolbars = true;
                }

                if (settingsBeforeDialog.recolorParameters != settings->getRecolorParameters()) {
                    ctrl->getWindow()->getToolMenuHandler()->updateColorToolItemsRecoloring(
                            settings->getRecolorParameters().recolorizeMainView ?
                                    std::make_optional(settings->getRecolorParameters().recolor) :
                                    std::nullopt);
                    reloadToolbars = true;
                }

                if (reloadToolbars) {
                    ctrl->getWindow()->reloadToolbars();
                }

                ctrl->getSidebar()->saveSize();
                ctrl->win->updateScrollbarSidebarPosition();
                ctrl->updateWindowTitle();

                ctrl->enableAutosave(settings->isAutosaveEnabled());

                ctrl->zoom->setZoomStep(settings->getZoomStep() / 100.0);
                ctrl->zoom->setZoomStepScroll(settings->getZoomStepScroll() / 100.0);
                ctrl->zoom->setZoom100Value(settings->getDisplayDpi() / Util::DPI_NORMALIZATION_FACTOR);

                if (settingsBeforeDialog.sidebarStyle != settings->getSidebarNumberingStyle()) {
                    ctrl->getSidebar()->updatePageNumberingStyle();
                }

                xournal->getHandRecognition()->reload();
                ctrl->win->updateColorscheme();
            });
    dlg.show(GTK_WINDOW(this->win->getWindow()));
}

static std::unique_ptr<Document> createNewDocument(Control* ctrl, fs::path filepath,
                                                   const std::optional<std::string>& pageTemplate) {
    auto newDoc = std::make_unique<Document>(ctrl);
    if (!filepath.empty()) {
        newDoc->setFilepath(std::move(filepath));
    }
    ctrl->addDefaultPage(pageTemplate, newDoc.get());
    return newDoc;
}

void Control::newFile(fs::path filepath) {
    this->close(
            [ctrl = this, filepath = std::move(filepath)](bool closed) {
                if (closed) {
                    ctrl->replaceDocument(createNewDocument(ctrl, std::move(filepath), std::nullopt), -1);
                }
            },
            true);
}

/**
 * Check if this is an autosave file, return false in this case and display a user instruction
 */
static auto shouldFileOpen(fs::path const& filepath, GtkWindow* win) -> bool {
    auto basePath = Util::getCacheSubfolder("");
    auto isChild = Util::isChildOrEquivalent(filepath, basePath);
    if (isChild) {
        string msg = FS(_F("Do not open Autosave files. They may will be overwritten!\n"
                           "Copy the files to another folder.\n"
                           "Files from Folder {1} cannot be opened.") %
                        basePath.u8string());
        XojMsgBox::showErrorToUser(win, msg);
    }
    return !isChild;
}

void Control::askToOpenFile() {
    /**
     * Question: in case the current file has not been saved yet, do we want:
     *      1. First ask to save it, save it or discard it and then show the FileChooserDialog to open a new file
     *      2. First show the FileChooserDialog, and if a valid file has been selected and successfully opened, ask to
     *         save or discard the current file
     * For now, this implements option 1.
     */
    this->close([ctrl = this](bool closed) {
        if (closed) {
            xoj::OpenDlg::showOpenFileDialog(ctrl->getGtkWindow(), ctrl->settings, [ctrl](fs::path path) {
                g_message("%s", (_F("file: {1}") % path.string()).c_str());
                ctrl->openFileWithoutSavingTheCurrentDocument(std::move(path), false, -1, [](bool) {});
            });
        }
    });
}

void Control::replaceDocument(std::unique_ptr<Document> doc, int scrollToPage) {
    this->closeDocument();

    fs::path filepath = doc->getFilepath();

    this->doc->lock();
    *this->doc = *doc;
    this->doc->unlock();

    // Set folder as last save path, so the next save will be at the current document location
    // This is important because of the new .xopp format, where Xournal .xoj handled as import,
    // not as file to load
    if (!filepath.empty()) {
        settings->setLastSavePath(filepath.parent_path());
    }

    fireDocumentChanged(DOCUMENT_CHANGE_COMPLETE);
    fileLoaded(scrollToPage);
}

void Control::openXoppFile(fs::path filepath, int scrollToPage, std::function<void(bool)> callback) {
    LoadHandler loadHandler;
    std::unique_ptr<Document> doc(loadHandler.loadDocument(filepath));

    if (!doc) {
        string msg = FS(_F("Error opening file \"{1}\"") % filepath.u8string()) + "\n" + loadHandler.getLastError();
        XojMsgBox::showErrorToUser(this->getGtkWindow(), msg);
        callback(false);
        return;
    }

    std::optional<MissingPdfData> missingPdf;
    if (!loadHandler.getMissingPdfFilename().empty() || loadHandler.isAttachedPdfMissing()) {
        missingPdf = {loadHandler.isAttachedPdfMissing(), loadHandler.getMissingPdfFilename()};
    }

    auto afterOpen = [ctrl = this, missingPdf = std::move(missingPdf), doc = std::move(doc), filepath,
                      scrollToPage]() mutable {
        ctrl->replaceDocument(std::move(doc), scrollToPage);

        if (missingPdf && (missingPdf->wasPdfAttached || !missingPdf->missingFileName.empty())) {
            // give the user a second chance to select a new PDF filepath, or to discard the PDF
            ctrl->promptMissingPdf(missingPdf.value(), filepath);
        }
    };

    if (loadHandler.getFileVersion() > FILE_FORMAT_VERSION) {
        enum { YES = 1, NO };
        std::vector<XojMsgBox::Button> buttons = {{_("Yes"), YES}, {_("No"), NO}};
        XojMsgBox::askQuestion(
                this->getGtkWindow(), _("File version mismatch"),
                _("The file being loaded has a file format version newer than the one currently supported by this "
                  "version of Xournal++, so it may not load properly. Open anyways?"),
                buttons, [afterOpen = std::move(afterOpen), callback = std::move(callback)](int response) mutable {
                    if (response == YES) {
                        afterOpen();
                        callback(true);
                    } else {
                        callback(false);
                    }
                });
    } else {
        afterOpen();
        callback(true);
    }
}

bool Control::openPdfFile(fs::path filepath, bool attachToDocument, int scrollToPage) {
    this->getCursor()->setCursorBusy(true);
    auto doc = std::make_unique<Document>(this);
    bool success = doc->readPdf(filepath, /*initPages=*/true, attachToDocument);
    if (success) {
        this->replaceDocument(std::move(doc), scrollToPage);
    } else {
        std::string msg = FS(_F("Error reading PDF file \"{1}\"\n{2}") % filepath.u8string() % doc->getLastErrorMsg());
        XojMsgBox::showErrorToUser(this->getGtkWindow(), msg);
    }
    this->getCursor()->setCursorBusy(false);
    return success;
}

bool Control::openXoptFile(fs::path filepath) {
    auto pageTemplate = Util::readString(filepath);
    if (!pageTemplate) {
        // Unable to read the template from the file
        return false;
    }
    this->replaceDocument(createNewDocument(this, std::move(filepath), pageTemplate), -1);
    return true;
}

void Control::openFileWithoutSavingTheCurrentDocument(fs::path filepath, bool attachToDocument, int scrollToPage,
                                                      std::function<void(bool)> callback) {
    if (filepath.empty() || !fs::exists(filepath)) {
        this->replaceDocument(createNewDocument(this, std::move(filepath), std::nullopt), -1);
        callback(true);
        return;
    }

    if (filepath.extension() == ".xopt") {
        this->openXoptFile(std::move(filepath));
        callback(true);
        return;
    }

    if (Util::hasPdfFileExt(filepath)) {
        if (!attachToDocument && this->settings->isAutoloadPdfXoj()) {
            const std::vector<std::string> exts = {".xopp", ".xoj", ".pdf.xopp", ".pdf.xoj"};
            fs::path root = filepath;
            Util::clearExtensions(root, ".pdf");
            for (const std::string& ext: exts) {
                fs::path f = root;
                f += ext;
                if (fs::exists(f)) {
                    this->openXoppFile(std::move(f), scrollToPage, std::move(callback));
                    return;
                }
            }
        }
        callback(this->openPdfFile(std::move(filepath), attachToDocument, scrollToPage));
        return;
    }

    this->openXoppFile(std::move(filepath), scrollToPage, std::move(callback));
}

void Control::openFile(fs::path filepath, std::function<void(bool)> callback, int scrollToPage, bool forceOpen) {
    if (filepath.empty() || (!forceOpen && !shouldFileOpen(filepath, getGtkWindow()))) {
        return;
    }

    this->close([ctrl = this, filepath = std::move(filepath), cb = std::move(callback),
                 scrollToPage](bool closed) mutable {
        if (closed) {
            ctrl->openFileWithoutSavingTheCurrentDocument(std::move(filepath), false, scrollToPage, std::move(cb));
        }
    });
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
    updatePageActions();
}

enum class MissingPdfDialogOptions : gint { USE_PROPOSED, SELECT_OTHER, REMOVE, CANCEL };

void Control::promptMissingPdf(Control::MissingPdfData& missingPdf, const fs::path& filepath) {
    const fs::path missingFilePath = fs::path(missingPdf.missingFileName);

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
    if (missingPdf.wasPdfAttached) {
        msg = FS(_F("The attached background file could not be found. It might have been moved, "
                    "renamed or deleted."));
    } else {
        msg = FS(_F("The background file \"{1}\" could not be found. It might have been moved, renamed or "
                    "deleted.\nIt was last seen at: \"{2}\"") %
                 filename % parentFolderPath);
    }

    // try to find file in current directory
    auto proposedPdfFilepath = filepath.parent_path() / filename;
    bool proposePdfFile = !missingPdf.wasPdfAttached && !filename.empty() && fs::exists(proposedPdfFilepath) &&
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

void Control::missingPdfDialogResponseHandler(fs::path proposedPdfFilepath, int responseId) {
    switch (static_cast<MissingPdfDialogOptions>(responseId)) {
        case MissingPdfDialogOptions::USE_PROPOSED:
            if (!proposedPdfFilepath.empty()) {
                this->pageBackgroundChangeController->changePdfPagesBackground(proposedPdfFilepath, false);
            }
            break;
        case MissingPdfDialogOptions::SELECT_OTHER:
            Util::execInUiThread([this]() {
                xoj::OpenDlg::showAnnotatePdfDialog(getGtkWindow(), settings, [this](fs::path path, bool attachPdf) {
                    if (!path.empty()) {
                        this->pageBackgroundChangeController->changePdfPagesBackground(path, attachPdf);
                    }
                });
            });
            break;
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

void Control::askToAnnotatePdf() {
    this->close([ctrl = this](bool closed) {
        if (closed) {
            xoj::OpenDlg::showAnnotatePdfDialog(ctrl->getGtkWindow(), ctrl->settings,
                                                [ctrl](fs::path path, bool attachPdf) {
                                                    if (!path.empty()) {
                                                        ctrl->openPdfFile(std::move(path), attachPdf, -1);
                                                    }
                                                });
        }
    });
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

void Control::showFontDialog() {
    this->actionDB->enableAction(Action::SELECT_FONT, false);  // Only one dialog
    auto* dlg = gtk_font_chooser_dialog_new(_("Select font"), GTK_WINDOW(this->win->getWindow()));
    gtk_font_chooser_set_font(GTK_FONT_CHOOSER(dlg), settings->getFont().asString().c_str());

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(
            GTK_DIALOG(dlg),
            [this, dlg](int response) {
                if (response == GTK_RESPONSE_OK) {
                    auto font =
                            xoj::util::OwnedCString::assumeOwnership(gtk_font_chooser_get_font(GTK_FONT_CHOOSER(dlg)));
                    this->actionDB->fireChangeActionState(Action::FONT, font.get());
                }
                this->actionDB->enableAction(Action::SELECT_FONT, true);
            },
            XojMsgBox::IMMEDIATE);  // We need IMMEDIATE so accessing GTK_FONT_CHOOSER(dlg) is not UB
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

void Control::showColorChooserDialog() {
    this->actionDB->enableAction(Action::SELECT_COLOR, false);  // Only one dialog
    auto* dlg = gtk_color_chooser_dialog_new(_("Select color"), GTK_WINDOW(this->win->getWindow()));
    GdkRGBA c = Util::argb_to_GdkRGBA(toolHandler->getColor());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dlg), &c);
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dlg), false);

    auto popup = xoj::popup::PopupWindowWrapper<XojMsgBox>(
            GTK_DIALOG(dlg),
            [this, dlg](int response) {
                if (response == GTK_RESPONSE_OK) {
                    GdkRGBA c;
                    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dlg), &c);
                    this->actionDB->fireChangeActionState(Action::TOOL_COLOR, Util::GdkRGBA_to_rgb(c));
                }
                this->actionDB->enableAction(Action::SELECT_COLOR, true);
            },
            XojMsgBox::IMMEDIATE);  // We need IMMEDIATE so accessing GTK_COLOR_CHOOSER(dlg) is not UB
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

void Control::save(std::function<void(bool)> callback) { saveImpl(false, std::move(callback)); }

void Control::saveAs(std::function<void(bool)> callback) { saveImpl(true, std::move(callback)); }

void Control::saveImpl(bool saveAs, std::function<void(bool)> callback) {
    // clear selection before saving
    clearSelectionEndText();

    this->doc->lock();
    fs::path filepath = this->doc->getFilepath();
    this->doc->unlock();

    auto doSave = [ctrl = this, cb = std::move(callback)]() {
        // clear selection before saving
        ctrl->clearSelectionEndText();

        auto* job = new SaveJob(ctrl, std::move(cb));
        ctrl->scheduler->addJob(job, JOB_PRIORITY_URGENT);
        job->unref();
    };

    if (saveAs || filepath.empty()) {
        // No need to backup the old saved file, as there is none
        this->doc->lock();
        this->doc->setCreateBackupOnSave(false);
        auto suggestedPath = this->doc->createSaveFolder(this->settings->getLastSavePath());
        suggestedPath /= this->doc->createSaveFilename(Document::XOPP, this->settings->getDefaultSaveName());
        this->doc->unlock();
        xoj::SaveExportDialog::showSaveFileDialog(getGtkWindow(), settings, std::move(suggestedPath),
                                                  [doSave = std::move(doSave), ctrl = this](std::optional<fs::path> p) {
                                                      if (p && !p->empty()) {
                                                          ctrl->settings->setLastSavePath(p->parent_path());
                                                          ctrl->doc->lock();
                                                          ctrl->doc->setFilepath(std::move(p.value()));
                                                          ctrl->doc->unlock();
                                                          doSave();
                                                      }
                                                  });
    } else {
        doSave();
    }
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
    auto afterClosed = [this, allowCancel](bool closed) {
        if (!closed) {
            if (!allowCancel) {
                // Cancel is not allowed, and the user close or did not save
                // This is probably called from macOS, where the Application
                // now will be killed - therefore do an emergency save.
                emergencySave();
            }
        } else {
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
    };

    this->close(std::move(afterClosed), true, allowCancel);
}

void Control::close(std::function<void(bool)> callback, const bool allowDestroy, const bool allowCancel) {
    clearSelectionEndText();
    metadata->documentChanged();
    resetGeometryTool();

    bool safeToClose = !undoRedo->isChanged();
    if (!safeToClose) {
        fs::path path = doc->getFilepath();
        const bool fileRemoved = !path.empty() && !fs::exists(path);
        const auto message = fileRemoved ? _("Document file was removed.") : _("This document is not saved yet.");
        const bool saveAs = fileRemoved || path.empty();
        const auto saveLabel = saveAs ? _("Save As...") : _("Save");

        enum { SAVE = 1, DISCARD, CANCEL };
        std::vector<XojMsgBox::Button> buttons = {{saveLabel, SAVE}, {_("Discard"), DISCARD}};
        if (allowCancel) {
            buttons.emplace_back(_("Cancel"), CANCEL);
        }
        XojMsgBox::askQuestion(getGtkWindow(), message, std::string(), std::move(buttons),
                               [ctrl = this, saveAs, allowDestroy, callback = std::move(callback)](int response) {
                                   auto execAfter = [allowDestroy, ctrl, cb = std::move(callback)](bool saved) {
                                       if (saved && allowDestroy) {
                                           ctrl->closeDocument();
                                       }
                                       cb(saved);
                                   };
                                   if (response == SAVE) {
                                       ctrl->saveImpl(saveAs, std::move(execAfter));
                                       return;
                                   }
                                   bool safeToClose = response == DISCARD;
                                   execAfter(safeToClose);
                               });
    } else {
        if (allowDestroy) {
            this->closeDocument();
        }
        callback(true);
    }
}

void Control::closeDocument() {
    this->undoRedo->clearContents();

    this->doc->lock();
    // FIXME: there could potentially be a data race if a job requires the old document but runs after it is closed
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

void Control::showAbout() {
    auto popup = xoj::popup::PopupWindowWrapper<xoj::popup::AboutDialog>(this->gladeSearchPath);
    popup.show(GTK_WINDOW(this->win->getWindow()));
}

static void onGtkDemoShown(GObject* proc_object, GAsyncResult* res, gpointer) {
    gboolean success = g_subprocess_wait_finish(G_SUBPROCESS(proc_object), res, NULL);

    if (success) {
        g_message("Gtk demo run successfully!\n");
    } else {
        g_message("Something went wrong running the Gtk demo!\n");
    }
}

void Control::showGtkDemo() {
    std::string binary = "gtk3-demo";
#ifdef __APPLE__
    if (!xoj::util::OwnedCString::assumeOwnership(g_find_program_in_path(binary.c_str()))) {
        // Try absolute path for binary
        auto path = Stacktrace::getExePath() / binary;

        binary = path.string();
    }
#endif
    gchar* prog = g_find_program_in_path(binary.c_str());
    if (!prog) {
        XojMsgBox::showErrorToUser(getGtkWindow(), "gtk3-demo was not found in path");
        return;
    }
    GError* err = nullptr;
    GSubprocess* process = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, &err, prog, nullptr);
    g_free(prog);

    if (err != nullptr) {
        std::string message =
                FS(_F("Creating Gtk demo subprocess failed: {1} (exit code: {2})") % err->message % err->code);
        XojMsgBox::showErrorToUser(getGtkWindow(), message);
        g_error_free(err);
    }

    g_subprocess_wait_async(process, nullptr, reinterpret_cast<GAsyncReadyCallback>(onGtkDemoShown), nullptr);
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
    t->setText(std::move(text));
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
        Stacktrace::printStacktrace();
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

auto Control::getPalette() const -> const Palette& { return *(this->palette); }

auto Control::loadPaletteFromSettings() -> void {
    const auto palettePath = this->settings->getColorPaletteSetting();
    if (palettePath.empty()) {
        this->palette->load_default();
        return;
    }

    auto newPalette = std::make_unique<Palette>(palettePath);
    this->palette = std::move(newPalette);

    // If file does not exist there is no need to attempt parsing it
    if (!fs::exists(this->palette->getFilePath())) {
        this->palette->load_default();
        return;
    }

    try {
        this->palette->load();
    } catch (const std::exception& e) {
        this->palette->parseErrorDialog(e);
        this->palette->load_default();
    }
}
