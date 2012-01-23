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

class XournalView;
class Selection;
class TextEditor;
class EraseHandler;
class VerticalToolHandler;
class SearchControl;
class Settings;
class InputHandler;

class PageView: public Redrawable {
public:
	PageView(XournalView * xournal, PageRef page);
	virtual ~PageView();

public:
	void updatePageSize(double width, double height);

	virtual void rerenderPage();
	virtual void rerenderRect(double x, double y, double width, double heigth);

	virtual void repaintPage();
	virtual void repaintArea(double x1, double y1, double x2, double y2);

	void setSelected(bool selected);

	void setIsVisibel(bool visibel);

	bool isSelected();

	void endText();

	bool searchTextOnPage(const char * text, int * occures, double * top);

	bool onKeyPressEvent(GdkEventKey *event);
	bool onKeyReleaseEvent(GdkEventKey *event);

	bool cut();
	bool copy();
	bool paste();

	bool actionDelete();

	void resetShapeRecognizer();

	void deleteViewBuffer();

	bool containsPoint(int x, int y);
	bool containsY(int y);

	GdkColor getSelectionColor();
	int getBufferPixels();
	int getLastVisibelTime();
	TextEditor * getTextEditor();
	PageRef getPage();
	XournalView * getXournal();
	double getHeight();
	double getWidth();
	int getDisplayWidth();
	int getDisplayHeight();
	int getX();
	int getY();

	virtual Rectangle * rectOnWidget(double x, double y, double width, double height);

public: // event handler
	bool onButtonPressEvent(GtkWidget * widget, GdkEventButton * event);
	bool onButtonReleaseEvent(GtkWidget * widget, GdkEventButton * event);
	bool onMotionNotifyEvent(GtkWidget * widget, GdkEventMotion * event);
	void translateEvent(GdkEvent * event, int xOffset, int yOffset);
	bool paintPage(cairo_t * cr, GdkRectangle * rect);

private:
	void handleScrollEvent(GdkEventButton * event);

	void startText(double x, double y);
	void selectObjectAt(double x, double y);

	void addRerenderRect(double x, double y, double width, double height);

public:
	// position in the layout
	LayoutData layout;

private:
	XOJ_TYPE_ATTRIB;

	PageRef page;
	XournalView * xournal;
	Settings * settings;
	EraseHandler * eraser;
	InputHandler * inputHandler;

	/**
	 * The selected (while selection)
	 */
	Selection * selection;

	/**
	 * The text editor View
	 */
	TextEditor * textEditor;

	bool selected;

	cairo_surface_t * crBuffer;

	bool inEraser;

	bool extendedWarningDisplayd;

	// Vertical Space
	VerticalToolHandler * verticalSpace;

	/**
	 * Search handling
	 */
	SearchControl * search;

	/**
	 * Unixtimestam when the page was last time in the visible area
	 */
	int lastVisibelTime;

	GMutex * repaintRectMutex;
	GList * rerenderRects;
	bool rerenderComplete;

	GMutex * drawingMutex;

	friend class InsertImageRunnable;
	friend class RenderJob;
};

#endif /* __PAGEVIEW_H__ */
