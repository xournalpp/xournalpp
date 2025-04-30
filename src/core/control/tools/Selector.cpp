#include "Selector.h"

#include <algorithm>  // for max, min
#include <cmath>      // for abs, NAN
#include <memory>     // for __shared_ptr_access

#include <gdk/gdk.h>  // for GdkRGBA, gdk_cairo_set_source_rgba

#include "gui/LegacyRedrawable.h"  // for Redrawable
#include "model/Document.h"        // for Document
#include "model/Layer.h"           // for Layer
#include "model/XojPage.h"         // for XojPage
#include "util/safe_casts.h"       // for as_unsigned

Selector::Selector(bool multiLayer):
        multiLayer(multiLayer), viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SelectorView>>()) {}

Selector::~Selector() = default;

auto Selector::finalize(PageRef page, bool disableMultilayer, Document* doc) -> size_t {
    this->page = page;
    size_t layerId = 0;

    if (multiLayer && !disableMultilayer) {
        std::lock_guard lock(*doc);
        const auto layers = page->getLayers();
        for (auto it = layers.rbegin(); it != layers.rend(); it++) {
            Layer* l = *it;
            if (!l->isVisible()) {
                continue;
            }
            bool selectionOnLayer = false;
            Element::Index pos = 0;
            for (auto&& e: l->getElements()) {
                if (e->isInSelection(this)) {
                    this->selectedElements.emplace_back(e.get(), pos);
                    selectionOnLayer = true;
                }
                pos++;
            }
            if (selectionOnLayer) {
                layerId = layers.size() - as_unsigned(std::distance(layers.rbegin(), it));
                break;
            }
        }
    } else {
        std::lock_guard lock(*doc);
        Layer* l = page->getSelectedLayer();
        Element::Index pos = 0;
        for (auto&& e: l->getElements()) {
            if (e->isInSelection(this)) {
                this->selectedElements.emplace_back(e.get(), pos);
                layerId = page->getSelectedLayerId();
            }
            pos++;
        }
    }

    this->viewPool->dispatchAndClear(xoj::view::SelectorView::DELETE_VIEWS_REQUEST, this->bbox);

    return layerId;
}

auto Selector::isMultiLayerSelection() -> bool { return this->multiLayer; }

auto Selector::releaseElements() -> InsertionOrderRef { return std::move(this->selectedElements); }

//////////////////////////////////////////////////////////

RectangularSelector::RectangularSelector(double x, double y, bool multiLayer):
        Selector(multiLayer), sx(x), sy(y), ex(x), ey(y) {
    bbox.addPoint(x, y);
}

RectangularSelector::~RectangularSelector() = default;

auto RectangularSelector::contains(double x, double y) const -> bool { return bbox.contains(x, y); }

void RectangularSelector::currentPos(double x, double y) {
    bbox = Range(sx, sy);
    bbox.addPoint(x, y);

    Range rg = bbox;
    rg.addPoint(ex, ey);  // in case the selection is shrinking

    this->ex = x;
    this->ey = y;

    boundaryPoints = {{sx, sy}, {sx, ey}, {ex, ey}, {ex, sy}};

    this->viewPool->dispatch(xoj::view::SelectorView::FLAG_DIRTY_REGION, rg);

    this->maxDist = std::max({this->maxDist, x - this->sx, this->sx - x, y - this->sy, this->sy - y});
}

auto RectangularSelector::userTapped(double zoom) const -> bool { return this->maxDist < 10 / zoom; }

auto RectangularSelector::getBoundary() const -> const std::vector<BoundaryPoint>& { return boundaryPoints; }

//////////////////////////////////////////////////////////

LassoSelector::LassoSelector(double x, double y, bool multiLayer): Selector(multiLayer) { currentPos(x, y); }

void LassoSelector::currentPos(double x, double y) {
    boundaryPoints.emplace_back(x, y);
    bbox.addPoint(x, y);

    // at least three points needed
    if (boundaryPoints.size() >= 3) {
        Range rg(x, y);
        const BoundaryPoint& penultimatePoint = boundaryPoints[boundaryPoints.size() - 2];
        rg.addPoint(penultimatePoint.x, penultimatePoint.y);
        // rg contains the added segment
        // add the first point to make sure the filling is painted correctly
        rg.addPoint(boundaryPoints.front().x, boundaryPoints.front().y);

        this->viewPool->dispatch(xoj::view::SelectorView::FLAG_DIRTY_REGION, rg);
    }
}

auto LassoSelector::contains(double x, double y) const -> bool {
    if (boundaryPoints.size() <= 2 || !this->bbox.contains(x, y)) {
        return false;
    }

    int hits = 0;

    const BoundaryPoint& last = boundaryPoints.back();

    double lastx = last.x;
    double lasty = last.y;
    double curx = NAN, cury = NAN;

    // Walk the edges of the polygon
    for (auto pointIterator = boundaryPoints.begin(); pointIterator != boundaryPoints.end();
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

auto LassoSelector::userTapped(double zoom) const -> bool {
    double maxDist = 10 / zoom;
    const BoundaryPoint& r0 = boundaryPoints.front();
    for (const BoundaryPoint& p: boundaryPoints) {
        if (std::abs(r0.x - p.x) > maxDist || std::abs(r0.y - p.y) > maxDist) {
            return false;
        }
    }
    return true;
}

auto LassoSelector::getBoundary() const -> const std::vector<BoundaryPoint>& { return boundaryPoints; }
