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

#include "../model/Page.h"
#include "../model/Stroke.h"
#include "../model/Text.h"

#include "../control/Settings.h"
#include "../control/Selection.h"
#include "../control/SearchControl.h"
#include "../view/DocumentView.h"
#include "../gui/TextEditor.h"


class XournalWidget;
class DeleteUndoAction;
class EraseUndoAction;

class PageView : public Redrawable {
public:
	PageView(XournalWidget * xournal, XojPage * page);
	virtual ~PageView();

	double getHeight();
	double getWidth();

	int getDisplayWidth();
	int getDisplayHeight();

	void updatePageSize(double width, double height);

	void firstPaint();
	bool paintPage(GtkWidget *widget, GdkEventExpose *event);

	void repaint();
	void repaint(int x, int y, int width, int heigth);

	void updateSize();

	XojPage * getPage();

	GtkWidget * getWidget();

	XournalWidget * getXournal();

	void setSelected(bool selected);
	void updateXEvents();

	virtual void deleteViewBuffer();

	bool isSelected();

	void clearSelection();
	void endText();

	bool searchTextOnPage(const char * text, int * occures, double * top);

	bool onKeyPressEvent(GdkEventKey *event);
	bool onKeyReleaseEvent(GdkEventKey *event);

	void cut();
	void copy();
	void paste();

	void actionDelete();
public: // Redrawable
	void redrawDocumentRegion(double x1, double y1, double x2, double y2);
	GdkColor getSelectionColor();
private:
	static gboolean exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, PageView * page);
	static gboolean onButtonPressEventCallback(GtkWidget *widget, GdkEventButton *event, PageView * view);
	void onButtonPressEvent(GtkWidget *widget, GdkEventButton *event);
	static bool onButtonReleaseEventCallback(GtkWidget *widget, GdkEventButton *event, PageView * view);
	bool onButtonReleaseEvent(GtkWidget *widget, GdkEventButton *event);
	static gboolean onMotionNotifyEventCallback(GtkWidget *widget, GdkEventMotion *event, PageView * view);
	gboolean onMotionNotifyEvent(GtkWidget *widget, GdkEventMotion *event);

	void handleScrollEvent(GdkEventButton *event);

	void addPointToTmpStroke(GdkEventMotion *event);
	bool getPressureMultiplier(GdkEvent *event, double & presure);

	void doErase(double x, double y);
	void startText(double x, double y);
	void selectObjectOn(double x, double y);

	void doScroll(GdkEventMotion *event);

	void drawTmpStroke();
	void repaintLater();
	static bool repaintCallback(PageView * view);
private:
	XojPage * page;
	GtkWidget * widget;
	XournalWidget * xournal;
	Settings * settings;

	/**
	 * If you are drawing on the document
	 */
	Stroke * tmpStroke;

	/**
	 * What has already be drawed, only draw the new part
	 */
	int tmpStrokeDrawElem;

	/**
	 * The selected (while selection)
	 */
	Selection * selectionEdit;

	/**
	 * Selected content
	 */
	EditSelection * selection;

	/**
	 * The last Mouse Position, for scrolling
	 */
	double lastMousePositionX;
	double lastMousePositionY;

	bool inScrolling;

	/**
	 * The View to draw the page
	 */
	DocumentView * view;

	/**
	 * The text editor View
	 */
	TextEditor * textEditor;

	/**
	 * The current input device for stroken, do not react on other devices (linke mices)
	 */
	GdkDevice * currentInputDevice;

	bool firstPainted;
	bool selected;

	cairo_surface_t * crBuffer;
	int idleRepaintId;

	bool inEraser;
	DeleteUndoAction * eraseDeleteUndoAction;
	EraseUndoAction * eraseUndoAction;

	bool extendedWarningDisplayd;

	/**
	 * Search handling
	 */
	SearchControl * search;
};

#endif /* __PAGEVIEW_H__ */
