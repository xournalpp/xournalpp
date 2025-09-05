/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <algorithm>
#include <array>
#include <execution>

#include <cairo-script.h>
#include <cairo-tee.h>
#include <cairo.h>
#include <config-test.h>
#include <glib-2.0/glib.h>
#include <gtest/gtest.h>

#include "control/xojfile/LoadHandler.h"
#include "model/PageRef.h"
#include "model/XojPage.h"
#include "view/DocumentView.h"
#include "view/Mask.h"
#include "view/Tiling.h"

static constexpr int DPI_SCALE_FACTOR = 1;

struct BenchCase {
    double zoom;
    double renderAreaRadius;
};
static constexpr std::array<BenchCase, 10> BENCH_CASES{
        BenchCase{2., 2.}, BenchCase{4., 2.},  BenchCase{7., 2.},  BenchCase{12., 2.}, BenchCase{16., 2.},
        BenchCase{7., 1.}, BenchCase{12., 1.}, BenchCase{16., 1.}, BenchCase{12., .7}, BenchCase{16., .7}};
static constexpr int NB_ITERATIONS = 5;

template <auto fun>
static void runBench(const PageRef& page) {
    auto time = g_get_monotonic_time();
    for (auto c: BENCH_CASES) {
        xoj::view::Tiling tiles;
        tiles.populate(DPI_SCALE_FACTOR, {.5 * page->getWidth(), .5 * page->getHeight()},
                       Range(0, 0, page->getWidth(), page->getHeight()), c.renderAreaRadius * .5 * page->getHeight(),
                       c.zoom, {});

        auto t = g_get_monotonic_time();
        for (int i = 0; i < NB_ITERATIONS; i++) {
            fun(tiles, page, c.zoom);
        }
        printf(u8" -> Rendered %2zu tiles %d times in %8ld µs (zoom %f)\n", tiles.getTiles().size(), NB_ITERATIONS,
               g_get_monotonic_time() - t, c.zoom);
    }
    printf("   ------- Done in %8ld ms\n", (g_get_monotonic_time() - time) / 1000);
}

static inline void renderToBuffer(const PageRef& page, cairo_t* cr, bool parallelExecution) {
    DocumentView().drawPage(page, cr, false, xoj::view::BACKGROUND_SHOW_ALL, parallelExecution);
}

static void renderUsingCairoTee(xoj::view::Tiling& tiles, const PageRef& page, double zoom) {
    xoj::util::Rectangle<int> e = tiles.getTiles().front()->getExtent();
    for (auto&& t: tiles.getTiles()) {
        cairo_surface_set_device_offset(cairo_get_target(t->get()), -t->getExtent().x, -t->getExtent().y);
        e.unite(t->getExtent());
    }
    cairo_rectangle_t r{static_cast<double>(e.x), static_cast<double>(e.y), static_cast<double>(e.width),
                        static_cast<double>(e.height)};
    auto* rec = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, &r);

    auto* tee = cairo_tee_surface_create(rec);
    for (auto it = tiles.getTiles().begin(); it != tiles.getTiles().end(); ++it) {
        cairo_tee_surface_add(tee, cairo_get_target((*it)->get()));
    }
    cairo_t* c = cairo_create(tee);
    cairo_scale(c, tiles.getZoom(), tiles.getZoom());

    renderToBuffer(page, c, false);

    cairo_destroy(c);
    cairo_surface_destroy(tee);
    cairo_surface_destroy(rec);

    for (auto&& t: tiles.getTiles()) {
        cairo_surface_set_device_offset(cairo_get_target(t->get()), 0, 0);
    }
}

static void renderUsingSequencialLoop(xoj::view::Tiling& tiles, const PageRef& page, double zoom) {
    std::for_each(tiles.getTiles().begin(), tiles.getTiles().end(),
                  [&page](auto&& t) { renderToBuffer(page, t->get(), false); });
}

template <auto fun>
static void bench(const Document& doc) {
    printf("Bench rendering TexImages\n");
    runBench<fun>(doc.getPage(0));
    printf("Bench rendering Strokes\n");
    runBench<fun>(doc.getPage(1));
    printf("Bench rendering Texts\n");
    runBench<fun>(doc.getPage(2));
}

TEST(RenderBenchmark, benchmarkCairoTee) {
    auto doc = LoadHandler().loadDocument(GET_TESTFILE("render-benchmark.xopp"));
    ASSERT_TRUE(doc) << "Unable to load test file \"render-benchmark.xopp\"";
    ASSERT_EQ(doc->getPageCount(), 3);

    bench<renderUsingCairoTee>(*doc);
}

TEST(RenderBenchmark, benchmarkSequencialLoop) {
    auto doc = LoadHandler().loadDocument(GET_TESTFILE("render-benchmark.xopp"));
    ASSERT_TRUE(doc) << "Unable to load test file \"render-benchmark.xopp\"";
    ASSERT_EQ(doc->getPageCount(), 3);

    bench<renderUsingSequencialLoop>(*doc);
}

#ifdef BENCHMARK_USE_STD_EXECUTION
static void renderUsingParallelLoop(xoj::view::Tiling& tiles, const PageRef& page, double zoom) {
    std::for_each(std::execution::par, tiles.getTiles().begin(), tiles.getTiles().end(),
                  [&page](auto&& t) { renderToBuffer(page, t->get(), true); });
}
TEST(RenderBenchmark, benchmarkParallelLoop) {
    auto doc = LoadHandler().loadDocument(GET_TESTFILE("render-benchmark.xopp"));
    ASSERT_TRUE(doc) << "Unable to load test file \"render-benchmark.xopp\"";
    ASSERT_EQ(doc->getPageCount(), 3);

    bench<renderUsingParallelLoop>(*doc);
}
#endif
