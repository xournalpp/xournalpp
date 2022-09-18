/*
 * Xournal++
 *
 * Displays a single page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <mutex>   // for mutex
#include <string>  // for string
#include <vector>  // for vector

#include <cairo.h>    // for cairo_t, cairo_surface_t
#include <gdk/gdk.h>  // for GdkEventKey, GdkRectangle, GdkEventB...

#include "model/PageListener.h"       // for PageListener
#include "model/PageRef.h"            // for PageRef
#include "util/Rectangle.h"           // for Rectangle
#include "util/raii/CairoWrappers.h"  // for CairoSurfaceSPtr, CairoSPtr
#include "view/Repaintable.h"

#include "Layout.h"      // for Layout
#include "LegacyRedrawable.h"  // for Redrawable

class EraseHandler;
class InputHandler;
class SearchControl;
class Selection;
class Settings;
class Text;
class TextEditor;
class VerticalToolHandler;
class XournalView;
class Element;
class PositionInputData;
class Range;
class TexImage;
class OverlayBase;

namespace xoj::view {
class OverlayView;
}

class XojPageView: public LegacyRedrawable, public PageListener, public xoj::view::Repaintable {
public:
    XojPageView(XournalView* xournal, const PageRef& page);
    ~XojPageView() override;

public:
    void rerenderPage() override;
    void rerenderRect(double x, double y, double width, double height) override;

    void repaintPage() const override;
    void repaintArea(double x1, double y1, double x2, double y2) const override;

    // Repaintable interface
    void flagDirtyRegion(const Range& rg) const override;
    int getDPIScaling() const override;
    double getZoom() const override;
    Range getVisiblePart() const override;

    double getWidth() const override;
    double getHeight() const override;


    void setSelected(bool selected);

    void setIsVisible(bool visible);

    bool isSelected() const;

    void endText();

    bool searchTextOnPage(std::string& text, int* occures, double* top);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    bool cut();
    bool copy();
    bool paste();

    bool actionDelete();

    void deleteViewBuffer() override;

    /**
     * Returns whether this PageView contains the
     * given point on the display
     */
    bool containsPoint(int x, int y, bool local = false) const;

    /**
     * Returns Row assigned in current layout
     */
    int getMappedRow() const;

    /**
     * Returns Column assigned in current layout
     */
    int getMappedCol() const;

    GdkRGBA getSelectionColor() override;
    int getBufferPixels();

    /**
     * 0 if currently visible
     * -1 if no image is saved (never visible or cleanup)
     * else the time in Seconds
     */
    int getLastVisibleTime();
    TextEditor* getTextEditor();

    /**
     * Returns a reference to the XojPage belonging to
     * this PageView
     */
    const PageRef getPage() const;

    XournalView* getXournal() const;

    /**
     * Returns the width of this XojPageView as displayed
     * on the display taking into account the current zoom
     */
    int getDisplayWidth() const;
    double getDisplayWidthDouble() const;
    /**
     * Returns the height of this XojPageView as displayed
     * on the display taking into account the current zoom
     */
    int getDisplayHeight() const;
    double getDisplayHeightDouble() const;

    /**
     * Returns the x coordinate of this XojPageView with
     * respect to the display
     */
    int getX() const override;

    /**
     * Returns the y coordinate of this XojPageView with
     * respect to the display
     */
    int getY() const override;

    TexImage* getSelectedTex();
    Text* getSelectedText();

    xoj::util::Rectangle<double> getRect() const;

public:  // event handler
    bool onButtonPressEvent(const PositionInputData& pos);
    bool onButtonReleaseEvent(const PositionInputData& pos);
    bool onButtonDoublePressEvent(const PositionInputData& pos);
    bool onButtonTriplePressEvent(const PositionInputData& pos);
    bool onMotionNotifyEvent(const PositionInputData& pos);
    void onSequenceCancelEvent();

    /**
     * This event fires after onButtonPressEvent and also
     * if no input sequence is actively running and a stylus button was pressed
     */
    bool onButtonClickEvent(const PositionInputData& pos);

    /**
     * This method actually repaints the XojPageView, triggering
     * a rerender call if necessary
     */
    bool paintPage(cairo_t* cr, GdkRectangle* rect);

public:  // listener
    void rectChanged(xoj::util::Rectangle<double>& rect) override;
    void rangeChanged(Range& range) override;
    void pageChanged() override;
    void elementChanged(Element* elem) override;

private:
    void startText(double x, double y);

    void addRerenderRect(double x, double y, double width, double height);

    void drawLoadingPage(cairo_t* cr);

    void setX(int x);
    void setY(int y);

    void setMappedRowCol(int row, int col);  // row, column assigned by mapper during layout.

    /**
     * Shows the floating toolbox at the location of an input event
     */
    void showFloatingToolbox(const PositionInputData& pos);

    /**
     * Shows the PDF toolbox at the location of an input event
     */
    void showPdfToolbox(const PositionInputData& pos);

    auto getViewOf(OverlayBase* overlay) const -> xoj::view::OverlayView*;
    void deleteViewOf(OverlayBase* overlay);

private:
    PageRef page;
    XournalView* xournal = nullptr;
    Settings* settings = nullptr;
    EraseHandler* eraser = nullptr;
    InputHandler* inputHandler = nullptr;

    std::vector<std::unique_ptr<xoj::view::OverlayView>> overlayViews;

    /**
     * The selected (while selection)
     */
    Selection* selection = nullptr;

    /**
     * The text editor View
     */
    TextEditor* textEditor = nullptr;

    /**
     * For keeping old text changes to undo!
     */
    Text* oldtext;

    bool selected = false;

    xoj::util::CairoSurfaceSPtr crBuffer;
    std::mutex drawingMutex;

    bool inEraser = false;

    /**
     * Vertical Space
     */
    VerticalToolHandler* verticalSpace{};

    /**
     * Search handling
     */
    SearchControl* search = nullptr;

    /**
     * Unixtimestam when the page was last time in the visible area
     */
    int lastVisibleTime = -1;

    std::mutex repaintRectMutex;
    std::vector<xoj::util::Rectangle<double>> rerenderRects;
    bool rerenderComplete = false;

    int dispX{};  // position on display - set in Layout::layoutPages
    int dispY{};


    int mappedRow{};
    int mappedCol{};


    friend class RenderJob;
    friend class InputHandler;
    friend class BaseSelectObject;
    friend class SelectObject;
    friend class PlayObject;
    friend class PdfFloatingToolbox;
    // only function allowed to setX(), setY(), setMappedRowCol():
    friend void Layout::layoutPages(int width, int height);
};
