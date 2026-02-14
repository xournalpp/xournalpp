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

#include <cstddef>   // for size_t
#include <limits>    // for numeric_limits
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <string>    // for string
#include <utility>   // for pair
#include <vector>    // for vector

#include <gdk/gdk.h>  // for GdkEventKey, GdkEventExpose
#include <glib.h>     // for gboolean
#include <gtk/gtk.h>  // for GtkWidget, GtkAllocation

#include "control/zoom/ZoomListener.h"     // for ZoomListener
#include "gui/inputdevices/InputEvents.h"  // for KeyEvent
#include "model/DocumentChangeType.h"      // for DocumentChangeType
#include "model/DocumentListener.h"        // for DocumentListener
#include "pdf/base/XojPdfPage.h"           // for XojPdfRectangle
#include "util/Interval.h"                 // for Interval
#include "util/Util.h"                     // for npos

class Control;
class XournalppCursor;
class Document;
class EditSelection;
class XojPageView;
class XojPdfRectangle;
class PdfCache;
class RepaintHandler;
class ScrollHandling;
class TextEditor;
class HandRecognition;
class Layout;
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

    void scrollTo(size_t pageNo, XojPdfRectangle rect = {0, 0, -1, -1});

    // Relative navigation in current layout:
    void pageRelativeXY(int offCol, int offRow);

    size_t getCurrentPage() const;

    void clearSelection();

    void layerChanged(size_t page);

    void requestFocus();

    void forceUpdatePagenumbers();

    XojPageView* getViewFor(size_t pageNr) const;

    bool searchTextOnPage(const std::string& text, size_t pageNumber, size_t index, size_t* occurrences,
                          XojPdfRectangle* matchRect);

    bool cut();
    bool copy();
    bool paste();

    void getPasteTarget(double& x, double& y) const;

    bool actionDelete();

    void endTextAllPages(XojPageView* except = nullptr) const;

    void endSplineAllPages() const;

    int getDisplayWidth() const;
    int getDisplayHeight() const;

    bool isPageVisible(size_t page, int* visibleHeight) const;

    void ensureRectIsVisible(int x, int y, int width, int height);

    void setSelection(EditSelection* selection);
    EditSelection* getSelection() const;
    void deleteSelection(EditSelection* sel = nullptr);
    void repaintSelection(bool evenWithoutSelection = false);

    TextEditor* getTextEditor() const;
    std::vector<std::unique_ptr<XojPageView>> const& getViewPages() const;

    Control* getControl() const;
    double getZoom() const;
    int getDpiScaleFactor() const;
    Document* getDocument() const;
    PdfCache* getCache() const;
    RepaintHandler* getRepaintHandler() const;
    GtkWidget* getWidget() const;
    XournalppCursor* getCursor() const;
    Layout* getLayout() const;


    /// Return the rectangle (if any) which is visible on screen in page cooordinates
    xoj::util::Rectangle<double>* getVisibleRect(size_t page) const;
    /// Return the rectangle (if any) which is visible on screen in page cooordinates
    xoj::util::Rectangle<double>* getVisibleRect(const XojPageView* redrawable) const;

    /**
     * Recreate the PDF cache, for example after the underlying PDF file has changed
     */
    void recreatePdfCache();

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
    bool onKeyPressEvent(const KeyEvent& event);
    bool onKeyReleaseEvent(const KeyEvent& event);

    void onSettingsChanged();

    /// Called upon scrolling. Updates/flushes the views' buffers as needed and sets the most visible page as active
    void updateVisibility();

private:
    /**
     * Scrollbars
     */
    ScrollHandling* scrollHandling = nullptr;

    GtkWidget* widget = nullptr;

    std::vector<std::unique_ptr<XojPageView>> viewPages;

    Control* control = nullptr;

    size_t currentPage = 0;
    size_t lastSelectedPage = npos;

    std::unique_ptr<PdfCache> cache;

    /**
     * Handler for rerendering pages / repainting pages
     */
    std::unique_ptr<RepaintHandler> repaintHandler;

    ///< pages outside of this range of indices do not have a buffer
    Interval<size_t> pagesMaybeWithBuffers{std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::lowest()};

    std::optional<unsigned int> maxCacheUsageOverride = std::nullopt;
};
