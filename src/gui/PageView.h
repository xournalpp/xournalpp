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

#include "gui/inputdevices/PositionInputData.h"
#include "model/PageListener.h"
#include "model/PageRef.h"
#include "model/TexImage.h"

#include "Layout.h"
#include "Range.h"
#include "Redrawable.h"

class EditSelection;
class EraseHandler;
class InputHandler;
class SearchControl;
class Selection;
class Settings;
class Text;
class TextEditor;
class VerticalToolHandler;
class XournalView;

class XojPageView: public Redrawable, public PageListener {
public:
    XojPageView(XournalView* xournal, const PageRef& page);
    virtual ~XojPageView();

public:
    void updatePageSize(double width, double height);

    virtual void rerenderPage();
    virtual void rerenderRect(double x, double y, double width, double height);

    virtual void repaintPage();
    virtual void repaintArea(double x1, double y1, double x2, double y2);

    void setSelected(bool selected);

    void setIsVisible(bool visible);

    bool isSelected() const;

    void endText();

    bool searchTextOnPage(string& text, int* occures, double* top);

    bool onKeyPressEvent(GdkEventKey* event);
    bool onKeyReleaseEvent(GdkEventKey* event);

    bool cut();
    bool copy();
    bool paste();

    bool actionDelete();

    void resetShapeRecognizer();

    void deleteViewBuffer();

    /**
     * Returns whether this PageView contains the
     * given point on the display
     */
    bool containsPoint(int x, int y, bool local = false) const;
    bool containsY(int y) const;

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
    PageRef getPage();

    XournalView* getXournal();

    /**
     * Returns the width of this PageView
     */
    double getWidth() const;

    /**
     * Returns the height of this XojPageView
     */
    double getHeight() const;

    /**
     * Returns the width of this XojPageView as displayed
     * on the display taking into account the current zoom
     */
    int getDisplayWidth() const;
    /**
     * Returns the height of this XojPageView as displayed
     * on the display taking into account the current zoom
     */
    int getDisplayHeight() const;

    /**
     * Returns the x coordinate of this XojPageView with
     * respect to the display
     */
    int getX() const;

    /**
     * Returns the y coordinate of this XojPageView with
     * respect to the display
     */
    int getY() const;

    TexImage* getSelectedTex();
    Text* getSelectedText();

    Rectangle<double> getRect() const;

public:  // event handler
    bool onButtonPressEvent(const PositionInputData& pos);
    bool onButtonReleaseEvent(const PositionInputData& pos);
    bool onButtonDoublePressEvent(const PositionInputData& pos);
    bool onButtonTriplePressEvent(const PositionInputData& pos);
    bool onMotionNotifyEvent(const PositionInputData& pos);

    /**
     * This method actually repaints the XojPageView, triggering
     * a rerender call if necessary
     */
    bool paintPage(cairo_t* cr, GdkRectangle* rect);

    /**
     * Does the painting, called in synchronized block
     */
    void paintPageSync(cairo_t* cr, GdkRectangle* rect);

public:  // listener
    void rectChanged(Rectangle<double>& rect);
    void rangeChanged(Range& range);
    void pageChanged();
    void elementChanged(Element* elem);

private:
    void handleScrollEvent(GdkEventButton* event);

    void startText(double x, double y);

    void addRerenderRect(double x, double y, double width, double height);

    void drawLoadingPage(cairo_t* cr);

    void setX(int x);
    void setY(int y);

    void setMappedRowCol(int row, int col);  // row, column assigned by mapper during layout.


private:
    PageRef page;
    XournalView* xournal;
    Settings* settings;
    EraseHandler* eraser = nullptr;
    InputHandler* inputHandler = nullptr;

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

    cairo_surface_t* crBuffer = nullptr;

    bool inEraser = false;

    /**
     * Vertical Space
     */
    VerticalToolHandler* verticalSpace = nullptr;

    /**
     * Search handling
     */
    SearchControl* search = nullptr;

    /**
     * Unixtimestam when the page was last time in the visible area
     */
    int lastVisibleTime = -1;

    GMutex repaintRectMutex{};
    vector<Rectangle<double>> rerenderRects;
    bool rerenderComplete = false;

    GMutex drawingMutex{};

    int dispX{};  // position on display - set in Layout::layoutPages
    int dispY{};


    int mappedRow{};
    int mappedCol{};


    friend class RenderJob;
    friend class InputHandler;
    friend class BaseSelectObject;
    friend class SelectObject;
    friend class PlayObject;
    // only function allowed to setX(), setY(), setMappedRowCol():
    friend void Layout::layoutPages(int width, int height);
};
