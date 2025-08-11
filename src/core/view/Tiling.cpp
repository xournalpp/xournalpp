#include "Tiling.h"

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

static constexpr int MAX_TILE_SIZE = 2000;              ///< Each tile has width/height <= MAX_TILE_SIZE
static constexpr int ELLIPSE_VERTICAL_RADIUS = 6000;    ///< In pixels
static constexpr int ELLIPSE_HORIZONTAL_RADIUS = 4000;  ///< In pixels

static constexpr int RETILE_THRESHOLD =
        MAX_TILE_SIZE / 10;  ///< Smaller movements do not trigger an update of the tiles. In pixels


#ifdef DEBUG_TILING
#define IF_DBG_TILING(f) f
static void printTilingSize(const std::vector<std::unique_ptr<xoj::view::Tile>>& tiles) {
    size_t sizeInMem = 0;
    for (auto&& t: tiles) {
        sizeInMem += strict_cast<size_t>(t->getExtent().width) * strict_cast<size_t>(t->getExtent().height);
    }
    sizeInMem *= 4;  // 4 bytes per pixel

    if (!tiles.empty()) {
        // Multiply by DPI scaling if any
        auto* surf = cairo_get_target(tiles.front()->get());
        double sx, sy;
        cairo_surface_get_device_scale(surf, &sx, &sy);
        sizeInMem *= round_cast<size_t>(sx) * round_cast<size_t>(sy);
    }

    printf("tiles: %zu - mem %f MB\n", tiles.size(), static_cast<double>(sizeInMem) / 1e6);
}
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

void Tiling::populate(int DPIscaling, const xoj::util::Point<double>& c, const Range& extent, double zoom) {
    this->zoom = zoom;
    this->createTiles(DPIscaling, this->recenterAndGetMissingTiles(c, extent));
    IF_DBG_TILING(printTilingSize(this->tiles));
}

auto Tiling::recenterAndGetMissingTiles(const xoj::util::Point<double>& p, const Range& extent)
        -> std::vector<xoj::util::Rectangle<int>> {
    xoj::util::Point<int> c(round_cast<int>(p.x * this->zoom), round_cast<int>(p.y * this->zoom));

    if (!this->tiles.empty() && this->center.distance(c) < RETILE_THRESHOLD) {
        return {};
    }

    this->tiles.erase(std::remove_if(this->tiles.begin(), this->tiles.end(),
                                     [&](auto&& t) {
                                         const auto& ext = t->getExtent();
                                         return !isTileWithinEllipse(ext.x, ext.y, c);
                                     }),
                      this->tiles.end());

    auto box = Range(p.x - ELLIPSE_HORIZONTAL_RADIUS / this->zoom, p.y - ELLIPSE_VERTICAL_RADIUS / this->zoom,
                     p.x + ELLIPSE_HORIZONTAL_RADIUS / this->zoom, p.y + ELLIPSE_VERTICAL_RADIUS / this->zoom)
                       .intersect(extent);

    if (box.empty()) [[unlikely]] {
        // The ellipse is entirely out of the page
        return {};
    }

    // Find the range of tile positions that may be missing
    // Using simple integer division may not give the right results if coordinates are non-positives.
    int minX = floor_cast<int>(box.minX * this->zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int maxX = floor_cast<int>(box.maxX * this->zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int minY = floor_cast<int>(box.minY * this->zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;
    int maxY = floor_cast<int>(box.maxY * this->zoom / MAX_TILE_SIZE) * MAX_TILE_SIZE;

    int maxXplusWidth = ceil_cast<int>(extent.maxX * this->zoom);
    int maxYplusHeight = ceil_cast<int>(extent.maxY * this->zoom);

    std::vector<xoj::util::Rectangle<int>> newTilesExtents;
    for (int x = minX; x <= maxX; x += MAX_TILE_SIZE) {
        for (int y = minY; y <= maxY; y += MAX_TILE_SIZE) {
            if (isTileWithinEllipse(x, y, c) &&
                std::none_of(this->tiles.begin(), this->tiles.end(),
                             [&](const auto& t) { return t->getExtent().x == x && t->getExtent().y == y; })) {
                newTilesExtents.emplace_back(x, y,
                                             x + MAX_TILE_SIZE > maxXplusWidth ? maxXplusWidth - x : MAX_TILE_SIZE,
                                             y + MAX_TILE_SIZE > maxYplusHeight ? maxYplusHeight - y : MAX_TILE_SIZE);
            }
        }
    }
    this->center = c;
    return newTilesExtents;
}

void Tiling::createTiles(int DPIscaling, const std::vector<xoj::util::Rectangle<int>>& extents) {
    tiles.reserve(tiles.size() + extents.size());
    for (auto&& e: extents) {
        tiles.emplace_back(std::make_unique<Tile>(DPIscaling, e, this->zoom, CAIRO_CONTENT_COLOR_ALPHA));
    }
}

void Tiling::append(Tiling& o) {
    tiles.insert(tiles.end(), std::make_move_iterator(o.tiles.begin()), std::make_move_iterator(o.tiles.end()));
    IF_DBG_TILING(printTilingSize(this->tiles));
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
