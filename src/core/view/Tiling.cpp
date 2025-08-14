#include "Tiling.h"

#include <algorithm>
#include <functional>

#include <cairo.h>

#include "util/Point.h"
#include "util/Range.h"
#include "util/Rectangle.h"
#include "util/safe_casts.h"

#include "Mask.h"
#include "config-debug.h"

// You can try that out for debug/fun
// static constexpr int MAX_TILE_SIZE = 50;               ///< Each tile has width/height <= MAX_TILE_SIZE
// static constexpr int ELLIPSE_VERTICAL_RADIUS = 400;    ///< In pixels
// static constexpr int ELLIPSE_HORIZONTAL_RADIUS = 300;  ///< In pixels

static constexpr int MAX_TILE_SIZE = 2048;                           ///< Each tile has width/height <= MAX_TILE_SIZE
static constexpr int ELLIPSE_VERTICAL_RADIUS = 3 * MAX_TILE_SIZE;    ///< In pixels
static constexpr int ELLIPSE_HORIZONTAL_RADIUS = 2 * MAX_TILE_SIZE;  ///< In pixels

/*
 * Each tile takes MAX_TILE_SIZE * MAX_TILE_SIZE * DPIScaling^2 * 4 bytes (=8M or 32M for DPIScaling=2) of memory,
 * so we avoid to reallocate them all the time.
 */
size_t xoj::view::Tiling::getEstimatedMemUsageForOneTile(int DPIScaling) {
    return as_unsigned(MAX_TILE_SIZE) * as_unsigned(MAX_TILE_SIZE) * 4 * as_unsigned(DPIScaling * DPIScaling);
}


/// Smaller movements do not trigger an update of the tiles. In pixels
static constexpr int RETILE_THRESHOLD = MAX_TILE_SIZE / 10;

#ifdef DEBUG_TILING
#define IF_DBG_TILING(f) f
#else
#define IF_DBG_TILING(f)
#endif

using namespace xoj::view;

Tiling::Tiling() = default;
Tiling::Tiling(Tiling&&) = default;
Tiling& Tiling::operator=(Tiling&&) = default;
Tiling::~Tiling() = default;

void Tiling::clear() { tiles.clear(); }
bool Tiling::empty() const { return tiles.empty(); }


void Tiling::paintTo(cairo_t* targetCr) const {
    xoj::util::CairoSaveGuard guard(targetCr);
    cairo_scale(targetCr, 1. / zoom, 1. / zoom);

    for (const auto& t: tiles) {
        t->paintTo(targetCr);

        IF_DBG_TILING(({
            static constexpr double COLORS[6][3] = {{1., 0., 0.}, {0., 1., 0.}, {0., 0., 1.},
                                                    {1., 1., 0.}, {1., 0., 1.}, {0., 1., 1.}};
            auto ext = t->getExtent();
            int x = ext.x / MAX_TILE_SIZE;
            int y = ext.y / MAX_TILE_SIZE;
            const auto& c = COLORS[(x + 2 * y) % 6];
            cairo_set_source_rgba(targetCr, c[0], c[1], c[2], .3);
            cairo_rectangle(targetCr, ext.x, ext.y, ext.width, ext.height);
            cairo_fill(targetCr);

            cairo_move_to(targetCr, ext.x + .1 * ext.width, ext.y + .5 * ext.height);
            std::string txt = std::to_string(x) + " ; " + std::to_string(y);
            cairo_set_source_rgb(targetCr, 0, 0, 0);
            cairo_show_text(targetCr, txt.c_str());
        }));
    }
}

static bool isTileWithinEllipse(int x, int y, const xoj::util::Point<int>& center) {
    return std::hypot((x + MAX_TILE_SIZE / 2 - center.x) / static_cast<double>(ELLIPSE_HORIZONTAL_RADIUS),
                      (y + MAX_TILE_SIZE / 2 - center.y) / static_cast<double>(ELLIPSE_VERTICAL_RADIUS)) <= 1;
}

void Tiling::populate(int DPIscaling, const xoj::util::Point<int>& c, const Range& extent, double zoom) {
    this->zoom = zoom;
    this->createTiles(DPIscaling, this->computeRetiling(c, extent, zoom));
}

auto Tiling::computeRetiling(const xoj::util::Point<int>& c, const Range& extent, double zoom) -> RetilingData {
    if (this->zoom == zoom && !this->tiles.empty() && this->center.distance(c) < RETILE_THRESHOLD) {
        return {};
    }

    auto box = Range((c.x - ELLIPSE_HORIZONTAL_RADIUS) / zoom, (c.y - ELLIPSE_VERTICAL_RADIUS) / zoom,
                     (c.x + ELLIPSE_HORIZONTAL_RADIUS) / zoom, (c.y + ELLIPSE_VERTICAL_RADIUS) / zoom)
                       .intersect(extent);

    if (box.empty()) {
        // The ellipse is entirely out of the page - none of the tiles are useful anymore
        return {{}, std::move(this->tiles)};
    }

    bool zoomChanged = this->zoom != zoom;

    auto partitionFun = [&]() {
        if (zoomChanged) {
            // Keep the old tiles until the new ones have been rendered but some may still be too far away to be of use
            return std::function<bool(const std::unique_ptr<Tile>&)>([&, zoomRatio = zoom / this->zoom](const auto& t) {
                const auto& ext = t->getExtent();
                return isTileWithinEllipse(round_cast<int>(ext.x * zoomRatio), round_cast<int>(ext.y * zoomRatio), c);
            });
        } else {
            return std::function<bool(const std::unique_ptr<Tile>&)>([&](const auto& t) {
                const auto& ext = t->getExtent();
                return isTileWithinEllipse(ext.x, ext.y, c);
            });
        }
    }();

    auto firstUnused = std::partition(this->tiles.begin(), this->tiles.end(), partitionFun);

    RetilingData res;
    // Discarded tiles are kept to avoid reallocation. They may be reused elsewhere. See Tiling::createTiles()
    res.unusedTiles.reserve(as_unsigned(std::distance(firstUnused, this->tiles.end())));
    std::move(firstUnused, this->tiles.end(), std::back_inserter(res.unusedTiles));

    this->tiles.erase(firstUnused, this->tiles.end());

    // Find the range of tile positions that may be missing
    // Using simple integer division may not give the right results if coordinates are non-positives.
    int minX = floor_cast<int>(box.minX * zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int maxX = floor_cast<int>(box.maxX * zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int minY = floor_cast<int>(box.minY * zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int maxY = floor_cast<int>(box.maxY * zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;

    int maxXplusWidth = ceil_cast<int>(extent.maxX * zoom);
    int maxYplusHeight = ceil_cast<int>(extent.maxY * zoom);

    for (int x = minX; x <= maxX; x += MAX_TILE_SIZE) {
        for (int y = minY; y <= maxY; y += MAX_TILE_SIZE) {
            if (isTileWithinEllipse(x, y, c) &&
                (zoomChanged || std::none_of(this->tiles.begin(), this->tiles.end(), [&](const auto& t) {
                     return t->getExtent().x == x && t->getExtent().y == y;
                 }))) {
                res.missingTiles.emplace_back(x, y,
                                             x + MAX_TILE_SIZE > maxXplusWidth ? maxXplusWidth - x : MAX_TILE_SIZE,
                                             y + MAX_TILE_SIZE > maxYplusHeight ? maxYplusHeight - y : MAX_TILE_SIZE);
            }
        }
    }
    this->center = c;
    return res;
}

void Tiling::createTiles(int DPIscaling, RetilingData retiling) {
    tiles.reserve(tiles.size() + retiling.missingTiles.size());
    for (auto&& e: retiling.missingTiles) {
        // We try to find an already allocated tile to avoid the cost of huge allocation
        auto it = std::find_if(retiling.unusedTiles.begin(), retiling.unusedTiles.end(), [&](const auto& t) { return t && t->getExtent().width == e.width && t->getExtent().height == e.height; });
        if (it != retiling.unusedTiles.end()) {
            tiles.emplace_back(std::move(*it));
            tiles.back()->repurpose(e, this->zoom);
        } else {
            tiles.emplace_back(std::make_unique<Tile>(DPIscaling, e, this->zoom, CAIRO_CONTENT_COLOR_ALPHA));
        }
    }
}

void Tiling::append(Tiling& o) {
    if (this->zoom != o.zoom) {
        // Discard old zoom tiles
        std::swap(*this, o);
    } else {
        tiles.insert(tiles.end(), std::make_move_iterator(o.tiles.begin()), std::make_move_iterator(o.tiles.end()));
    }
}

std::vector<Tile*> Tiling::getTilesFor(const Range& rg) {
    xoj::util::Point<int> begin(floor_cast<int>(rg.minX * zoom), floor_cast<int>(rg.minY * zoom));
    xoj::util::Point<int> end(ceil_cast<int>(rg.maxX * zoom), ceil_cast<int>(rg.maxY * zoom));
    xoj::util::Rectangle<int> r(begin.x, begin.y, end.x - begin.x, end.y - begin.y);

    std::vector<Tile*> res;
    for (auto& t: tiles) {
        if (t->getExtent().intersects(r)) {
            res.emplace_back(t.get());
        }
    }
    return res;
}

void Tiling::RetilingData::merge(RetilingData other) {
    auto comp = [](const auto& t1, const auto& t2) { return t1.x < t2.x || (t1.x == t2.x && t1.y < t2.y); };
    xoj_assert(std::is_sorted(this->missingTiles.begin(), this->missingTiles.end(), comp));
    xoj_assert(std::is_sorted(other.missingTiles.begin(), other.missingTiles.end(), comp));

    std::vector<xoj::util::Rectangle<int>> merged;
    merged.reserve(this->missingTiles.size() + other.missingTiles.size());
    std::set_union(this->missingTiles.begin(), this->missingTiles.end(), other.missingTiles.begin(),
                       other.missingTiles.end(), std::back_inserter(merged), comp);
    std::swap(this->missingTiles, merged);

    this->unusedTiles.reserve(this->unusedTiles.size() + other.unusedTiles.size());
    std::move(other.unusedTiles.begin(), other.unusedTiles.end(), std::back_inserter(this->unusedTiles));
}

