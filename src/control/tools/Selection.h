/*
 * Xournal++
 *
 * A selection while you are selection, not for editing, only for selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>
#include "model/PageRef.h"
#include "model/Element.h"
#include "gui/Redrawable.h"
#include <Util.h>
#include <XournalType.h>

class Selection : public ShapeContainer
{
public:
	Selection(Redrawable* view);
	virtual ~Selection();

public:
	virtual bool finalize(PageRef page) = 0;
	virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom) = 0;
	virtual void currentPos(double x, double y) = 0;
	virtual void getSelectedRect(double& x, double& y, double& width,
								double& height);

private:
	XOJ_TYPE_ATTRIB;

protected:
	ElementVector selectedElements;
	PageRef page;
	Redrawable* view;

	double x1Box;
	double x2Box;
	double y1Box;
	double y2Box;

	friend class EditSelection;
};

class RectSelection : public Selection
{
public:
	RectSelection(double x, double y, Redrawable* view);
	virtual ~RectSelection();

public:
	virtual bool finalize(PageRef page);
	virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
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

class RegionSelect : public Selection
{
public:
	RegionSelect(double x, double y, Redrawable* view);
	virtual ~RegionSelect();

public:
	virtual bool finalize(PageRef page);
	virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
	virtual void currentPos(double x, double y);
	virtual bool contains(double x, double y);

private:
	XOJ_TYPE_ATTRIB;

	GList* points;
};
