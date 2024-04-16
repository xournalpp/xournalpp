#include "XournalView.h"

#include <algorithm>  // for max, min
#include <iterator>   // for begin
#include <memory>     // for unique_ptr, make_unique
#include <optional>   // for optional

#include <gdk/gdk.h>         // for GdkEventKey, GDK_SHIF...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Page_Down
#include <glib-object.h>     // for g_object_ref_sink

#include "control/Control.h"                     // for Control
#include "control/PdfCache.h"                    // for PdfCache
#include "control/ScrollHandler.h"               // for ScrollHandler
#include "control/ToolHandler.h"                 // for ToolHandler
#include "control/actions/ActionDatabase.h"      // for ActionDatabase
#include "control/jobs/XournalScheduler.h"       // for XournalScheduler
#include "control/settings/MetadataManager.h"    // for MetadataManager
#include "control/settings/Settings.h"           // for Settings
#include "control/tools/CursorSelectionType.h"   // for CURSOR_SELECTION_NONE
#include "control/tools/EditSelection.h"         // for EditSelection
#include "control/zoom/ZoomControl.h"            // for ZoomControl
#include "gui/MainWindow.h"                      // for MainWindow
#include "gui/PdfFloatingToolbox.h"              // for PdfFloatingToolbox
#include "gui/inputdevices/HandRecognition.h"    // for HandRecognition
#include "gui/inputdevices/InputContext.h"       // for InputContext
#include "gui/toolbarMenubar/ColorToolItem.h"    // for ColorToolItem
#include "gui/toolbarMenubar/ToolMenuHandler.h"  // for ToolMenuHandler
#include "gui/widgets/XournalWidget.h"           // for gtk_xournal_get_layout
#include "model/Document.h"                      // for Document
#include "model/Element.h"                       // for Element, ELEMENT_STROKE
#include "model/PageRef.h"                       // for PageRef
#include "model/Stroke.h"                        // for Stroke, StrokeTool::E...
#include "model/XojPage.h"                       // for XojPage
#include "undo/DeleteUndoAction.h"               // for DeleteUndoAction
#include "undo/UndoRedoHandler.h"                // for UndoRedoHandler
#include "util/Assert.h"                         // for xoj_assert
#include "util/Point.h"                          // for Point
#include "util/Rectangle.h"                      // for Rectangle
#include "util/Util.h"                           // for npos
#include "util/glib_casts.h"                     // for wrap_v
#include "util/safe_casts.h"                     // for round_cast

#include "Layout.h"           // for Layout
#include "PageView.h"         // for XojPageView
#include "RepaintHandler.h"   // for RepaintHandler
#include "XournalppCursor.h"  // for XournalppCursor

using xoj::util::Rectangle;

constexpr int REGULAR_MOVE_AMOUNT = 3;
constexpr int SMALL_MOVE_AMOUNT = 1;
constexpr int LARGE_MOVE_AMOUNT = 10;

std::pair<size_t, size_t> XournalView::preloadPageBounds(size_t page, size_t maxPage) {
    const size_t preloadBefore = this->control->getSettings()->getPreloadPagesBefore();
    const size_t preloadAfter = this->control->getSettings()->getPreloadPagesAfter();
    const size_t lower = page > preloadBefore ? page - preloadBefore : 0;
    const size_t upper = std::min(maxPage, page + preloadAfter);
    return {lower, upper};
}

static void redraw(GtkAdjustment*, gpointer d) { gtk_widget_queue_draw(GTK_WIDGET(d)); }

XournalView::XournalView(GtkScrolledWindow* parent, Control* control, ScrollHandling* scrollHandling):
        scrollHandling(scrollHandling), control(control) {
    Document* doc = control->getDocument();
    doc->lock();
    if (doc->getPdfPageCount() != 0) {
        this->cache = std::make_unique<PdfCache>(doc->getPdfDocument(), control->getSettings());
    }
    doc->unlock();

    registerListener(control);

    InputContext* inputContext = new InputContext(this, scrollHandling);
    this->widget.reset(gtk_xournal_new(this, inputContext), xoj::util::adopt);

    gtk_scrolled_window_set_child(parent, this->widget.get());
    gtk_viewport_set_scroll_to_focus(GTK_VIEWPORT(gtk_scrolled_window_get_child(parent)), false);

    g_signal_connect(getWidget(), "realize", G_CALLBACK(onRealized), this);

    // Repaint when scrolling
    auto* hadj = gtk_scrolled_window_get_hadjustment(parent);
    g_signal_connect_object(hadj, "changed", G_CALLBACK(redraw), this->widget.get(), GConnectFlags(0));
    g_signal_connect_object(hadj, "value-changed", G_CALLBACK(redraw), this->widget.get(), GConnectFlags(0));
    auto* vadj = gtk_scrolled_window_get_vadjustment(parent);
    g_signal_connect_object(vadj, "changed", G_CALLBACK(redraw), this->widget.get(), GConnectFlags(0));
    g_signal_connect_object(vadj, "value-changed", G_CALLBACK(redraw), this->widget.get(), GConnectFlags(0));


    this->repaintHandler = std::make_unique<RepaintHandler>(this);
    this->handRecognition = std::make_unique<HandRecognition>(this->widget.get(), inputContext, control->getSettings());

    control->getZoomControl()->addZoomListener(this);

    gtk_window_set_default_widget(GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(parent), GTK_TYPE_WINDOW)),
                                  this->widget.get());
    [[maybe_unused]] auto r = gtk_widget_grab_focus(this->widget.get());
    xoj_assert_message(r, "Main widget must be able to grab focus (all its ancestors must have can-focus==true)");

    this->cleanupTimeout = g_timeout_add_seconds(5, xoj::util::wrap_v<clearMemoryTimer>, this);
}

XournalView::~XournalView() { g_source_remove(this->cleanupTimeout); }


auto XournalView::clearMemoryTimer(XournalView* widget) -> gboolean {
    widget->cleanupBufferCache();
    return true;
}

auto XournalView::cleanupBufferCache() -> void {
    const auto& [pagesLower, pagesUpper] = this->preloadPageBounds(this->currentPage, this->viewPages.size());
    xoj_assert(pagesLower <= pagesUpper);

    for (size_t i = 0; i < this->viewPages.size(); i++) {
        auto&& page = this->viewPages[i];
        const size_t pageNum = i + 1;
        const bool isPreload = pagesLower <= pageNum && pageNum <= pagesUpper;
        if (!isPreload && !page->isVisible() && page->hasBuffer()) {
            page->deleteViewBuffer();
        }
    }
}

auto XournalView::getCurrentPage() const -> size_t { return currentPage; }

const int scrollKeySize = 30;

auto XournalView::onKeyPressEvent(const KeyEvent& event) -> bool {
    size_t p = getCurrentPage();
    if (p != npos && p < this->viewPages.size()) {
        auto& v = this->viewPages[p];
        if (v->onKeyPressEvent(event)) {
            return true;
        }
    }

    auto keyval = event.keyval;
    auto state = event.state;
    if (auto* tool = getControl()->getWindow()->getPdfToolbox(); tool->hasSelection()) {
        if ((keyval == GDK_KEY_c && state == GDK_CONTROL_MASK) || keyval == GDK_KEY_Copy) {
            // Shortcut to get selected PDF text.
            tool->copyTextToClipboard();
            return true;
        }
    }

    if (auto* selection = getSelection(); selection) {
        if (keyval == GDK_KEY_Escape) {
            clearSelection();
            return true;
        }

        int d = REGULAR_MOVE_AMOUNT;
        if (state == GDK_ALT_MASK) {
            d = SMALL_MOVE_AMOUNT;
        } else if (state == GDK_SHIFT_MASK) {
            d = LARGE_MOVE_AMOUNT;
        }

        int xdir = 0;
        int ydir = 0;
        if (keyval == GDK_KEY_Left) {
            xdir = -1;
        } else if (keyval == GDK_KEY_Up) {
            ydir = -1;
        } else if (keyval == GDK_KEY_Right) {
            xdir = 1;
        } else if (keyval == GDK_KEY_Down) {
            ydir = 1;
        }
        if (xdir != 0 || ydir != 0) {
            selection->moveSelection(d * xdir, d * ydir, /*addMoveUndo=*/true);
            selection->ensureWithinVisibleArea();
            return true;
        }
    }

    Layout* layout = gtk_xournal_get_layout(this->widget.get());

    if (!state) {
        if (keyval == GDK_KEY_Page_Down || keyval == GDK_KEY_KP_Page_Down) {
            control->getScrollHandler()->goToNextPage();
            return true;
        }
        if (keyval == GDK_KEY_Page_Up || keyval == GDK_KEY_KP_Page_Up) {
            control->getScrollHandler()->goToPreviousPage();
            return true;
        }
    }

    if (keyval == GDK_KEY_space) {
        GtkAllocation alloc = {0};
        gtk_widget_get_allocation(gtk_widget_get_parent(this->widget.get()), &alloc);
        int windowHeight = alloc.height - scrollKeySize;

        if (!state) {
            layout->scrollRelative(0, windowHeight);
            return true;
        }
        if (state == GDK_SHIFT_MASK) {
            layout->scrollRelative(0, -windowHeight);
            return true;
        }
    }

    // Numeric keypad always navigates by page
    if (keyval == GDK_KEY_KP_Up) {
        this->pageRelativeXY(0, -1);
        return true;
    }

    if (keyval == GDK_KEY_KP_Down) {
        this->pageRelativeXY(0, 1);
        return true;
    }

    if (keyval == GDK_KEY_KP_Left) {
        this->pageRelativeXY(-1, 0);
        return true;
    }

    if (keyval == GDK_KEY_KP_Right) {
        this->pageRelativeXY(1, 0);
        return true;
    }


    if (keyval == GDK_KEY_Up || keyval == GDK_KEY_k || keyval == GDK_KEY_K) {
        if (control->getSettings()->isPresentationMode()) {
            control->getScrollHandler()->goToPreviousPage();
            return true;
        }

        if (state == GDK_SHIFT_MASK) {
            this->pageRelativeXY(0, -1);
            return true;
        }
        if (!state) {
            layout->scrollRelative(0, -scrollKeySize);
            return true;
        }
    }

    if (keyval == GDK_KEY_Down || keyval == GDK_KEY_j || keyval == GDK_KEY_J) {
        if (control->getSettings()->isPresentationMode()) {
            control->getScrollHandler()->goToNextPage();
            return true;
        }

        if (state == GDK_SHIFT_MASK) {
            this->pageRelativeXY(0, 1);
            return true;
        }
        if (!state) {
            layout->scrollRelative(0, scrollKeySize);
            return true;
        }
    }

    if (keyval == GDK_KEY_Left || keyval == GDK_KEY_h) {
        if (state == GDK_SHIFT_MASK) {
            this->pageRelativeXY(-1, 0);
            return true;
        }
        if (!state) {
            if (control->getSettings()->isPresentationMode()) {
                control->getScrollHandler()->goToPreviousPage();
            } else {
                layout->scrollRelative(-scrollKeySize, 0);
            }
            return true;
        }
    }

    if (keyval == GDK_KEY_Right || keyval == GDK_KEY_l) {
        if (state == GDK_SHIFT_MASK) {
            this->pageRelativeXY(1, 0);
            return true;
        }

        if (!state) {
            if (control->getSettings()->isPresentationMode()) {
                control->getScrollHandler()->goToNextPage();
            } else {
                layout->scrollRelative(scrollKeySize, 0);
            }
            return true;
        }
    }

    if (keyval == GDK_KEY_End || keyval == GDK_KEY_KP_End) {
        control->getScrollHandler()->goToLastPage();
        return true;
    }

    if (keyval == GDK_KEY_Home || keyval == GDK_KEY_KP_Home) {
        control->getScrollHandler()->goToFirstPage();
        return true;
    }

    // Switch color on number key
    auto& colors = control->getWindow()->getToolMenuHandler()->getColorToolItems();
    if (!state && (keyval >= GDK_KEY_0) && (keyval < GDK_KEY_0 + std::min((std::size_t)10, colors.size()))) {
        std::size_t index = std::min(colors.size() - 1, (std::size_t)(9 + (keyval - GDK_KEY_0)) % 10);
        const auto& colorToolItem = colors.at(index);
        if (auto* db = control->getActionDatabase(); db->isActionEnabled(Action::TOOL_COLOR)) {
            control->getActionDatabase()->fireChangeActionState(Action::TOOL_COLOR, colorToolItem->getColor());
        }
        return true;
    }
    return false;
}

auto XournalView::getRepaintHandler() const -> RepaintHandler* { return this->repaintHandler.get(); }

auto XournalView::onKeyReleaseEvent(const KeyEvent& event) -> bool {
    size_t p = getCurrentPage();
    if (p != npos && p < this->viewPages.size()) {
        auto& v = this->viewPages[p];
        if (v->onKeyReleaseEvent(event)) {
            return true;
        }
    }

    return false;
}

void XournalView::onRealized(GtkWidget* widget, XournalView* view) {
    // Disable event compression
    if (gtk_widget_get_realized(view->getWidget())) {
        g_warning("Find replacement for gdk_window_set_event_compression()");
        // gdk_window_set_event_compression(gtk_widget_get_window(view->getWidget()), false);
    } else {
        g_warning("could not disable event compression");
    }
}

void XournalView::onSettingsChanged() {
    if (this->cache) {
        this->cache->updateSettings(control->getSettings());
    }
}

// send the focus back to the appropriate widget
void XournalView::requestFocus() { gtk_widget_grab_focus(this->widget.get()); }

auto XournalView::searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                                   XojPdfRectangle* matchRect) -> bool {
    if (pageNumber == npos || pageNumber >= this->viewPages.size()) {
        return false;
    }
    auto& v = this->viewPages[pageNumber];

    return v->searchTextOnPage(text, index, occurrences, matchRect);
}

void XournalView::forceUpdatePagenumbers() {
    size_t p = this->currentPage;
    this->currentPage = npos;

    control->firePageSelected(p);
}

auto XournalView::getViewFor(size_t pageNr) const -> XojPageView* {
    if (pageNr == npos || pageNr >= this->viewPages.size()) {
        return nullptr;
    }
    return this->viewPages[pageNr].get();
}

void XournalView::pageSelected(size_t page) {
    if (this->currentPage == page && this->lastSelectedPage == page) {
        return;
    }

    Document* doc = control->getDocument();
    doc->lock();
    auto const& file = doc->getEvMetadataFilename();
    doc->unlock();

    control->getMetadataManager()->storeMetadata(file, static_cast<int>(page), getZoom());

    control->getWindow()->getPdfToolbox()->userCancelSelection();

    if (this->lastSelectedPage != npos && this->lastSelectedPage < this->viewPages.size()) {
        this->viewPages[this->lastSelectedPage]->setSelected(false);
    }

    endTextAllPages();

    this->currentPage = page;

    size_t pdfPage = npos;

    if (page != npos && page < viewPages.size()) {
        auto& vp = viewPages[page];
        vp->setSelected(true);
        lastSelectedPage = page;
        pdfPage = vp->getPage()->getPdfPageNr();
    }

    control->updatePageNumbers(currentPage, pdfPage);

    control->updateBackgroundSizeButton();
    control->updatePageActions();

    if (control->getSettings()->isEagerPageCleanup()) {
        this->cleanupBufferCache();
    }

    // Load surrounding pages if they are not
    const auto& [pagesLower, pagesUpper] = preloadPageBounds(page, this->viewPages.size());
    xoj_assert(pagesLower <= pagesUpper);
    for (size_t i = pagesLower; i < pagesUpper; i++) {
        if (!this->viewPages[i]->hasBuffer()) {
            this->viewPages[i]->rerenderPage();
        }
    }
}

auto XournalView::getControl() const -> Control* { return control; }

void XournalView::scrollTo(size_t pageNo, XojPdfRectangle rect) {
    double zoom = getZoom();
    if (pageNo >= this->viewPages.size()) {
        return;
    }

    auto& v = this->viewPages[pageNo];

    // Make sure it is visible
    Layout* layout = gtk_xournal_get_layout(this->widget.get());

    int x = v->getX() + round_cast<int>(rect.x1 * zoom);
    int y = v->getY() + round_cast<int>(rect.y1 * zoom);
    int width;
    int height;
    if (rect.x2 == -1 || rect.y2 == -1) {
        width = v->getDisplayWidth();
        height = v->getDisplayHeight();
    } else {
        width = round_cast<int>((rect.x2 - rect.x1) * zoom);
        height = round_cast<int>((rect.y2 - rect.y1) * zoom);
    }

    layout->ensureRectIsVisible(x, y, width, height);

    // Select the page
    control->firePageSelected(pageNo);
}


void XournalView::pageRelativeXY(int offCol, int offRow) {
    size_t currPage = getCurrentPage();

    XojPageView* view = getViewFor(currPage);
    int row = view->getMappedRow();
    int col = view->getMappedCol();

    Layout* layout = gtk_xournal_get_layout(this->widget.get());
    auto optionalPageIndex = layout->getPageIndexAtGridMap(as_unsigned(row + offRow), as_unsigned(col + offCol));
    if (optionalPageIndex) {
        this->scrollTo(*optionalPageIndex);
    }
}


void XournalView::endTextAllPages(XojPageView* except) const {
    for (auto& v: this->viewPages) {
        if (except != v.get()) {
            v->endText();
        }
    }
}

void XournalView::endSplineAllPages() const {
    for (auto& v: this->viewPages) {
        v->endSpline();
    }
}

void XournalView::layerChanged(size_t page) {
    if (page != npos && page < this->viewPages.size()) {
        this->viewPages[page]->rerenderPage();
    }
}

void XournalView::getPasteTarget(double& x, double& y) const {
    size_t pageNo = getCurrentPage();
    if (pageNo == npos) {
        return;
    }

    Rectangle<double>* rect = getVisibleRect(pageNo);

    if (rect) {
        x = rect->x + rect->width / 2;
        y = rect->y + rect->height / 2;
        delete rect;
    }
}

/**
 * Return the rectangle which is visible on screen, in document cooordinates
 *
 * Or nullptr if the page is not visible
 */
auto XournalView::getVisibleRect(size_t page) const -> Rectangle<double>* {
    if (page == npos || page >= this->viewPages.size()) {
        return nullptr;
    }
    auto& p = this->viewPages[page];

    return getVisibleRect(p.get());
}

auto XournalView::getVisibleRect(const XojPageView* redrawable) const -> Rectangle<double>* {
    return gtk_xournal_get_visible_area(this->widget.get(), redrawable);
}

void XournalView::recreatePdfCache() {
    this->cache.reset();

    Document* doc = control->getDocument();
    doc->lock();
    if (doc->getPdfPageCount() != 0) {
        this->cache = std::make_unique<PdfCache>(doc->getPdfDocument(), control->getSettings());
    }
    doc->unlock();
}

/**
 * @return Helper class for Touch specific fixes
 */
auto XournalView::getHandRecognition() const -> HandRecognition* { return handRecognition.get(); }

/**
 * @return Scrollbars
 */
auto XournalView::getScrollHandling() const -> ScrollHandling* { return scrollHandling; }

auto XournalView::getWidget() const -> GtkWidget* { return widget.get(); }

void XournalView::ensureRectIsVisible(int x, int y, int width, int height) {
    Layout* layout = gtk_xournal_get_layout(this->widget.get());
    layout->ensureRectIsVisible(x, y, width, height);
}

void XournalView::zoomChanged() {

    size_t currentPage = this->getCurrentPage();
    XojPageView* view = getViewFor(currentPage);

    ZoomControl* zoom = control->getZoomControl();

    if (!view) {
        return;
    }

    layoutPages();

    if (zoom->isZoomPresentationMode() || zoom->isZoomFitMode()) {
        scrollTo(currentPage);
    } else if (zoom->isZoomSequenceActive()) {
        auto pos = zoom->getScrollPositionAfterZoom();
        Layout* layout = gtk_xournal_get_layout(this->widget.get());
        layout->scrollAbs(pos.x, pos.y);
    }

    Document* doc = control->getDocument();
    doc->lock();
    auto const& file = doc->getEvMetadataFilename();
    doc->unlock();

    control->getMetadataManager()->storeMetadata(file, static_cast<int>(getCurrentPage()), zoom->getZoomReal());

    // Updates the Eraser's cursor icon in order to make it as big as the erasing area
    control->getCursor()->updateCursor();

    // if we changed the zoom of the page, we should hide the pdf floating toolbox
    // and if user clicked the selection again, the floating toolbox shows again
    control->getWindow()->getPdfToolbox()->hide();

    this->control->getScheduler()->blockRerenderZoom();
}

void XournalView::pageSizeChanged(size_t page) {
    layoutPages();
    if (page != npos && page < this->viewPages.size()) {
        this->viewPages[page]->rerenderPage(/* sizeChanged */ true);
    }
}

void XournalView::pageChanged(size_t page) {
    if (page != npos && page < this->viewPages.size()) {
        this->viewPages[page]->rerenderPage();
    }
}

void XournalView::pageDeleted(size_t page) {
    const size_t currentPageNo = control->getCurrentPageNo();

    viewPages.erase(begin(viewPages) + static_cast<long>(page));

    layoutPages();

    if (currentPageNo > page) {
        control->getScrollHandler()->scrollToPage(currentPageNo - 1);
    } else {
        control->getScrollHandler()->scrollToPage(currentPageNo);
    }
}

auto XournalView::getTextEditor() const -> TextEditor* {
    for (auto&& page: viewPages) {
        if (page->getTextEditor()) {
            return page->getTextEditor();
        }
    }

    return nullptr;
}

auto XournalView::getCache() const -> PdfCache* { return this->cache.get(); }

void XournalView::pageInserted(size_t page) {
    Document* doc = control->getDocument();
    doc->lock();
    auto pageView = std::make_unique<XojPageView>(this, doc->getPage(page));
    doc->unlock();

    viewPages.insert(begin(viewPages) + as_signed(page), std::move(pageView));

    layoutPages();
    // check which pages are visible and select the most visible page
    Layout* layout = gtk_xournal_get_layout(this->widget.get());
    layout->updateVisibility();
}

auto XournalView::getZoom() const -> double { return control->getZoomControl()->getZoom(); }

auto XournalView::getDpiScaleFactor() const -> int { return gtk_widget_get_scale_factor(widget.get()); }

void XournalView::clearSelection() {
    EditSelection* sel = GTK_XOURNAL(widget.get())->selection;
    GTK_XOURNAL(widget.get())->selection = nullptr;
    delete sel;

    control->setClipboardHandlerSelection(getSelection());

    getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
    control->getToolHandler()->setSelectionEditTools(false, false, false, false);
}

void XournalView::deleteSelection(EditSelection* sel) {
    if (sel == nullptr) {
        sel = getSelection();
    }

    if (sel) {
        auto undo = std::make_unique<DeleteUndoAction>(sel->getSourcePage(), false);
        sel->fillUndoItem(undo.get());
        control->getUndoRedoHandler()->addUndoAction(std::move(undo));

        clearSelection();

        repaintSelection(true);
    }
}

void XournalView::setSelection(EditSelection* selection) {
    clearSelection();
    GTK_XOURNAL(this->widget.get())->selection = selection;

    control->setClipboardHandlerSelection(getSelection());

    bool canChangeSize = false;
    bool canChangeColor = false;
    bool canChangeFill = false;
    bool canChangeLineStyle = false;

    for (const Element* e: selection->getElements()) {
        switch (e->getType()) {
            case ELEMENT_TEXT:
                canChangeColor = true;
                continue;
            case ELEMENT_STROKE: {
                canChangeSize = true;

                const auto* s = dynamic_cast<const Stroke*>(e);
                if (s->getToolType() == StrokeTool::PEN) {
                    // can change everything, leave loop with break
                    canChangeColor = true;
                    canChangeFill = true;
                    canChangeLineStyle = true;
                    break;
                }
                if (s->getToolType() == StrokeTool::HIGHLIGHTER) {
                    canChangeColor = true;
                    canChangeFill = true;
                }
                continue;
            }
            default:
                continue;
        }

        // leave loop
        break;
    }

    control->getToolHandler()->setSelectionEditTools(canChangeColor, canChangeSize, canChangeFill, canChangeLineStyle);

    repaintSelection();
}

void XournalView::repaintSelection(bool evenWithoutSelection) {
    if (evenWithoutSelection) {
        gtk_widget_queue_draw(this->widget.get());
        return;
    }

    EditSelection* selection = getSelection();
    if (selection == nullptr) {
        return;
    }

    // repaint always the whole widget
    gtk_widget_queue_draw(this->widget.get());
}

void XournalView::layoutPages() {
    Layout* layout = gtk_xournal_get_layout(this->widget.get());
    layout->recalculate();

    // Todo (fabian): the following lines are conceptually wrong, the Layout::layoutPages function is meant to be
    // called by an expose event, but removing it, will break "add page".
    auto rectangle = layout->getVisibleRect();
    layout->layoutPages(std::max<int>(layout->getMinimalWidth(), round_cast<int>(rectangle.width)),
                        std::max<int>(layout->getMinimalHeight(), round_cast<int>(rectangle.height)));
}

auto XournalView::getDisplayHeight() const -> int {
    GtkAllocation allocation = {0};
    gtk_widget_get_allocation(this->widget.get(), &allocation);
    return allocation.height;
}

auto XournalView::getDisplayWidth() const -> int {
    GtkAllocation allocation = {0};
    gtk_widget_get_allocation(this->widget.get(), &allocation);
    return allocation.width;
}

auto XournalView::isPageVisible(size_t page, int* visibleHeight) const -> bool {
    Rectangle<double>* rect = getVisibleRect(page);
    if (rect) {
        if (visibleHeight) {
            *visibleHeight = round_cast<int>(rect->height);
        }

        delete rect;
        return true;
    }
    if (visibleHeight) {
        *visibleHeight = 0;
    }

    return false;
}

void XournalView::documentChanged(DocumentChangeType type) {
    if (type != DOCUMENT_CHANGE_CLEARED && type != DOCUMENT_CHANGE_COMPLETE) {
        return;
    }

    XournalScheduler* scheduler = this->control->getScheduler();
    scheduler->lock();
    scheduler->removeAllJobs();

    clearSelection();

    viewPages.clear();

    recreatePdfCache();

    Document* doc = control->getDocument();
    doc->lock();

    size_t pagecount = doc->getPageCount();
    viewPages.reserve(pagecount);
    for (size_t i = 0; i < pagecount; i++) {
        viewPages.emplace_back(std::make_unique<XojPageView>(this, doc->getPage(i)));
    }

    doc->unlock();

    layoutPages();
    scrollTo(0);

    scheduler->unlock();
}

auto XournalView::cut() -> bool {
    size_t p = getCurrentPage();
    if (p == npos || p >= viewPages.size()) {
        return false;
    }

    auto& page = viewPages[p];
    return page->cut();
}

auto XournalView::copy() -> bool {
    size_t p = getCurrentPage();
    if (p == npos || p >= viewPages.size()) {
        return false;
    }

    auto& page = viewPages[p];
    return page->copy();
}

auto XournalView::paste() -> bool {
    size_t p = getCurrentPage();
    if (p == npos || p >= viewPages.size()) {
        return false;
    }

    auto& page = viewPages[p];
    return page->paste();
}

auto XournalView::actionDelete() -> bool {
    size_t p = getCurrentPage();
    if (p == npos || p >= viewPages.size()) {
        return false;
    }

    auto& page = viewPages[p];
    return page->actionDelete();
}

auto XournalView::getDocument() const -> Document* { return control->getDocument(); }

auto XournalView::getViewPages() const -> std::vector<std::unique_ptr<XojPageView>> const& { return viewPages; }

auto XournalView::getCursor() const -> XournalppCursor* { return control->getCursor(); }

auto XournalView::getSelection() const -> EditSelection* {
    g_return_val_if_fail(this->widget.get() != nullptr, nullptr);
    g_return_val_if_fail(GTK_IS_XOURNAL(this->widget.get()), nullptr);

    return GTK_XOURNAL(this->widget.get())->selection;
}
