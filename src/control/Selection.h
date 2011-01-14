/*
 * Xournal Extended
 *
 * A selection while you are selection, not for editing, only for selection
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
#include "../util/Util.h"

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

#endif /* __SELECTION_H__ */
