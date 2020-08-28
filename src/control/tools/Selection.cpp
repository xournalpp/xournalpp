#include "Selection.h"

#include <cmath>

#include "model/Layer.h"

Selection::Selection(Redrawable* view) {
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

RectSelection::RectSelection(double x, double y, Redrawable* view): Selection(view) {
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
    for (Element* e: *l->getElements()) {
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

void RectSelection::paint(cairo_t* cr, GdkRectangle* rect, double zoom) {
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

RegionSelect::RegionSelect(double x, double y, Redrawable* view): Selection(view) {
    this->points = nullptr;
    currentPos(x, y);
}

RegionSelect::~RegionSelect() {
    for (GList* l = this->points; l != nullptr; l = l->next) {
        delete static_cast<RegionPoint*>(l->data);
    }
    g_list_free(this->points);
}

void RegionSelect::paint(cairo_t* cr, GdkRectangle* rect, double zoom) {
    // at least three points needed
    if (this->points && this->points->next && this->points->next->next) {
        GdkRGBA selectionColor = view->getSelectionColor();

        // set the line always the same size on display
        cairo_set_line_width(cr, 1 / zoom);
        gdk_cairo_set_source_rgba(cr, &selectionColor);

        auto* r0 = static_cast<RegionPoint*>(this->points->data);
        cairo_move_to(cr, r0->x, r0->y);

        for (GList* l = this->points->next; l != nullptr; l = l->next) {
            auto* r = static_cast<RegionPoint*>(l->data);
            cairo_line_to(cr, r->x, r->y);
        }

        cairo_line_to(cr, r0->x, r0->y);

        cairo_stroke_preserve(cr);
        auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
        gdk_cairo_set_source_rgba(cr, &applied);
        cairo_fill(cr);
    }
}

void RegionSelect::currentPos(double x, double y) {
    this->points = g_list_append(this->points, new RegionPoint(x, y));

    // at least three points needed
    if (this->points && this->points->next && this->points->next->next) {

        auto* r0 = static_cast<RegionPoint*>(this->points->data);
        double ax = r0->x;
        double bx = r0->x;
        double ay = r0->y;
        double by = r0->y;

        for (GList* l = this->points; l != nullptr; l = l->next) {
            auto* r = static_cast<RegionPoint*>(l->data);
            if (ax > r->x) {
                ax = r->x;
            }
            if (bx < r->x) {
                bx = r->x;
            }
            if (ay > r->y) {
                ay = r->y;
            }
            if (by < r->y) {
                by = r->y;
            }
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
    if (this->points == nullptr || this->points->next == nullptr) {
        return false;
    }

    int hits = 0;

    auto* last = static_cast<RegionPoint*>(g_list_last(this->points)->data);

    double lastx = last->x;
    double lasty = last->y;
    double curx = NAN, cury = NAN;

    // Walk the edges of the polygon
    for (GList* l = this->points; l != nullptr; lastx = curx, lasty = cury, l = l->next) {
        auto* last = static_cast<RegionPoint*>(l->data);
        curx = last->x;
        cury = last->y;

        if (cury == lasty) {
            continue;
        }

        int leftx = 0;
        if (curx < lastx) {
            if (x >= lastx) {
                continue;
            }
            leftx = curx;
        } else {
            if (x >= curx) {
                continue;
            }
            leftx = lastx;
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

    this->x1Box = 0;
    this->x2Box = 0;
    this->y1Box = 0;
    this->y2Box = 0;

    for (GList* l = this->points; l != nullptr; l = l->next) {
        auto* p = static_cast<RegionPoint*>(l->data);

        if (p->x < this->x1Box) {
            this->x1Box = p->x;
        } else if (p->x > this->x2Box) {
            this->x2Box = p->x;
        }

        if (p->y < this->y1Box) {
            this->y1Box = p->y;
        } else if (p->y > this->y2Box) {
            this->y2Box = p->y;
        }
    }

    Layer* l = page->getSelectedLayer();
    for (Element* e: *l->getElements()) {
        if (e->isInSelection(this)) {
            this->selectedElements.push_back(e);
        }
    }

    view->repaintArea(this->x1Box - 10, this->y1Box - 10, this->x2Box + 10, this->y2Box + 10);

    return !this->selectedElements.empty();
}

auto RegionSelect::userTapped(double zoom) -> bool {
    double maxDist = 10 / zoom;
    auto* r0 = static_cast<RegionPoint*>(this->points->data);
    for (GList* l = this->points; l != nullptr; l = l->next) {
        auto* p = static_cast<RegionPoint*>(l->data);
        if (r0->x - p->x > maxDist || p->x - r0->x > maxDist || r0->y - p->y > maxDist || p->y - r0->y > maxDist) {
            return false;
        }
    }
    return true;
}
