/*
 * Xournal Extended
 *
 * Xournal Settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SELECTION_H__
#define __SELECTION_H__

#include <gtk/gtk.h>
#include "../model/Page.h"
#include "Redrawable.h"

enum CursorSelectionType {
	CURSOR_SELECTION_NONE = 0,
	//Inside the selection
	CURSOR_SELECTION_MOVE = 1,
	// Edges
	CURSOR_SELECTION_TOP_LEFT,
	CURSOR_SELECTION_TOP_RIGHT,
	CURSOR_SELECTION_BOTTOM_LEFT,
	CURSOR_SELECTION_BOTTOM_RIGHT,
	// Sides
	CURSOR_SELECTION_LEFT,
	CURSOR_SELECTION_RIGHT,
	CURSOR_SELECTION_TOP,
	CURSOR_SELECTION_BOTTOM,
};

class Selection: public ShapeContainer {
public:
	Selection(Redrawable * view);
	virtual ~Selection();

	virtual bool finnalize(XojPage * page) = 0;
	virtual void paint(cairo_t * cr, GdkEventExpose *event, double zoom) = 0;
	virtual void currentPos(double x, double y) = 0;
	virtual void getSelectedRect(double & x, double & y, double & width, double & height) = 0;
protected:
	GList * selectedElements;
	XojPage * page;
	Redrawable * view;

	double x1Box;
	double x2Box;
	double y1Box;
	double y2Box;

	friend class EditSelection;
};

class RectSelection: public Selection {
public:
	RectSelection(double x, double y, Redrawable * view);

	virtual bool finnalize(XojPage * page);
	virtual void paint(cairo_t * cr, GdkEventExpose *event, double zoom);
	virtual void currentPos(double x, double y);
	virtual void getSelectedRect(double & x, double & y, double & width, double & height);
	virtual bool contains(double x, double y);

private:
	double sx;
	double sy;
	double ex;
	double ey;

	/**
	 * In zoom coordinates
	 */
	double x1;
	double x2;
	double y1;
	double y2;
};

class RegionSelect: public Selection {
public:
	RegionSelect(double x, double y, Redrawable * view);
	~RegionSelect();
	virtual bool finnalize(XojPage * page);
	virtual void paint(cairo_t * cr, GdkEventExpose *event, double zoom);
	virtual void currentPos(double x, double y);
	virtual void getSelectedRect(double & x, double & y, double & width, double & height);
	virtual bool contains(double x, double y);
private:
	GList * points;
};

class PageView;
class XournalWidget;

class EditSelection {
public:
	EditSelection(Selection * selection, Redrawable * view);
	EditSelection(Element * e, Redrawable * view, XojPage * page);
	~EditSelection();
	void paint(cairo_t * cr, GdkEventExpose *event, double zoom);

	CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom);
	void setEditMode(CursorSelectionType selType, double x, double y);
	void move(double x, double y, PageView * view, XournalWidget * xournal);
	CursorSelectionType getEditMode();
	void finalizeEditing();

private:
	void drawAnchorRect(cairo_t * cr, double x, double y, double zoom);
private:
	double x;
	double y;
	double width;
	double height;

	CursorSelectionType selType;
	double selX;
	double selY;

	Redrawable * view;

	GList * selected;
	XojPage * page;
};

#endif /* __SELECTION_H__ */
