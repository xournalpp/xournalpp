/*
 * Xournal++
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

#include <glib.h>
#include "../../model/Page.h"
#include "../../gui/Redrawable.h"
#include "../../util/Util.h"
#include "../../util/XournalType.h"

class Selection: public ShapeContainer {
public:
	Selection(Redrawable * view);
	virtual ~Selection();

public:
	virtual bool finalize(XojPage * page) = 0;
	virtual void paint(cairo_t * cr, GdkRectangle * rect, double zoom) = 0;
	virtual void currentPos(double x, double y) = 0;
	virtual void getSelectedRect(double & x, double & y, double & width, double & height);

private:
	XOJ_TYPE_ATTRIB;

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
	virtual ~RectSelection();

public:
	virtual bool finalize(XojPage * page);
	virtual void paint(cairo_t * cr, GdkRectangle * rect, double zoom);
	virtual void currentPos(double x, double y);
	virtual bool contains(double x, double y);

private:
	XOJ_TYPE_ATTRIB;

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
	virtual ~RegionSelect();

public:
	virtual bool finalize(XojPage * page);
	virtual void paint(cairo_t * cr, GdkRectangle * rect, double zoom);
	virtual void currentPos(double x, double y);
	virtual bool contains(double x, double y);

private:
	XOJ_TYPE_ATTRIB;

	GList * points;
};

#endif /* __SELECTION_H__ */
