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

#include "LayoutData.h"
#include "Redrawable.h"

#include "model/PageListener.h"
#include "model/PageRef.h"
#include "model/TexImage.h"

#include <Range.h>

#include <vector>

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

class XojPageView : public Redrawable, public PageListener
{
public:
	XojPageView(XournalView* xournal, PageRef page);
	virtual ~XojPageView();

public:
	void updatePageSize(double width, double height);

	virtual void rerenderPage();
	virtual void rerenderRect(double x, double y, double width, double height);

	virtual void repaintPage();
	virtual void repaintArea(double x1, double y1, double x2, double y2);

	void setSelected(bool selected);

	void setIsVisible(bool visible);

	bool isSelected();

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
	bool containsPoint(int x, int y, bool local = false);
	bool containsY(int y);

	GtkColorWrapper getSelectionColor();
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

	Rectangle* getVisibleRect();
	Rectangle getRect();

public: // event handler
	bool onButtonPressEvent(GtkWidget* widget, GdkEventButton* event);
	bool onButtonReleaseEvent(GtkWidget* widget, GdkEventButton* event);
	bool onMotionNotifyEvent(GtkWidget* widget, double pageX, double pageY, double pressure, bool shiftDown);
	void translateEvent(GdkEvent* event, int xOffset, int yOffset);

	/**
	 * This method actually repaints the XojPageView, triggering
	 * a rerender call if necessary
	 */
	bool paintPage(cairo_t* cr, GdkRectangle* rect);

	/**
	 * Does the painting, called in synchronized block
	 */
	void paintPageSync(cairo_t* cr, GdkRectangle* rect);

public: // listener
	void rectChanged(Rectangle& rect);
	void rangeChanged(Range &range);
	void pageChanged();
	void elementChanged(Element* elem);

private:
	void handleScrollEvent(GdkEventButton* event);

	void startText(double x, double y);
	void selectObjectAt(double x, double y);
	void playObjectAt(double x, double y);

	void addRerenderRect(double x, double y, double width, double height);

	void drawLoadingPage(cairo_t* cr);

public:
	/**
	 * position in the layout
	 */
	LayoutData layout;

private:
	XOJ_TYPE_ATTRIB;

	PageRef page;
	XournalView* xournal;
	Settings* settings;
	EraseHandler* eraser;
	InputHandler* inputHandler;

	/**
	 * The selected (while selection)
	 */
	Selection* selection;

	/**
	 * The text editor View
	 */
	TextEditor* textEditor;

	/**
	 * For keeping old text changes to undo!
	 */
	Text* oldtext;

	bool selected;

	cairo_surface_t* crBuffer;

	bool inEraser;

	/**
	 * Vertical Space
	 */
	VerticalToolHandler* verticalSpace;

	/**
	 * Search handling
	 */
	SearchControl* search;

	/**
	 * Unixtimestam when the page was last time in the visible area
	 */
	int lastVisibleTime;

	GMutex repaintRectMutex;
	std::vector<Rectangle*> rerenderRects;
	bool rerenderComplete;

	GMutex drawingMutex;

	friend class RenderJob;
	friend class InputHandler;
};
