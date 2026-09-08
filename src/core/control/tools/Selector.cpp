#include "Selector.h"

#include <algorithm>  // for max, min
#include <cmath>      // for abs
#include <limits>     // for numeric_limits
#include <memory>     // for __shared_ptr_access

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

    // Trigger redraw before any geometry modifications
    this->viewPool->dispatchAndClear(xoj::view::SelectorView::DELETE_VIEWS_REQUEST, this->bbox);

    // Extend selection geometry (bbox) to infinity where touching page edges
    this->extendAtPageEdges();

    // Find selected elements
    size_t layerId = 0;
    if (multiLayer && !disableMultilayer) {
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

void RectangularSelector::extendAtPageEdges() {
    const double pageWidth = page->getWidth();
    const double pageHeight = page->getHeight();

    if (pageWidth > 0 && pageHeight > 0) {
        if (bbox.minX <= EDGE_TOUCHING_THRESHOLD) {
            bbox.minX = std::numeric_limits<double>::lowest();
        }
        if (bbox.minY <= EDGE_TOUCHING_THRESHOLD) {
            bbox.minY = std::numeric_limits<double>::lowest();
        }
        if (bbox.maxX >= pageWidth - EDGE_TOUCHING_THRESHOLD) {
            bbox.maxX = std::numeric_limits<double>::max();
        }
        if (bbox.maxY >= pageHeight - EDGE_TOUCHING_THRESHOLD) {
            bbox.maxY = std::numeric_limits<double>::max();
            ;
        }
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

void LassoSelector::extendAtPageEdges() {
    const double pageWidth = page->getWidth();
    const double pageHeight = page->getHeight();

    // Skip extension when the page geometry is invalid or the lasso cannot form a polygon.
    if (pageWidth <= 0 || pageHeight <= 0 || boundaryPoints.size() <= 2) {
        return;
    }

    // Fast path: Most lassos stay fully inside the page, so avoid the projection work in that common case.
    if (bbox.minX > EDGE_TOUCHING_THRESHOLD && bbox.minY > EDGE_TOUCHING_THRESHOLD &&
        bbox.maxX < pageWidth - EDGE_TOUCHING_THRESHOLD && bbox.maxY < pageHeight - EDGE_TOUCHING_THRESHOLD) {
        return;
    }

    auto const isOnEdge = [&](BoundaryPoint const& p) -> bool {
        return p.x <= EDGE_TOUCHING_THRESHOLD || p.x >= pageWidth - EDGE_TOUCHING_THRESHOLD ||
               p.y <= EDGE_TOUCHING_THRESHOLD || p.y >= pageHeight - EDGE_TOUCHING_THRESHOLD;
    };

    // Project edge-touching coordinates to infinity while leaving interior coordinates unchanged.
    auto const extendCoordinate = [&](double value, double pageExtent) -> double {
        if (value <= EDGE_TOUCHING_THRESHOLD) {
            return std::numeric_limits<double>::lowest();
        }
        if (value >= pageExtent - EDGE_TOUCHING_THRESHOLD) {
            return std::numeric_limits<double>::max();
        }
        return value;
    };

    auto const project = [&](BoundaryPoint const& p) -> BoundaryPoint {
        return {extendCoordinate(p.x, pageWidth), extendCoordinate(p.y, pageHeight)};
    };

    // Build the final polygon in a scratch buffer because one input point may emit multiple output points.
    std::vector<BoundaryPoint> newBoundaryPoints;
    newBoundaryPoints.reserve(boundaryPoints.size() * 2);

    // Keep the extended polygon and bbox in sync as points are emitted.
    auto const appendExtendedPoint = [&](BoundaryPoint const& p) -> void {
        newBoundaryPoints.push_back(p);
        bbox.addPoint(p.x, p.y);
    };

    // Walk the original lasso and splice in projected points for runs that touch the page edge.
    auto const n = boundaryPoints.size();
    for (size_t i = 0; i < n; i++) {
        auto const& current = boundaryPoints[i];
        auto const currentOnEdge = isOnEdge(current);

        if (!currentOnEdge) {
            appendExtendedPoint(current);
            continue;
        }

        // Current point is on a page edge.
        auto const prevOnEdge = isOnEdge(boundaryPoints[(i + n - 1) % n]);
        auto const nextOnEdge = isOnEdge(boundaryPoints[(i + 1) % n]);

        if (!prevOnEdge) {
            // Entering an edge run: emit original, then projected
            appendExtendedPoint(current);
            appendExtendedPoint(project(current));
        } else if (!nextOnEdge) {
            // Leaving an edge run: emit projected, then original
            appendExtendedPoint(project(current));
            appendExtendedPoint(current);
        } else {
            // Interior of an edge run: emit only projected
            appendExtendedPoint(project(current));
        }
    }

    // Replace the original lasso with the extended polygon used for containment checks.
    boundaryPoints = std::move(newBoundaryPoints);
}

auto LassoSelector::contains(double x, double y) const -> bool {
    if (boundaryPoints.size() <= 2 || !bbox.contains(x, y)) {
        return false;
    }

    int hits = 0;
    auto hitSign = [](const BoundaryPoint& last, const BoundaryPoint& current) {
        return current.y > last.y ? 1 : current.y < last.y ? -1 : current.x < last.x ? 1 : -1;
    };

    const BoundaryPoint* last = &boundaryPoints.back();

    // Walk the edges of the polygon
    for (auto current = boundaryPoints.begin(); current != boundaryPoints.end(); last = &*current, ++current) {
        // Change `hits` by +/-1 if {x,y} is in the horizontal half-stripe going left, delimited by the current segment
        if (auto [min, max] = std::minmax(current->y, last->y); y < min || y > max) {
            // Not in the horizontal stripe
            continue;
        }
        if (auto [min, max] = std::minmax(current->x, last->x); x < min) {
            // Left of the segment's bounding box: in the half stripe
            hits += hitSign(*last, *current);
            continue;
        } else if (x > max) {
            // Right of the segment's bounding box: out of the half stripe
            continue;
        }

        // Now, the point is in the bounding box of the segment

        if (current->y == last->y) {
            // Horizontal segment: the bounding box is everything
            hits += hitSign(*last, *current);
            continue;
        }

        // Abscissa of the projection of the point onto the horizontal line passing through *current
        // The projection is parallel to the vector (*current - *last)
        double projX = x + (current->x - last->x) * (current->y - y) / (current->y - last->y);
        if (projX <= current->x) {
            hits += hitSign(*last, *current);
        }
    }

    return hits != 0;
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
