#include "QuadCache.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>

#include <cairo.h>

#include "gui/PageView.h"
#include "util/Point.h"
#include "util/Range.h"
#include "util/Rectangle.h"
#include "util/safe_casts.h"

#include "Mask.h"
#include "config-debug.h"  // for DEBUG_QUAD_TREE

#ifdef DEBUG_QUAD_TREE
#define IF_DBG_QUAD_TREE(f) f
#else
#define IF_DBG_QUAD_TREE(f)
#endif

using namespace xoj::view;

class QuadCache::Node {
public:
    Node(Range area, unsigned lvl, const QuadCache& parent):
            outdated(false), depth(lvl), area(area), parent(parent), lastNeeded(Clock::now()) {
        xoj_assert(area.isValid());
    }

    void markForRendering() const { parent.view->scheduleMakeTile(this->area, this->depth); }

    enum Child : size_t { TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT };
    static Range getQuarter(const Range& area, const xoj::util::Point<double>& center, Child c) {
        if (c == TOP_LEFT) {
            return Range(area.minX, area.minY, center.x, center.y);
        } else if (c == BOTTOM_LEFT) {
            return Range(area.minX, center.y, center.x, area.maxY);
        } else if (c == TOP_RIGHT) {
            return Range(center.x, area.minY, area.maxX, center.y);
        } else {
            return Range(center.x, center.y, area.maxX, area.maxY);
        }
    }

    /**
     * minLevel = minimum level to ensure a good enough resolution
     * @return true if rg intersects this->area
     */
    bool paint(cairo_t* cr, const Range& rg, unsigned minLevel, const Node* nearestAncestorWithBuffer) {
        if (!area.hasIntersectionWith(rg)) {
            return false;
        }

        IF_DBG_QUAD_TREE(auto beforeReturn = [&]() {
            xoj::util::CairoSaveGuard saveGuard(cr);
            xoj::util::Point<double> center(.5 * (area.minX + area.maxX), .5 * (area.minY + area.maxY));
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
            cairo_set_dash(cr, nullptr, 0, 0);
            if (this->depth < minLevel) {
                cairo_set_line_width(cr, std::exp2(-static_cast<int>(this->depth)));
                cairo_set_source_rgb(cr, 1, 0, 0);
                cairo_move_to(cr, .25 * area.minX + .75 * area.maxX, .25 * area.minY + .75 * area.maxY);
                cairo_line_to(cr, center.x, center.y);
                cairo_line_to(cr, .75 * area.minX + .25 * area.maxX, .25 * area.minY + .75 * area.maxY);
                cairo_move_to(cr, .25 * area.minX + .75 * area.maxX, .75 * area.minY + .25 * area.maxY);
                cairo_line_to(cr, center.x, center.y);
                cairo_line_to(cr, .75 * area.minX + .25 * area.maxX, .75 * area.minY + .25 * area.maxY);
                cairo_stroke(cr);
            } else {
                cairo_set_line_width(cr, std::exp2(-static_cast<int>(this->depth) + 2));
                cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
                cairo_rectangle(cr, .95 * area.minX + .05 * area.maxX, .95 * area.minY + .05 * area.maxY,
                                .9 * area.getWidth(), .9 * area.getHeight());
                cairo_stroke(cr);
            }
        };);

        if (this->buffer) {
            if (this->depth == minLevel) {
                if (this->outdated) {
                    this->markForRendering();
                    if (nearestAncestorWithBuffer && !nearestAncestorWithBuffer->outdated) {
                        // Prefer the coarser but more up-to-date rendering
                        auto inter = rg.intersect(this->area);
                        inter.addPadding(1e-3 * area.getWidth());  // Add a tiny padding to avoid border effects
                        xoj::util::CairoSaveGuard guard(cr);
                        cairo_rectangle(cr, inter.minX, inter.minY, inter.getWidth(), inter.getHeight());
                        cairo_clip(cr);
                        nearestAncestorWithBuffer->buffer->paintTo(cr);
                    } else {
                        this->buffer->paintTo(cr);
                    }
                } else {
                    this->buffer->paintTo(cr);
                }
                this->lastNeeded = Clock::now();
                IF_DBG_QUAD_TREE(beforeReturn());
                return true;
            } else if (this->depth > minLevel) {
                // Using a buffer with too high a resolution
                this->buffer->paintTo(cr);
                IF_DBG_QUAD_TREE(beforeReturn());
                return true;
            } else {
                if (!nearestAncestorWithBuffer) {
                    // The topmost node will be kept alive: this coarse rendering is then used to avoid missing tiles
                    // from being too noticeable.
                    this->lastNeeded = Clock::now();
                    if (this->outdated) {
                        this->markForRendering();
                    }
                    // Will be used in case we do not have a dense enough buffer
                    nearestAncestorWithBuffer = this;
                } else if (nearestAncestorWithBuffer->outdated || !this->outdated) {
                    // We only use the finer rendering if it is (more) up to date
                    nearestAncestorWithBuffer = this;
                }
            }
        }

        xoj::util::Point<double> center(.5 * (area.minX + area.maxX), .5 * (area.minY + area.maxY));
        for (auto quarter: {TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT}) {
            if (auto& c = this->children[quarter]; c) {
                c->paint(cr, rg, minLevel, nearestAncestorWithBuffer);
            } else {
                if (auto part = getQuarter(area, center, quarter), inter = rg.intersect(part); !inter.empty()) {
                    if (this->depth < minLevel) {
                        // Make the child and recurse - eventually scheduling a rendering
                        c = std::make_unique<Node>(part, this->depth + 1, this->parent);
                        c->paint(cr, rg, minLevel, nullptr);  // No need to transmit the ancestral buffer
                    }
                    if (nearestAncestorWithBuffer) {
                        // Use the low resolution buffer we have
                        inter.addPadding(1e-3 * area.getWidth());  // Add a tiny padding to avoid border effects
                        xoj::util::CairoSaveGuard guard(cr);
                        cairo_rectangle(cr, inter.minX, inter.minY, inter.getWidth(), inter.getHeight());
                        cairo_clip(cr);
                        nearestAncestorWithBuffer->buffer->paintTo(cr);
                    }
                }
            }
        }

        if (this->depth == minLevel) {
            this->markForRendering();
            this->lastNeeded = Clock::now();
        }
        IF_DBG_QUAD_TREE(beforeReturn());
        return true;
    }

    bool preload(const Range& rg, unsigned level) {
        if (!area.hasIntersectionWith(rg)) {
            return false;
        }
        if (this->depth < level) {
            bool preloaded = false;
            xoj::util::Point<double> center(.5 * (area.minX + area.maxX), .5 * (area.minY + area.maxY));
            for (auto quarter: {TOP_LEFT, BOTTOM_LEFT, TOP_RIGHT, BOTTOM_RIGHT}) {
                if (auto& c = this->children[quarter]; c) {
                    preloaded = c->preload(rg, level) || preloaded;
                } else {
                    if (auto part = getQuarter(area, center, quarter); rg.hasIntersectionWith(part)) {
                        // Make the child and recurse - eventually scheduling a rendering
                        c = std::make_unique<Node>(part, this->depth + 1, this->parent);
                        c->preload(rg, level);
                        preloaded = true;  // Some child must have been scheduled for rendering
                    }
                }
            }
            return preloaded;
        } else if (!this->buffer || this->outdated) {
            xoj_assert(this->depth == level);
            this->markForRendering();
            this->lastNeeded = Clock::now();
            return true;
        }
        return false;
    }

    /**
     * Removes any data that hasn't been used since ref
     * Returns true if this node can be destroyed
     */
    bool prune(const TimePoint& ref) {
        bool prunable = ref > this->lastNeeded;
        if (prunable) {
            this->buffer.reset();
        }
        return pruneChildren(ref) && prunable;
    }

    /**
     * Remove any data that hasn't been used since ref in the children of the node
     * Returns true if the node no longer has children.
     */
    bool pruneChildren(const TimePoint& ref) {
        bool childless = true;
        for (auto& c: children) {
            if (c) {
                if (c->prune(ref)) {
                    c.reset();
                } else {
                    childless = false;
                }
            }
        }
        return childless;
    }

    void appendBuffersLastUsedDates(std::vector<TimePoint>& res) {
        for (const auto& c: children) {
            if (c) {
                c->appendBuffersLastUsedDates(res);
            }
        }
        if (this->buffer) {
            res.emplace_back(this->lastNeeded);
        }
    }

    void clear() {
        for (auto& c: this->children) {
            c.reset();
        }
        this->buffer.reset();
    }

    void appendSurfacesForRange(const Range& rg, std::vector<cairo_t*>& res) {
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

    /// Find the node (if any) corresponding to the given data and gives it the buffer
    void assignBufferToNode(std::unique_ptr<Mask> mask, unsigned int depth, const xoj::util::Point<double>& center) {
        if (depth == this->depth) {
            xoj_assert(.5 * (this->area.minX + this->area.maxX) == center.x &&
                       .5 * (this->area.minY + this->area.maxY) == center.y);
            this->buffer = std::move(mask);
            this->outdated = false;
        } else {
            xoj_assert(depth > this->depth);
            xoj::util::Point<double> thisCenter{.5 * (this->area.minX + this->area.maxX),
                                                .5 * (this->area.minY + this->area.maxY)};
            if (thisCenter.x > center.x && thisCenter.y > center.y) {
                if (children[TOP_LEFT]) {
                    children[TOP_LEFT]->assignBufferToNode(std::move(mask), depth, center);
                }  // else: the child got pruned while the RenderJob was running
            } else if (thisCenter.x < center.x && thisCenter.y > center.y) {
                if (children[TOP_RIGHT]) {
                    children[TOP_RIGHT]->assignBufferToNode(std::move(mask), depth, center);
                }
            } else if (thisCenter.x > center.x && thisCenter.y < center.y) {
                if (children[BOTTOM_LEFT]) {
                    children[BOTTOM_LEFT]->assignBufferToNode(std::move(mask), depth, center);
                }
            } else /*if (thisCenter.x < center.x && thisCenter.y < center.y)*/ {
                if (children[BOTTOM_RIGHT]) {
                    children[BOTTOM_RIGHT]->assignBufferToNode(std::move(mask), depth, center);
                }
            }
        }
    }

    void markAsOutdated() {
        this->outdated = true;
        for (const auto& c: children) {
            if (c) {
                c->markAsOutdated();
            }
        }
    }


private:
    std::array<std::unique_ptr<Node>, 4> children;
    std::unique_ptr<Mask> buffer;

    bool outdated;   ///< The buffer is outdated and needs rerendering
    unsigned depth;  ///< Depth of the node - the root has depth 0
    Range area;      ///< The user-space area covered by this node

    const QuadCache& parent;

    /// Last time this node's buffer was used
    mutable Clock::time_point lastNeeded;
};


QuadCache::QuadCache(const Range& area, int DPIscaling, XojPageView* view):
        view(view),
        DPIscaling(DPIscaling),
        zoom(std::sqrt(TILE_SIZE / (area.getHeight() * area.getWidth()))),
        root(std::make_unique<Node>(area, 0, *this)) {}
QuadCache::~QuadCache() = default;

void xoj::view::QuadCache::changeArea(const Range& area) {
    this->zoom = std::sqrt(TILE_SIZE / (area.getHeight() * area.getWidth()));
    this->root = std::make_unique<Node>(area, 0, *this);
}

void QuadCache::clear() { root->clear(); }

auto QuadCache::makeSuitableMask(unsigned int depth, const Range& area) const -> std::unique_ptr<Mask> {
    auto pow = std::exp2(depth);
    return std::make_unique<Mask>(this->DPIscaling, area, pow * this->zoom, CAIRO_CONTENT_COLOR_ALPHA);
}

void QuadCache::assignBufferToNode(std::unique_ptr<Mask> mask, unsigned int depth, const Range& area) {
    root->assignBufferToNode(std::move(mask), depth, {.5 * (area.minX + area.maxX), .5 * (area.minY + area.maxY)});
}

void QuadCache::paintTo(cairo_t* targetCr) const {
    xoj_assert(root);
    Range rg;
    cairo_clip_extents(targetCr, &rg.minX, &rg.minY, &rg.maxX, &rg.maxY);
    if (!rg.isValid()) {
        return;
    }
    cairo_matrix_t matrix;
    cairo_get_matrix(targetCr, &matrix);
    xoj_assert(matrix.xx == matrix.yy && matrix.xy == 0. && matrix.yx == 0);
    unsigned minLevel = ceil_cast<unsigned>(std::max(0., std::log2(matrix.xx / this->zoom)));

    cairo_pattern_set_filter(cairo_get_source(targetCr), CAIRO_FILTER_FAST);
    this->root->paint(targetCr, rg, minLevel, nullptr);
}

bool xoj::view::QuadCache::preload(const Range& rg, double zoom) {
    unsigned minLevel = ceil_cast<unsigned>(std::max(0., std::log2(zoom / this->zoom)));
    return this->root->preload(rg, minLevel);
}

void xoj::view::QuadCache::markAsOutdated() { this->root->markAsOutdated(); }

auto xoj::view::QuadCache::getSurfacesFor(const Range& rg) -> std::vector<cairo_t*> {
    std::vector<cairo_t*> res;
    root->appendSurfacesForRange(rg, res);
    return res;
}

auto xoj::view::QuadCache::getBuffersLastUsedDates() const -> std::vector<TimePoint> {
    std::vector<TimePoint> res;
    root->appendBuffersLastUsedDates(res);
    return res;
}
auto xoj::view::QuadCache::prune(TimePoint date) -> bool { return this->root->prune(date); }
