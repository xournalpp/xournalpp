/*
 * Xournal++
 *
 * The widget wich displays the PDF and the drawings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <control/Control.h>
#include <gtk/gtk.h>
#include <gui/toolbarMenubar/ToolZoomSlider.h>
#include <model/softstorage/PageLayout.h>
#include <model/softstorage/Selections.h>

#include "model/DocumentListener.h"
#include "model/PageRef.h"
#include "widgets/XournalWidget.h"

#include "XournalRenderer.h"

class XournalView: public DocumentListener {
public:
    XournalView(GtkScrolledWindow* parent, Control* control);
    virtual ~XournalView();

public:
    void scrollTo(size_t pageNo, double y = 0);

    // Relative navigation in current layout:
    void pageRelativeXY(int offCol, int offRow);

    size_t getCurrentPage() const;

    void clearSelection();

    void layerChanged(size_t page);

    void requestFocus();

    void forceUpdatePagenumbers();

    XojPageView* getViewFor(size_t pageNr);

    bool searchTextOnPage(string text, size_t p, int* occures, double* top);

    bool cut();
    bool copy();
    bool paste();

    void getPasteTarget(double& x, double& y);

    bool actionDelete();

    void endTextAllPages(XojPageView* except = nullptr);

    void resetShapeRecognizer();

    bool isPageVisible(size_t page, int* visibleHeight);

    void ensureRectIsVisible(int x, int y, int width, int height);

    auto scrollRelative(int offsetX, int offsetY) -> void;

    void setSelection(std::shared_ptr<EditSelection> selection);
    void deleteSelection();
    void repaintSelection(bool evenWithoutSelection = false);

    TextEditor* getTextEditor();

    Control* getControl();
    double getZoom();
    int getDpiScaleFactor();
    Document* getDocument();
    PdfCache* getCache();
    XournalWidget* getWidget();
    XournalppCursor* getCursor();

    Rectangle<double>* getVisibleRect(XojPageView* redrawable);

    auto getLayout() -> std::shared_ptr<PageLayout>;
    auto getViewport() -> std::shared_ptr<Viewport>;
    auto getSelections() -> std::shared_ptr<Selections>;

    // Adapter functions
    /**
     * Layout zoom adjusted coordinates
     * @param x
     * @param y
     * @return
     */
    auto getPageViewAt(double x, double y) -> XojPageView*;

    /**
     *
     * @return
     */
    auto getZoomReal() -> double;

    /**
     * Indicate changed layout settings (probably noop as xournalview will get updated on settings change by default)
     */
    auto layoutPages() -> void;

    /**
     * Update zoom presentation value
     * @return success
     */
    auto updateZoomPresentationValue() -> bool;

    /**
     * Update zoom fit value
     * @return success
     */
    auto updateZoomFitValue() -> bool;

    /**
     *
     * @return iszoompresentationmode
     */
    auto isZoomPresentationMode() -> bool;

    /**
     * Is zoom fit width mode
     * @return
     */
    auto isZoomFitMode() -> bool;

    auto setZoomFitMode(bool enable) -> void;

    /**
     * Set scale factor (not raw), has to get dpi adjusted
     * @param value
     */
    auto setZoom(double value) -> void;
    auto getZoomFitValue() -> double;

    auto getZoom100Value() -> double;

    auto startZoomSequence(double x, double y) -> void;
    auto zoomSequenceChange(double zoom, bool b) -> void;
    auto endZoomSequence() -> void;

    auto addZoomListener(ToolZoomSlider* slider) -> void;

    auto setSelection(EditSelection* selection) -> void;

    auto getScrollPositionAfterZoom() -> std::tuple<double, double>;
    auto setScrollPositionAfterZoom(double x, double y) -> void;

    auto getSelection() -> std::shared_ptr<EditSelection>;

    /**
     * Return widgets translation relative to top window
     * @param x
     * @param y
     */
    auto translateToplevel(int* x, int* y) -> void;

    /**
     * Repaint specified rectangle, coordinates are dependent on current layout state and absolute
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     */
    auto repaintArea(int x1, int y1, int x2, int y2) -> void;

    auto getVisibleRect() -> Rectangle<double>;

    constexpr auto getZoomMax() -> double;
    constexpr auto getZoomMin() -> double;

public:
    // DocumentListener interface
    void pageSelected(size_t page);
    void pageSizeChanged(size_t page);
    void pageChanged(size_t page);
    void pageInserted(size_t page);
    void pageDeleted(size_t page);
    void documentChanged(DocumentChangeType type);

public:
    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

private:
    std::unique_ptr<XournalWidget> widget;
    std::shared_ptr<PageLayout> layout;
    std::shared_ptr<Viewport> viewport;
    std::shared_ptr<InputContext> input;
    std::shared_ptr<Selections> selection;

    Control* control = nullptr;
    PdfCache* cache = nullptr;
};
