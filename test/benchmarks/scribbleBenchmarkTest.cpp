#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/settings/Settings.h"
#include "control/tools/StrokeHandler.h"
#include "control/tools/StrokeStabilizerEnum.h"
#include "control/xojfile/LoadHandler.h"
#include "gui/GladeSearchpath.h"
#include "gui/inputdevices/PositionInputData.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/UndoRedoHandler.h"

namespace {
class BenchmarkControl: public Control {
public:
    using Control::Control;
    void undoRedoChanged() override {}
};

void measure(size_t population, size_t targets, bool candidate, bool enabled, bool recorded) {
    GladeSearchpath resources;
    resources.addSearchDirectory(GET_RESOURCE_TEMPLATE_FOLDER);
    BenchmarkControl control(nullptr, &resources);
    auto* settings = control.getSettings();
    settings->setScribbleToEraseEnabled(enabled);
    settings->setEmptyLastPageAppend(EmptyLastPageAppendType::Disabled);
    settings->setStabilizerAveragingMethod(StrokeStabilizer::AveragingMethod::NONE);
    settings->setStabilizerPreprocessor(StrokeStabilizer::Preprocessor::NONE);
    control.getToolHandler()->selectTool(TOOL_PEN);
    control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
    auto page = std::make_shared<XojPage>(600, 800);
    control.getDocument()->addPage(page);
    page->setSelectedLayerId(1);
    auto* layer = page->getSelectedLayer();
    for (size_t i = 0; i < population; ++i) {
        auto stroke = std::make_unique<Stroke>();
        stroke->setToolType(StrokeTool::PEN);
        stroke->setWidth(1);
        stroke->setColor(Colors::black);
        stroke->setFill(-1);
        // Spread contacts through the layer, rather than making undo an append-only best case.
        const bool hit = targets && i % (population / targets) == 0 && i / (population / targets) < targets;
        const double x = hit ? 80 : 300;
        stroke->setPointVector({{x, 20}, {x, 100}});
        layer->addElement(std::move(stroke));
    }
    std::vector<Point> input =
            candidate ? std::vector<Point>{{40, 40}, {120, 42}, {40, 44}, {120, 46}, {40, 48}, {120, 50}, {40, 52}} :
                        std::vector<Point>{{40, 40}, {50, 42}, {60, 44}, {70, 42}, {80, 40}};
    if (recorded && candidate) {
        auto document = LoadHandler{}.loadDocument(GET_TESTFILE(u8"scribble/uneven-scrubs.xopp"));
        ASSERT_NE(document, nullptr);
        const auto& source = static_cast<const Stroke&>(*document->getPage(0)->getSelectedLayer()->getElements()[2]);
        const auto box = source.getBoundingBox();
        input.clear();
        for (const auto& point: source.getPointVector()) {
            input.emplace_back(80 + point.x - box.x - box.width / 2, 60 + point.y - box.y - box.height / 2);
        }
    }
    std::vector<PositionInputData> events;
    for (size_t i = 0; i < input.size(); ++i) {
        PositionInputData event{};
        event.x = input[i].x;
        event.y = input[i].y;
        event.pressure = Point::NO_PRESSURE;
        event.timestamp = static_cast<guint32>(i * 10);
        events.push_back(event);
    }
    constexpr size_t SAMPLES = 1000;
    constexpr size_t WARMUP = 50;
    std::vector<double> timings;
    timings.reserve(SAMPLES);
    auto* history = control.getUndoRedoHandler();
    for (size_t sample = 0; sample < WARMUP + SAMPLES; ++sample) {
        StrokeHandler handler(&control, page);
        handler.onButtonPressEvent(events.front(), 1);
        for (size_t i = 1; i < events.size(); ++i) {
            handler.onMotionNotifyEvent(events[i], 1);
        }
        const auto start = std::chrono::steady_clock::now();
        handler.onButtonReleaseEvent(events.back(), 1);
        const auto stop = std::chrono::steady_clock::now();
        if (sample >= WARMUP) {
            timings.push_back(std::chrono::duration<double, std::micro>(stop - start).count());
        }
        const auto expected = enabled && candidate && targets ? population - targets : population + 1;
        ASSERT_EQ(layer->getElements().size(), expected);
        history->undo();
        ASSERT_EQ(layer->getElements().size(), population);
        history->clearContents();
    }
    if (const char* path = std::getenv("XOURNALPP_SCRIBBLE_BENCH_SAMPLES")) {
        std::ofstream samples(path, std::ios::app);
        ASSERT_TRUE(samples.is_open());
        for (size_t i = 0; i < timings.size(); ++i) {
            samples << population << ',' << targets << ',' << candidate << ',' << enabled << ',' << i << ','
                    << timings[i] << '\n';
        }
        samples.flush();
        ASSERT_TRUE(samples.good());
    }
    std::sort(timings.begin(), timings.end());
    std::cout << "SCRIBBLE_BENCH," << population << ',' << targets << ',' << candidate << ',' << enabled << ','
              << timings.size() << ',' << timings[499] << ',' << timings[949] << ',' << timings[989] << '\n';
}
void runScenarios(bool recorded) {
    std::cout << "SCRIBBLE_BENCH,population,targets,candidate,enabled,n,p50_us,p95_us,p99_us\n";
    for (size_t population: {0U, 1000U, 10000U, 50000U}) {
        for (bool enabled: {false, true}) {
            measure(population, 0, false, enabled, recorded);
            measure(population, 0, true, enabled, recorded);
            for (size_t targets: {1U, 10U, 100U}) {
                if (targets <= population) {
                    measure(population, targets, true, enabled, recorded);
                }
            }
        }
    }
}

}  // namespace

TEST(ScribbleBenchmark, penUpLatencyWithoutWindowOrRepaint) { runScenarios(false); }

TEST(ScribbleBenchmark, recordedScrubPenUpLatencyWithoutWindowOrRepaint) { runScenarios(true); }
