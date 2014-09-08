/*
 * Xournal++
 *
 * Displays a single page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PAGEVIEW_H__
#define __PAGEVIEW_H__

#include <gtk/gtk.h>
#include <Range.h>
#include "Redrawable.h"
#include "../model/PageRef.h"
#include "LayoutData.h"
#include "../model/TexImage.h"
#include "../model/PageListener.h"

class XournalView;
class EditSelection;
class Selection;
class TextEditor;
class EraseHandler;
class VerticalToolHandler;
class SearchControl;
class Settings;
class InputHandler;

class Text;

/**
 * @brief A widget displaying a single XojPage of a given Document
 * 
 * The PageView contains an internal buffer to speed up the rendering
 * of its page
 */
class PageView: public Redrawable, public PageListener
{
public:
	PageView(XournalView* xournal, PageRef page);
	virtual ~PageView();

public:
	void updatePageSize(double width, double height);

	virtual void rerenderPage();
	virtual void rerenderRect(double x, double y, double width, double heigth);

	virtual void repaintPage();
	virtual void repaintArea(double x1, double y1, double x2, double y2);

	/**
	 * Marks the PageView as selected.
	 * There should always be exactly one selected PageView
	 */
	void setSelected(bool selected);
	bool isSelected();

	/**
	 * Marks the PageView to be visible.
	 * Used by the Layout
	 */
	void setIsVisible(bool visible);

	/**
	 * Returns whether this PageView is visible
	 */
	bool isVisible();

	void endText();

	bool searchTextOnPage(const char* text, int* occures, double* top);

	bool onKeyPressEvent(GdkEventKey* event);
	bool onKeyReleaseEvent(GdkEventKey* event);

	bool cut();
	bool copy();
	bool paste();

	bool actionDelete();

	void resetShapeRecognizer();

	/**
	 * Deletes the view buffer freeing its memory
	 * Assumes that the calling thread holds the drawing mutex
	 */
	void deleteViewBuffer();

	/**
	 * Creates a new view buffer with the specified size
	 * Assumes that the calling thread holds the drawing mutex
	 */
	void createViewBuffer(int width, int height);

	/**
	 * Replaces the current view buffer by the specified one
	 * Assumes that the calling thread holds the drawing mutex
	 */
	void setViewBuffer(cairo_surface_t* buffer);

	/**
	 * Returns this PageView%s interal buffer
	 * You should hold the drawing mutex in this case
	 */
	cairo_surface_t* getViewBuffer()
	{
		return this->crBuffer;
	}

	/**
	 * The drawing mutex protects this PageView%s internal buffer
	 */
	GMutex* getDrawingMutex()
	{
		return &this->drawingMutex;
	}

	/**
	 * The repaint mutex protectes this PageView%s collection
	 * of repainting rectangles
	 */
	GMutex* getRepaintMutex()
	{
		return &this->repaintRectMutex;
	}

	/**
	 * Returns whether this PageView contains the
	 * given point on the display
	 */
	bool containsPoint(int x, int y, bool local = false);
	bool containsY(int y);

	GdkRGBA getSelectionColor();

	/**
	 * Returns the number of pixels in this PageView%s internal buffer
	 */
	int getBufferPixels();

	/**
	 * Returns the size (in bytes) of this PageView%s interal buffer
	 */
	int getBufferSize() const;

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
	 * Returns the height of this PageView
	 */
	double getHeight() const;

	/**
	 * Returns the width of this PageView as assigned
	 * by the Layout
	 */
	int getDisplayWidth() const;
	/**
	 * Returns the height of this PageView as assigned
	 * by the Layout
	 */
	int getDisplayHeight() const;

	/**
	 * Returns the x coordinate of this PageView as
	 * assigned by the Layout
	 */
	int getX() const;

	/**
	 * Returns the y coordinate of this PageView as
	 * assigned by the Layout
	 */
	int getY() const;

	/**
	 * Maps a Rectangle from display coordinates to local
	 * coordinates
	 */
	virtual Rectangle* rectOnWidget(double x, double y, double width,
	                                double height);

	TexImage* getSelectedTex();

	Rectangle* getVisibleRect();
	Rectangle getRect();

public: // event handler
	bool onButtonPressEvent(GtkWidget* widget, GdkEventButton* event);
	bool onButtonReleaseEvent(GtkWidget* widget, GdkEventButton* event);
	bool onMotionNotifyEvent(GtkWidget* widget, GdkEventMotion* event);
	void translateEvent(GdkEvent* event, int xOffset, int yOffset);

	/**
	 * This method actually repaints the PageView, triggering
	 * a rerender call if necessary
	 */
	bool paintPage(cairo_t* cr, GdkRectangle* rect);

public: // listener
	void rectChanged(Rectangle& rect);
	void rangeChanged(Range &range);
	void pageChanged();
	void elementChanged(Element* elem);

private:
	void handleScrollEvent(GdkEventButton* event);

	void startText(double x, double y);
	void selectObjectAt(double x, double y);

	void addRerenderRect(double x, double y, double width, double height);

public:
	// position in the layout
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

	//For keeping old text changes to undo!
	Text* oldtext;

	bool selected;

	cairo_surface_t* crBuffer;

	bool inEraser;

	// Vertical Space
	VerticalToolHandler* verticalSpace;

	/**
	 * Search handling
	 */
	SearchControl* search;

	bool visible;

	GMutex repaintRectMutex;
	GList * rerenderRects;
	bool rerenderComplete;

	GMutex drawingMutex;

	friend class InsertImageRunnable;
	friend class RenderJob;
	friend class InputHandler;
};

#endif /* __PAGEVIEW_H__ */
