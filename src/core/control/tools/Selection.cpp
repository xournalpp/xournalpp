#include "Selection.h"

#include <algorithm>  // for max, min
#include <cfloat>     // for DBL_MAX
#include <cmath>      // for abs, NAN
#include <memory>     // for __shared_ptr_access

#include <gdk/gdk.h>  // for GdkRGBA, gdk_cairo_set_source_rgba

#include "gui/LegacyRedrawable.h"  // for Redrawable
#include "model/Layer.h"     // for Layer
#include "model/XojPage.h"   // for XojPage

Selection::Selection(LegacyRedrawable* view) {
    this->view = view;
    this->page = nullptr;

    this->x1Box = 0;
    this->x2Box = 0;
    this->y1Box = 0;
    this->y2Box = 0;
}

Selection::~Selection() {
    this->view = nullptr;
    this->page = nullptr;
}

//////////////////////////////////////////////////////////

RectSelection::RectSelection(double x, double y, LegacyRedrawable* view): Selection(view) {
    this->sx = x;
    this->sy = y;
    this->ex = x;
    this->ey = y;
    this->x1 = 0;
    this->x2 = 0;
    this->y1 = 0;
    this->y2 = 0;
}

RectSelection::~RectSelection() = default;

auto RectSelection::finalize(PageRef page) -> bool {
    this->x1 = std::min(this->sx, this->ex);
    this->x2 = std::max(this->sx, this->ex);

    this->y1 = std::min(this->sy, this->ey);
    this->y2 = std::max(this->sy, this->ey);

    this->page = page;

    Layer* l = page->getSelectedLayer();
    for (Element* e: l->getElements()) {
        if (e->isInSelection(this)) {
            this->selectedElements.push_back(e);
        }
    }

    view->repaintArea(this->x1 - 10, this->y1 - 10, this->x2 + 10, this->y2 + 10);

    return !this->selectedElements.empty();
}

auto RectSelection::contains(double x, double y) -> bool {
    if (x < this->x1 || x > this->x2) {
        return false;
    }
    if (y < this->y1 || y > this->y2) {
        return false;
    }

    return true;
}

void RectSelection::currentPos(double x, double y) {
    double aX = std::min(x, this->ex);
    aX = std::min(aX, this->sx) - 10;

    double bX = std::max(x, this->ex);
    bX = std::max(bX, this->sx) + 10;

    double aY = std::min(y, this->ey);
    aY = std::min(aY, this->sy) - 10;

    double bY = std::max(y, this->ey);
    bY = std::max(bY, this->sy) + 10;

    view->repaintArea(aX, aY, bX, bY);

    this->ex = x;
    this->ey = y;

    this->maxDist = std::max({this->maxDist, x - this->sx, this->sx - x, y - this->sy, this->sy - y});
}

auto RectSelection::userTapped(double zoom) -> bool { return this->maxDist < 10 / zoom; }

void RectSelection::paint(cairo_t* cr, double zoom) {
    GdkRGBA selectionColor = view->getSelectionColor();

    // set the line always the same size on display
    cairo_set_line_width(cr, 1 / zoom);
    gdk_cairo_set_source_rgba(cr, &selectionColor);

    int aX = std::min(this->sx, this->ex);
    int bX = std::max(this->sx, this->ex);

    int aY = std::min(this->sy, this->ey);
    int bY = std::max(this->sy, this->ey);

    cairo_move_to(cr, aX, aY);
    cairo_line_to(cr, bX, aY);
    cairo_line_to(cr, bX, bY);
    cairo_line_to(cr, aX, bY);
    cairo_close_path(cr);

    cairo_stroke_preserve(cr);
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
    gdk_cairo_set_source_rgba(cr, &applied);
    cairo_fill(cr);
}

//////////////////////////////////////////////////////////

class RegionPoint {
public:
    RegionPoint(double x, double y) {
        this->x = x;
        this->y = y;
    }

    double x;
    double y;
};

RegionSelect::RegionSelect(double x, double y, LegacyRedrawable* view): Selection(view) { currentPos(x, y); }

void RegionSelect::paint(cairo_t* cr, double zoom) {
    // at least three points needed
    if (points.size() >= 3) {
        GdkRGBA selectionColor = view->getSelectionColor();

        // set the line always the same size on display
        cairo_set_line_width(cr, 1 / zoom);
        gdk_cairo_set_source_rgba(cr, &selectionColor);

        const RegionPoint& r0 = points.front();
        cairo_move_to(cr, r0.x, r0.y);

        for (auto pointIterator = points.begin() + 1; pointIterator != points.end(); ++pointIterator) {
            cairo_line_to(cr, pointIterator->x, pointIterator->y);
        }

        cairo_line_to(cr, r0.x, r0.y);

        cairo_stroke_preserve(cr);
        auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    }
}

void RegionSelect::currentPos(double x, double y) {
    points.emplace_back(x, y);

    // at least three points needed
    if (points.size() >= 3) {
        const RegionPoint& r0 = points.front();
        double ax = r0.x;
        double bx = r0.x;
        double ay = r0.y;
        double by = r0.y;

        for (const RegionPoint& p: points) {
            ax = std::min(ax, p.x);
            bx = std::max(bx, p.x);
            ay = std::min(ay, p.y);
            by = std::max(by, p.y);
        }

        view->repaintArea(ax, ay, bx, by);
    }
}

auto RegionSelect::contains(double x, double y) -> bool {
    if (x < this->x1Box || x > this->x2Box) {
        return false;
    }
    if (y < this->y1Box || y > this->y2Box) {
        return false;
    }
    if (points.size() <= 2) {
        return false;
    }

    int hits = 0;

    const RegionPoint& last = points.back();

    double lastx = last.x;
    double lasty = last.y;
    double curx = NAN, cury = NAN;

    // Walk the edges of the polygon
    for (auto pointIterator = points.begin(); pointIterator != points.end();
         lastx = curx, lasty = cury, ++pointIterator) {
        curx = pointIterator->x;
        cury = pointIterator->y;

        if (cury == lasty) {
            continue;
        }

        int leftx = 0;
        if (curx < lastx) {
            if (x >= lastx) {
                continue;
            }
            leftx = static_cast<int>(curx);
        } else {
            if (x >= curx) {
                continue;
            }
            leftx = static_cast<int>(lastx);
        }

        double test1 = NAN, test2 = NAN;
        if (cury < lasty) {
            if (y < cury || y >= lasty) {
                continue;
            }
            if (x < leftx) {
                hits++;
                continue;
            }
            test1 = x - curx;
            test2 = y - cury;
        } else {
            if (y < lasty || y >= cury) {
                continue;
            }
            if (x < leftx) {
                hits++;
                continue;
            }
            test1 = x - lastx;
            test2 = y - lasty;
        }

        if (test1 < (test2 / (lasty - cury) * (lastx - curx))) {
            hits++;
        }
    }

    return (hits & 1) != 0;
}

auto RegionSelect::finalize(PageRef page) -> bool {
    this->page = page;

    this->x1Box = DBL_MAX;
    this->x2Box = 0;
    this->y1Box = DBL_MAX;
    this->y2Box = 0;

    for (const RegionPoint& p: points) {
        this->x1Box = std::min(this->x1Box, p.x);
        this->x2Box = std::max(this->x2Box, p.x);
        this->y1Box = std::min(this->y1Box, p.y);
        this->y2Box = std::max(this->y2Box, p.y);
    }

    Layer* l = page->getSelectedLayer();
    for (Element* e: l->getElements()) {
        if (e->isInSelection(this)) {
            this->selectedElements.push_back(e);
        }
    }

    view->repaintArea(this->x1Box - 10, this->y1Box - 10, this->x2Box + 10, this->y2Box + 10);

    return !this->selectedElements.empty();
}

auto RegionSelect::userTapped(double zoom) -> bool {
    double maxDist = 10 / zoom;
    const RegionPoint& r0 = points.front();
    for (const RegionPoint& p: points) {
        if (std::abs(r0.x - p.x) > maxDist || std::abs(r0.y - p.y) > maxDist) {
            return false;
        }
    }
    return true;
}
