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

#include <string>
#include <vector>

#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/PageRef.h"

#include "Util.h"
#include "XournalType.h"

class Selection: public ShapeContainer {
public:
    Selection(Redrawable* view);
    virtual ~Selection();

public:
    virtual bool finalize(PageRef page) = 0;
    virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom) = 0;
    virtual void currentPos(double x, double y) = 0;
    virtual bool userTapped(double zoom) = 0;

private:
protected:
    vector<Element*> selectedElements;
    PageRef page;
    Redrawable* view;

    double x1Box;
    double x2Box;
    double y1Box;
    double y2Box;

    friend class EditSelection;
};

class RectSelection: public Selection {
public:
    RectSelection(double x, double y, Redrawable* view);
    virtual ~RectSelection();

public:
    virtual bool finalize(PageRef page);
    virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
    virtual void currentPos(double x, double y);
    virtual bool contains(double x, double y);
    virtual bool userTapped(double zoom);

private:
    double sx;
    double sy;
    double ex;
    double ey;
    double maxDist = 0;

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
    RegionSelect(double x, double y, Redrawable* view);
    virtual ~RegionSelect();

public:
    virtual bool finalize(PageRef page);
    virtual void paint(cairo_t* cr, GdkRectangle* rect, double zoom);
    virtual void currentPos(double x, double y);
    virtual bool contains(double x, double y);
    virtual bool userTapped(double zoom);

private:
    GList* points;
};
