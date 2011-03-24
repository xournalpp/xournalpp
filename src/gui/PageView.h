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
#include "../util/Range.h"
#include "Redrawable.h"

class XournalView;
class DeleteUndoAction;
class Selection;
class TextEditor;
class EraseHandler;
class VerticalToolHandler;
class SearchControl;
class Settings;
class InputHandler;

// Model
class Page;
class Stroke;
class Text;
class XojPage;

class PageView: public Redrawable {
public:
	PageView(XournalView * xournal, XojPage * page);
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

	void setPos(int x, int y);

	bool containsPoint(int x, int y);
	bool containsY(int y);

	GdkColor getSelectionColor();
	int getBufferPixels();
	int getLastVisibelTime();
	TextEditor * getTextEditor();
	XojPage * getPage();
	XournalView * getXournal();
	double getHeight();
	double getWidth();
	int getDisplayWidth();
	int getDisplayHeight();
	int getX();
	int getY();

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

private:
	XOJ_TYPE_ATTRIB;

	XojPage * page;
	XournalView * xournal;
	Settings * settings;
	EraseHandler * eraser;
	InputHandler * inputHandler;

	// position in px on screen
	int x;
	int y;

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
	GList * repaintRects;
	bool rerenderComplete;

	GMutex * drawingMutex;

	friend class InsertImageRunnable;
	friend class RenderJob;
};

#endif /* __PAGEVIEW_H__ */
