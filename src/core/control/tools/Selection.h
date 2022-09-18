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

#include <vector>  // for vector

#include <cairo.h>  // for cairo_t

#include "model/Element.h"  // for Element (ptr only), ShapeContainer
#include "model/PageRef.h"  // for PageRef

class LegacyRedrawable;


class Selection: public ShapeContainer {
public:
    Selection(LegacyRedrawable* view);
    ~Selection() override;

public:
    virtual bool finalize(PageRef page) = 0;
    virtual void paint(cairo_t* cr, double zoom) = 0;
    virtual void currentPos(double x, double y) = 0;
    virtual bool userTapped(double zoom) = 0;

private:
protected:
    std::vector<Element*> selectedElements;
    PageRef page;
    LegacyRedrawable* view;

    double x1Box;
    double x2Box;
    double y1Box;
    double y2Box;

    friend class EditSelection;
};

class RectSelection: public Selection {
public:
    RectSelection(double x, double y, LegacyRedrawable* view);
    ~RectSelection() override;

public:
    bool finalize(PageRef page) override;
    void paint(cairo_t* cr, double zoom) override;
    void currentPos(double x, double y) override;
    bool contains(double x, double y) override;
    bool userTapped(double zoom) override;

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

class RegionPoint;

class RegionSelect: public Selection {
public:
    RegionSelect(double x, double y, LegacyRedrawable* view);

public:
    bool finalize(PageRef page) override;
    void paint(cairo_t* cr, double zoom) override;
    void currentPos(double x, double y) override;
    bool contains(double x, double y) override;
    bool userTapped(double zoom) override;

private:
    std::vector<RegionPoint> points;
};
