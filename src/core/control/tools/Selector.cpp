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
    this->pageWidth = page->getWidth();
    this->pageHeight = page->getHeight();

    const bool useMultiLayer = multiLayer && !disableMultilayer;
    Range selectionDomain;

    // First pass: compute selectionDomain from all candidate elements
    auto addToSelectionDomain = [&selectionDomain](const Layer* layer) {
        if (!layer->isVisible()) {
            return;
        }
        for (const auto& e: layer->getElementsView()) {
            selectionDomain = selectionDomain.unite(Range(e->boundingRect()));
        }
    };

    {
        std::shared_lock lock(*doc);
        if (useMultiLayer) {
            for (const Layer* layer: page->getLayersView()) {
                addToSelectionDomain(layer);
            }
        } else {
            addToSelectionDomain(page->getSelectedLayer());
        }
    }

    if (selectionDomain.empty()) {
        selectionDomain = this->bbox;
    }

    this->tailorToSelectionDomain(selectionDomain);

    // Second pass: find selected elements
    size_t layerId = 0;

    if (useMultiLayer) {
        std::shared_lock lock(*doc);
        const auto layers = page->getLayersView();
        for (auto it = layers.rbegin(); it != layers.rend(); it++) {
            const Layer* l = *it;
            if (!l->isVisible()) {
                continue;
            }
            bool selectionOnLayer = false;
            Element::Index pos = 0;
            for (const auto& e: l->getElementsView()) {
                if (e->isInSelection(this)) {
                    this->selectedElements.emplace_back(e, pos);
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
        std::shared_lock lock(*doc);
        const Layer* l = page->getSelectedLayer();
        Element::Index pos = 0;
        for (const auto& e: l->getElementsView()) {
            if (e->isInSelection(this)) {
                this->selectedElements.emplace_back(e, pos);
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

auto RectangularSelector::contains(double x, double y) const -> bool { return extendedBbox.contains(x, y); }

void RectangularSelector::tailorToSelectionDomain(const Range& selectionDomain) {
    constexpr double THRESHOLD = 1.0;  // pt

    extendedBbox = bbox;

    if (pageWidth > 0 && pageHeight > 0 && selectionDomain.isValid()) {
        if (extendedBbox.minX <= THRESHOLD)
            extendedBbox.minX = selectionDomain.minX;
        if (extendedBbox.minY <= THRESHOLD)
            extendedBbox.minY = selectionDomain.minY;
        if (extendedBbox.maxX >= pageWidth - THRESHOLD)
            extendedBbox.maxX = selectionDomain.maxX;
        if (extendedBbox.maxY >= pageHeight - THRESHOLD)
            extendedBbox.maxY = selectionDomain.maxY;
    }
}

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

void LassoSelector::tailorToSelectionDomain(const Range& selectionDomain) {
    constexpr double THRESHOLD = 1.0;  // pt

    if (pageWidth <= 0 || pageHeight <= 0 || boundaryPoints.size() <= 2 || !selectionDomain.isValid()) {
        extendedBoundaryPoints = boundaryPoints;
        extendedBbox = bbox;
        return;
    }

    auto const isOnEdge = [&](BoundaryPoint const& p) -> bool {
        return p.x <= THRESHOLD || p.x >= pageWidth - THRESHOLD || p.y <= THRESHOLD || p.y >= pageHeight - THRESHOLD;
    };

    auto const project = [&](BoundaryPoint const& p) -> BoundaryPoint {
        double px = p.x;
        double py = p.y;
        if (px <= THRESHOLD)
            px = selectionDomain.minX;
        if (px >= pageWidth - THRESHOLD)
            px = selectionDomain.maxX;
        if (py <= THRESHOLD)
            py = selectionDomain.minY;
        if (py >= pageHeight - THRESHOLD)
            py = selectionDomain.maxY;
        return {px, py};
    };

    extendedBoundaryPoints.clear();
    extendedBoundaryPoints.reserve(boundaryPoints.size() * 2);

    auto const n = boundaryPoints.size();
    for (size_t i = 0; i < n; i++) {
        auto const& current = boundaryPoints[i];
        auto const currentOnEdge = isOnEdge(current);

        if (!currentOnEdge) {
            extendedBoundaryPoints.push_back(current);
            continue;
        }

        // Current point is on a page edge.
        auto const prevOnEdge = isOnEdge(boundaryPoints[(i + n - 1) % n]);
        auto const nextOnEdge = isOnEdge(boundaryPoints[(i + 1) % n]);

        if (!prevOnEdge) {
            // Entering an edge run: emit original, then projected
            extendedBoundaryPoints.push_back(current);
            extendedBoundaryPoints.push_back(project(current));
        } else if (!nextOnEdge) {
            // Leaving an edge run: emit projected, then original
            extendedBoundaryPoints.push_back(project(current));
            extendedBoundaryPoints.push_back(current);
        } else {
            // Interior of an edge run: emit only projected
            extendedBoundaryPoints.push_back(project(current));
        }
    }

    // Build extended bounding box
    extendedBbox = Range();
    for (auto const& p: extendedBoundaryPoints) {
        extendedBbox.addPoint(p.x, p.y);
    }
}

auto LassoSelector::contains(double x, double y) const -> bool {
    auto const& pts = extendedBoundaryPoints.empty() ? boundaryPoints : extendedBoundaryPoints;
    auto const& bboxRef = extendedBoundaryPoints.empty() ? bbox : extendedBbox;

    if (pts.size() <= 2 || !bboxRef.contains(x, y)) {
        return false;
    }

    int hits = 0;

    const BoundaryPoint& last = pts.back();

    double lastx = last.x;
    double lasty = last.y;
    double curx = NAN, cury = NAN;

    // Walk the edges of the polygon
    for (auto pointIterator = pts.begin(); pointIterator != pts.end(); lastx = curx, lasty = cury, ++pointIterator) {
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
