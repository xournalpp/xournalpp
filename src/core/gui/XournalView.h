/*
 * Xournal++
 *
 * The widget which displays the PDF and the drawings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#include <gdk/gdk.h>  // for GdkEventKey, GdkEventExpose
#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkWidget, GtkAllocation

#include "control/zoom/ZoomListener.h"  // for ZoomListener
#include "model/DocumentChangeType.h"   // for DocumentChangeType
#include "model/DocumentListener.h"     // for DocumentListener
#include "util/Util.h"                  // for npos

class Control;
class XournalppCursor;
class Document;
class EditSelection;
class XojPageView;
class PdfCache;
class RepaintHandler;
class ScrollHandling;
class TextEditor;
class HandRecognition;
class SetsquareView;
namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

class XournalView: public DocumentListener, public ZoomListener {
public:
    XournalView(GtkWidget* parent, Control* control, ScrollHandling* scrollHandling);
    ~XournalView() override;

public:
    // Recalculate the layout width and height amd layout the pages with the updated layout size
    void layoutPages();

    void scrollTo(size_t pageNo, double y = 0);

    // Relative navigation in current layout:
    void pageRelativeXY(int offCol, int offRow);

    size_t getCurrentPage() const;

    void clearSelection();

    void layerChanged(size_t page);

    void requestFocus();

    void forceUpdatePagenumbers();

    XojPageView* getViewFor(size_t pageNr) const;

    bool searchTextOnPage(const std::string& text, size_t pageNumber, size_t* occurrences, double* yOfUpperMostMatch);

    bool cut();
    bool copy();
    bool paste();

    void getPasteTarget(double& x, double& y) const;

    bool actionDelete();

    void endTextAllPages(XojPageView* except = nullptr);

    int getDisplayWidth() const;
    int getDisplayHeight() const;

    bool isPageVisible(size_t page, int* visibleHeight) const;

    void ensureRectIsVisible(int x, int y, int width, int height);

    void setSelection(EditSelection* selection);
    EditSelection* getSelection() const;
    void deleteSelection(EditSelection* sel = nullptr);
    void repaintSelection(bool evenWithoutSelection = false);

    void setSetsquareView(std::unique_ptr<SetsquareView> setsquareView);
    void resetSetsquareView();
    SetsquareView* getSetsquareView() const;
    void repaintSetsquare(bool evenWithoutSetsquare = false);

    TextEditor* getTextEditor() const;
    std::vector<XojPageView*> const& getViewPages() const;

    Control* getControl() const;
    double getZoom() const;
    int getDpiScaleFactor() const;
    Document* getDocument() const;
    PdfCache* getCache() const;
    RepaintHandler* getRepaintHandler() const;
    GtkWidget* getWidget() const;
    XournalppCursor* getCursor() const;

    xoj::util::Rectangle<double>* getVisibleRect(size_t page) const;
    xoj::util::Rectangle<double>* getVisibleRect(const XojPageView* redrawable) const;

    /**
     * A pen action was detected now, therefore ignore touch events
     * for a short time
     */
    void penActionDetected();

    /**
     * @return Helper class for Touch specific fixes
     */
    HandRecognition* getHandRecognition() const;

    /**
     * @return Scrollbars
     */
    ScrollHandling* getScrollHandling() const;

public:
    // ZoomListener interface
    void zoomChanged() override;

public:
    // DocumentListener interface
    void pageSelected(size_t page) override;
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void documentChanged(DocumentChangeType type) override;

public:
    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    static void onRealized(GtkWidget* widget, XournalView* view);

    void onSettingsChanged();

private:
    void fireZoomChanged();

    std::pair<size_t, size_t> preloadPageBounds(size_t page, size_t maxPage);

    static gboolean clearMemoryTimer(XournalView* widget);

    void cleanupBufferCache();

    static void staticLayoutPages(GtkWidget* widget, GtkAllocation* allocation, void* data);

private:
    /**
     * Scrollbars
     */
    ScrollHandling* scrollHandling = nullptr;

    GtkWidget* widget = nullptr;
    double margin = 75;

    std::vector<XojPageView*> viewPages;

    Control* control = nullptr;

    size_t currentPage = 0;
    size_t lastSelectedPage = npos;

    std::unique_ptr<PdfCache> cache;

    /**
     * Handler for rerendering pages / repainting pages
     */
    RepaintHandler* repaintHandler = nullptr;

    /**
     * Memory cleanup timeout
     */
    int cleanupTimeout = -1;

    /**
     * Helper class for Touch specific fixes
     */
    HandRecognition* handRecognition = nullptr;

    friend class Layout;
};
