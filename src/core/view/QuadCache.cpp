#include "QuadCache.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>

#include <cairo.h>

#include "util/Point.h"
#include "util/Range.h"
#include "util/safe_casts.h"

#include "Mask.h"
#include "config-debug.h"  // for DEBUG_QUAD_TREE

#ifdef DEBUG_QUAD_TREE
#define IF_DBG_QUAD_TREE(f) f
#else
#define IF_DBG_QUAD_TREE(f)
#endif

using namespace xoj::view;

QuadCache::Node::Node(Range area, Depth lvl, const QuadCache& parent):
        outdated(false), depth(lvl), area(area), parent(parent), lastNeeded(Clock::now()) {
    xoj_assert(area.isValid());
}
QuadCache::Node::~Node() = default;

bool QuadCache::Node::renderBuffer() { return parent.renderNode(*this); }

static Range getQuarter(const Range& area, const xoj::util::Point<double>& center, QuadCache::Node::Child c) {
    if (c == QuadCache::Node::TOP_LEFT) {
        return Range(area.minX, area.minY, center.x, center.y);
    } else if (c == QuadCache::Node::BOTTOM_LEFT) {
        return Range(area.minX, center.y, center.x, area.maxY);
    } else if (c == QuadCache::Node::TOP_RIGHT) {
        return Range(center.x, area.minY, area.maxX, center.y);
    } else {
        return Range(center.x, center.y, area.maxX, area.maxY);
    }
}

static xoj::util::Point<double> getCenter(const Range& rg) {
    return {(rg.minX + rg.maxX) / 2, (rg.minY + rg.maxY) / 2};
}

void QuadCache::Node::paintAndPreload(cairo_t* cr, const Range& paintRg, const Range& preloadRg, Depth tgtDepth,
                                      const Node* nearestAncestorWithBuffer) {
    if (!area.hasIntersectionWith(paintRg)) {
        if (this->depth <= tgtDepth) {
            preload(preloadRg, tgtDepth);
        }
        return;
    }

    IF_DBG_QUAD_TREE(class BeforeReturn {
    public:
        BeforeReturn(std::function<void()> f): beforeReturn(f) {}
        ~BeforeReturn() { beforeReturn(); }
        std::function<void()> beforeReturn;
    } beforeReturn([&]() {
                         // Executed upon function return to have the debug info on top
                         xoj::util::CairoSaveGuard saveGuard(cr);
                         cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
                         cairo_set_dash(cr, nullptr, 0, 0);
                         if (this->depth >= tgtDepth) {
                             cairo_set_line_width(cr, std::exp2(-static_cast<int>(this->depth) + 2));
                             cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
                             cairo_rectangle(cr, .95 * area.minX + .05 * area.maxX, .95 * area.minY + .05 * area.maxY,
                                             .9 * area.getWidth(), .9 * area.getHeight());
                             cairo_stroke(cr);
                         }
                     }););

    auto paintUsingNearestAncestor = [](cairo_t* cr, const Node* nearestAncestorWithBuffer, Range rg) {
        rg.addPadding(1e-3 * rg.getWidth());  // Add a tiny padding to avoid border effects
        xoj::util::CairoSaveGuard guard(cr);
        cairo_rectangle(cr, rg.minX, rg.minY, rg.getWidth(), rg.getHeight());
        cairo_clip(cr);
        nearestAncestorWithBuffer->buffer->paintTo(cr);
    };

    if (this->depth == tgtDepth) {
        this->lastNeeded = Clock::now();
        // The call to this->renderBuffer() will trigger a rendering of this Node's buffer (synch. or asynch.)
        if ((this->buffer && !this->outdated) || this->renderBuffer()) {
            // The buffer was already up-to-date or has been synchronously updated.
            this->buffer->paintTo(cr);
            return;
        } else if (nearestAncestorWithBuffer && !nearestAncestorWithBuffer->outdated) {
            // Prefer the coarser but more up-to-date rendering
            paintUsingNearestAncestor(cr, nearestAncestorWithBuffer, paintRg.intersect(this->area));
            return;
        } else if (this->buffer) {
            // Use this buffer, even if it is outdated. It'll be asynchronously updated anyway
            this->buffer->paintTo(cr);
            return;
        }
        // We reached the target depth but did not find a suitable buffer to paint yet: we will try to go deeper below
    } else if (this->depth < tgtDepth) {
        // Keep track of the closest ancestor to use its buffer as a default in case the target node has none
        if (this->buffer) {
            if (!nearestAncestorWithBuffer) {
                // The topmost node will be kept alive: this coarse rendering is then used to avoid missing tiles
                // from being too noticeable.
                this->lastNeeded = Clock::now();
                if (this->outdated) {
                    this->renderBuffer();
                }
                // Will be used in case we do not have a dense enough buffer
                nearestAncestorWithBuffer = this;
            } else if (nearestAncestorWithBuffer->outdated || !this->outdated) {
                // We only use the finer rendering if it is at least as up to date
                nearestAncestorWithBuffer = this;
            }
        }
    } else {
        // We are deeper than the target depth because no suitable buffer has been found so far
        // Use this finer buffer if it is there. Otherwise, we will keep going deeper in the tree.
        if (this->buffer) {
            this->buffer->paintTo(cr);
            return;
        }
    }

    // We haven't painted anything yet for this node: go deeper in the tree

    xoj::util::Point<double> center = getCenter(area);
    for (auto quarter: {TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT}) {
        if (auto& c = this->children[quarter]; c) {
            c->paintAndPreload(cr, paintRg, preloadRg, tgtDepth, nearestAncestorWithBuffer);
        } else {
            auto part = getQuarter(area, center, quarter);
            if (part.hasIntersectionWith(preloadRg)) {
                if (auto inter = paintRg.intersect(part); !inter.empty() && nearestAncestorWithBuffer) {
                    // Use the low resolution buffer we have on the missing quadrant
                    paintUsingNearestAncestor(cr, nearestAncestorWithBuffer, inter);
                }
                if (this->depth < tgtDepth) {
                    // Make the child and recurse - eventually scheduling a rendering
                    c = std::make_unique<Node>(part, this->depth + 1, this->parent);
                    // No need to transmit the ancestral buffer: we already painted it
                    c->paintAndPreload(cr, paintRg, preloadRg, tgtDepth, nullptr);
                }
            }
        }
    }
}

void QuadCache::Node::preload(const Range& rg, Depth level) {
    if (!area.hasIntersectionWith(rg)) {
        return;
    }
    if (this->depth < level) {
        xoj::util::Point<double> center = getCenter(area);
        for (auto quarter: {TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT}) {
            if (auto& c = this->children[quarter]; c) {
                c->preload(rg, level);
            } else {
                if (auto part = getQuarter(area, center, quarter); rg.hasIntersectionWith(part)) {
                    // Make the child and recurse - eventually scheduling a rendering
                    c = std::make_unique<Node>(part, this->depth + 1, this->parent);
                    c->preload(rg, level);
                }
            }
        }
    } else if (!this->buffer || this->outdated) {
        xoj_assert(this->depth == level);
        this->renderBuffer();
        this->lastNeeded = Clock::now();
    }
}

bool QuadCache::paintSingleTile(cairo_t* cr, const TileInfo& ti) {
    Node* n = root.ensureNodeExists(ti.depth, getCenter(ti.area));
    xoj_assert(n->area == ti.area);
    n->lastNeeded = Clock::now();
    if (n->buffer || n->renderBuffer()) {
        xoj_assert(n->buffer);
        n->buffer->paintTo(cr);
        return true;
    }
    return false;
}

bool QuadCache::Node::prune(const TimePoint& ref) {
    bool prunable = ref > this->lastNeeded;
    if (prunable) {
        this->buffer.reset();
    }

    for (auto& c: children) {
        if (c) {
            if (c->prune(ref)) {
                c.reset();
            } else {
                prunable = false;
            }
        }
    }
    return prunable;
}

void QuadCache::Node::appendBuffersLastUsedDates(std::vector<TimePoint>& res) const {
    for (const auto& c: children) {
        if (c) {
            c->appendBuffersLastUsedDates(res);
        }
    }
    if (this->buffer) {
        res.emplace_back(this->lastNeeded);
    }
}

void QuadCache::Node::clear() {
    for (auto& c: this->children) {
        c.reset();
    }
    this->buffer.reset();
}

void QuadCache::Node::appendSurfacesForRange(const Range& rg, std::vector<cairo_t*>& res) const {
    if (!this->area.hasIntersectionWith(rg)) {
        return;
    }
    if (this->buffer) {
        res.emplace_back(this->buffer->get());
    }
    for (auto& c: children) {
        if (c) {
            c->appendSurfacesForRange(rg, res);
        }
    }
}

void QuadCache::Node::assignBuffer(std::unique_ptr<Mask> buffer) {
    xoj_assert(buffer);
    this->buffer = std::move(buffer);
    this->outdated = false;
}

static QuadCache::Node::Child getChildContainingPoint(const xoj::util::Point<double>& p,
                                                      const xoj::util::Point<double>& center) {
    if (center.x > p.x && center.y > p.y) {
        return QuadCache::Node::TOP_LEFT;
    } else if (center.x < p.x && center.y > p.y) {
        return QuadCache::Node::TOP_RIGHT;
    } else if (center.x > p.x && center.y < p.y) {
        return QuadCache::Node::BOTTOM_LEFT;
    } else /*if (center.x < p.x && center.y < p.y)*/ {
        return QuadCache::Node::BOTTOM_RIGHT;
    }
}

auto QuadCache::Node::getNodeAt(Depth d, const xoj::util::Point<double>& p) -> Node* {
    if (d == this->depth) {
        return this;
    } else {
        xoj_assert(d > this->depth);
        auto child = getChildContainingPoint(p, getCenter(area));
        return children[child] ? children[child]->getNodeAt(d, p) : nullptr;
    }
}

auto QuadCache::Node::ensureNodeExists(Depth d, const xoj::util::Point<double>& p) -> Node* {
    if (d == this->depth) {
        return this;
    } else {
        xoj_assert(d > this->depth);
        xoj::util::Point<double> center = getCenter(area);
        auto child = getChildContainingPoint(p, center);
        if (!children[child]) {
            children[child] = std::make_unique<Node>(getQuarter(area, center, child), this->depth + 1, this->parent);
        }
        return children[child]->ensureNodeExists(d, p);
    }
}

void QuadCache::Node::markAsOutdated() {
    this->outdated = true;
    for (const auto& c: children) {
        if (c) {
            c->markAsOutdated();
        }
    }
}

void QuadCache::Node::changeArea(const Range& rg) {
    this->area = rg;
    this->clear();
}


QuadCache::QuadCache(const Range& area, int DPIscaling, std::function<bool(Node&)> renderNode):
        renderNode(std::move(renderNode)),
        DPIscaling(DPIscaling),
        zoom(std::sqrt(TILE_SIZE / (area.getHeight() * area.getWidth()))),
        root(area, 0, *this) {}
QuadCache::~QuadCache() = default;

void QuadCache::changeArea(const Range& area) {
    this->zoom = std::sqrt(TILE_SIZE / (area.getHeight() * area.getWidth()));
    this->root.changeArea(area);
}

void QuadCache::clear() { root.clear(); }

auto QuadCache::makeSuitableMask(const TileInfo& ti) const -> std::unique_ptr<Mask> {
    return std::make_unique<Mask>(DPIscaling, ti.area, std::exp2(ti.depth) * zoom, CAIRO_CONTENT_COLOR_ALPHA);
}

void QuadCache::assignBufferToNode(std::unique_ptr<Mask> mask, const TileInfo& ti) {
    auto* n = root.getNodeAt(ti.depth, {.5 * (ti.area.minX + ti.area.maxX), .5 * (ti.area.minY + ti.area.maxY)});
    if (n) {
        n->assignBuffer(std::move(mask));
    }
}

void QuadCache::paintTo(cairo_t* targetCr) { paintAndPreload(targetCr, 0.); }

void QuadCache::preload(const Range& rg, double zoom) {
    Depth minLevel = ceil_cast<Depth>(std::max(0., std::log2(zoom / this->zoom)));
    this->root.preload(rg, minLevel);
}

void QuadCache::paintAndPreload(cairo_t* targetCr, double preloadPaddingCoefficient) {
    Range rg;
    cairo_clip_extents(targetCr, &rg.minX, &rg.minY, &rg.maxX, &rg.maxY);
    if (!rg.isValid()) {
        return;
    }

    cairo_matrix_t matrix;
    cairo_get_matrix(targetCr, &matrix);
    xoj_assert(matrix.xx == matrix.yy && matrix.xy == 0. && matrix.yx == 0);
    Depth tgtDepth = ceil_cast<Depth>(std::max(0., std::log2(matrix.xx / this->zoom)));

    auto preloadArea = rg;
    xoj_assert(preloadPaddingCoefficient >= 0.);
    preloadArea.addPadding(preloadPaddingCoefficient * std::hypot(rg.getWidth(), rg.getHeight()));

    cairo_pattern_set_filter(cairo_get_source(targetCr), CAIRO_FILTER_FAST);
    this->root.paintAndPreload(targetCr, rg, preloadArea, tgtDepth, nullptr);
}

void QuadCache::markAsOutdated() { this->root.markAsOutdated(); }

auto QuadCache::getSurfacesFor(const Range& rg) -> std::vector<cairo_t*> {
    std::vector<cairo_t*> res;
    root.appendSurfacesForRange(rg, res);
    return res;
}

auto QuadCache::getBuffersLastUsedDates() const -> std::vector<TimePoint> {
    std::vector<TimePoint> res;
    root.appendBuffersLastUsedDates(res);
    return res;
}
auto QuadCache::prune(TimePoint date) -> bool { return this->root.prune(date); }
