#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <vector>

#include "../dialog/ScribbleGtkTest.h"
#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/XournalMain.h"
#include "control/settings/Settings.h"
#include "control/tools/ScribbleGestureRecognizer.h"
#include "control/xojfile/LoadHandler.h"
#include "control/zoom/ZoomControl.h"
#include "gui/GladeSearchpath.h"
#include "gui/Layout.h"
#include "gui/MainWindow.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/inputdevices/InputContext.h"
#include "gui/inputdevices/InputEvents.h"
#include "gui/inputdevices/MouseInputHandler.h"
#include "gui/inputdevices/StylusInputHandler.h"
#include "gui/scroll/ScrollHandling.h"
#include "gui/widgets/XournalWidget.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/UndoRedoHandler.h"

#include "config-test.h"

namespace {
using Path = std::vector<Point>;

auto scrub(double start, double end, int samplesPerPass = 24, double height = 6, bool irregular = false) -> Path {
    Path path;
    const int count = static_cast<int>(std::ceil((end - start) * samplesPerPass));
    for (int i = 0; i <= count; ++i) {
        const double phase = std::numbers::pi * (start + (end - start) * i / count);
        const double width = irregular ? 100 * (0.92 + 0.08 * std::sin(phase * 0.27)) : 100;
        path.emplace_back(220 + width * std::cos(phase) + (irregular ? 2 * std::sin(phase * 4.3) : 0),
                          200 + height * std::sin(phase) + phase * 0.4 + (irregular ? std::sin(phase * 7) : 0));
    }
    return path;
}

class ScribbleInputTest: public ScribbleGtkTest {
    void runTest(GtkApplication*) override {
        XournalMain::initLocalisation();
        auto& control = createControl();
        auto& window = createWindow();
        control.enableAutosave(false);
        control.getSettings()->setEmptyLastPageAppend(EmptyLastPageAppendType::Disabled);
        control.getSettings()->setStabilizerPreprocessor(StrokeStabilizer::Preprocessor::INERTIA);
        control.getSettings()->setScribbleToEraseEnabled(true);
        control.selectTool(TOOL_PEN);
        control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
        control.newFile();
        auto* view = window.getXournal();
        auto* pageView = view->getViewFor(0);
        ASSERT_NE(pageView, nullptr);
        auto page = pageView->getPage();
        auto* layer = page->getSelectedLayer();
        auto* history = control.getUndoRedoHandler();
        auto* widget = GTK_XOURNAL(view->getWidget());
        widget->layout->recalculate();
        auto* device = gdk_seat_get_pointer(gdk_display_get_default_seat(gdk_display_get_default()));
        ASSERT_NE(device, nullptr);
        MouseInputHandler mouse(widget->input);
        StylusInputHandler stylus(widget->input);
        gtk_widget_realize(GTK_WIDGET(window.getWindow()));
        gtk_widget_realize(GTK_WIDGET(widget));
        ASSERT_NE(gtk_widget_get_window(GTK_WIDGET(widget)), nullptr);
        bool throughGtk = false;
        guint32 clock = 1000;
        auto draw = [&](AbstractInputHandler& handler, const Path& points, bool pen, bool cancel = false) {
            const auto pagePosition = pageView->getPixelPosition();
            const double zoom = view->getZoom();
            for (size_t i = 0; i <= points.size(); ++i) {
                const size_t index = std::min(i, points.size() - 1);
                InputEvent event;
                event.device = device;
                event.deviceId = DeviceId(device);
                event.deviceClass = pen ? INPUT_DEVICE_PEN : INPUT_DEVICE_MOUSE;
                event.deviceName = "Scribble input regression";
                event.type = i == 0 ? BUTTON_PRESS_EVENT : (i == points.size() ? BUTTON_RELEASE_EVENT : MOTION_EVENT);
                event.button = 1;
                event.state = i == 0 || i == points.size() ? GdkModifierType(0) : GDK_BUTTON1_MASK;
                event.pressure = pen ? (i == points.size() ? 0.0 : 0.5) : Point::NO_PRESSURE;
                event.relative = {pagePosition.x + points[index].x * zoom, pagePosition.y + points[index].y * zoom};
                event.absolute = event.relative - view->getScrollHandling()->getPosition();
                event.timestamp = clock += 16;
                if (cancel && i == points.size()) {
                    pageView->onSequenceCancelEvent(event.deviceId);
                }
                if (!throughGtk) {
                    handler.handle(event);
                    continue;
                }
                // Exercise the connected widget event signal, InputContext dispatch
                // and GDK-to-input translation, including mouse/pen device mapping.
                auto* native = gdk_event_new(i == 0 ? GDK_BUTTON_PRESS :
                                                      (i == points.size() ? GDK_BUTTON_RELEASE : GDK_MOTION_NOTIFY));
                native->any.window = GDK_WINDOW(g_object_ref(gtk_widget_get_window(GTK_WIDGET(widget))));
                native->any.send_event = true;
                if (native->type == GDK_MOTION_NOTIFY) {
                    native->motion.x = event.absolute.x;
                    native->motion.y = event.absolute.y;
                    native->motion.state = event.state;
                    native->motion.time = event.timestamp;
                } else {
                    native->button.x = event.absolute.x;
                    native->button.y = event.absolute.y;
                    native->button.state = event.state;
                    native->button.time = event.timestamp;
                    native->button.button = 1;
                }
                gdk_event_set_device(native, device);
                gdk_event_set_source_device(native, device);
                gtk_widget_event(GTK_WIDGET(widget), native);
                gdk_event_free(native);
            }
        };
        unsigned positiveCases = 0;
        unsigned preservationCases = 0;
        for (double zoom: {0.25, 1.0, 4.0}) {
            control.getZoomControl()->setZoom(zoom);
            widget->layout->recalculate();
            for (bool pen: {false, true}) {
                auto& input =
                        pen ? static_cast<AbstractInputHandler&>(stylus) : static_cast<AbstractInputHandler&>(mouse);
                for (double angle: {0.0, std::numbers::pi / 4, std::numbers::pi / 2}) {
                    SCOPED_TRACE(testing::Message() << "zoom=" << zoom << " pen=" << pen << " angle=" << angle);
                    auto rotate = [angle](const Path& path) {
                        Path rotated;
                        for (const Point& point: path) {
                            const double x = point.x - 220;
                            const double y = point.y - 200;
                            rotated.emplace_back(220 + x * std::cos(angle) - y * std::sin(angle),
                                                 200 + x * std::sin(angle) + y * std::cos(angle));
                        }
                        return rotated;
                    };
                    auto reset = [&]() {
                        history->clearContents();
                        layer->getElements().clear();
                        control.selectTool(TOOL_PEN);
                        control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
                        control.getSettings()->setScribbleToEraseEnabled(true);
                        for (double x: {160, 220, 280}) {
                            draw(input, rotate(Path{{x, 150}, {x, 250}}), pen);
                        }
                        EXPECT_EQ(layer->getElements().size(), 3) << "Targets must use device input";
                        history->clearContents();
                    };
                    for (int density: {12, 30}) {
                        for (bool irregular: {false, true}) {
                            for (const auto& ends: {std::array{0.0, 6.0}, std::array{0.0, 6.5}, std::array{0.5, 6.5},
                                                    std::array{0.5, 6.0}}) {
                                SCOPED_TRACE(testing::Message()
                                             << "zoom=" << zoom << " pen=" << pen << " angle=" << angle
                                             << " density=" << density << " start=" << ends[0] << " end=" << ends[1]);
                                reset();
                                SCOPED_TRACE(irregular);
                                const auto path =
                                        rotate(scrub(ends[0], ends[1], density, irregular ? 18 : 6, irregular));
                                draw(input, path, pen);
                                EXPECT_EQ(layer->getElements().size(), 0) << "Erase all targets and discard the scrub";
                                history->undo();
                                EXPECT_EQ(layer->getElements().size(), 3);
                                EXPECT_FALSE(history->canUndo()) << "One Undo must restore the complete gesture";
                                history->redo();
                                EXPECT_TRUE(layer->getElements().empty());
                                ++positiveCases;
                            }
                        }
                    }
                    const std::vector<Path> negatives{
                            {{120, 200}, {320, 200}},
                            {{120, 150}, {320, 250}, {220, 200}, {320, 150}, {120, 250}},
                            {{120, 150}, {170, 250}, {220, 150}, {270, 250}, {320, 150}},
                            {{120, 250}, {120, 150}, {220, 230}, {320, 150}, {320, 250}},
                            {{120, 170}, {320, 170}, {120, 200}, {320, 200}, {120, 230}, {320, 230}},
                            scrub(0, 3.5),
                    };
                    for (const auto& path: negatives) {
                        reset();
                        draw(input, rotate(path), pen);
                        EXPECT_EQ(layer->getElements().size(), 4) << "Ordinary drawing must preserve target ink";
                        history->undo();
                        EXPECT_EQ(layer->getElements().size(), 3);
                        EXPECT_FALSE(history->canUndo());
                        ++preservationCases;
                    }
                    reset();
                    control.getSettings()->setScribbleToEraseEnabled(false);
                    draw(input, rotate(scrub(0, 6.5)), pen);
                    EXPECT_EQ(layer->getElements().size(), 4);
                    ++preservationCases;
                    reset();
                    draw(input, rotate(scrub(0, 6.5)), pen, true);
                    EXPECT_EQ(layer->getElements().size(), 3);
                    EXPECT_FALSE(history->canUndo());
                    ++preservationCases;
                }
            }
        }
        auto recorded = LoadHandler{}.loadDocument(GET_TESTFILE(u8"scribble/uneven-scrubs.xopp"));
        ASSERT_NE(recorded, nullptr);
        window.show(nullptr);
        for (int i = 0; i < 100 && gtk_events_pending(); ++i) {
            gtk_main_iteration_do(false);
        }
        throughGtk = true;
        unsigned recordedCases = 0;
        for (auto stabilizer: {StrokeStabilizer::Preprocessor::NONE, StrokeStabilizer::Preprocessor::INERTIA}) {
            control.getSettings()->setStabilizerPreprocessor(stabilizer);
            for (double zoom: {0.25, 1.0, 4.0}) {
                control.getZoomControl()->setZoom(zoom);
                widget->layout->recalculate();
                for (bool pen: {false, true}) {
                    control.getSettings()->setDeviceClassForDevice(
                            device, pen ? InputDeviceTypeOption::Pen : InputDeviceTypeOption::Mouse);
                    auto& input = pen ? static_cast<AbstractInputHandler&>(stylus) :
                                        static_cast<AbstractInputHandler&>(mouse);
                    for (double angle: {0.0, std::numbers::pi / 4, std::numbers::pi / 2}) {
                        size_t sample = 0;
                        for (const auto& element: recorded->getPage(0)->getSelectedLayer()->getElements()) {
                            SCOPED_TRACE(testing::Message()
                                         << "recorded=" << sample++ << " zoom=" << zoom << " pen=" << pen
                                         << " angle=" << angle << " stabilizer=" << static_cast<int>(stabilizer));
                            history->clearContents();
                            layer->getElements().clear();
                            control.selectTool(TOOL_PEN);
                            control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
                            control.getSettings()->setScribbleToEraseEnabled(true);
                            const auto& stroke = static_cast<const Stroke&>(*element);
                            const auto box = stroke.getBoundingBox();
                            const double centerX = box.x + box.width / 2;
                            const double centerY = box.y + box.height / 2;
                            auto transform = [&](const Path& path) {
                                Path result;
                                for (const auto& p: path) {
                                    const double x = p.x - centerX;
                                    const double y = p.y - centerY;
                                    result.emplace_back(220 + x * std::cos(angle) - y * std::sin(angle),
                                                        200 + x * std::sin(angle) + y * std::cos(angle));
                                }
                                return result;
                            };
                            draw(input, transform(Path{{centerX, -20}, {centerX, box.y + box.height + 20}}), pen);
                            draw(input, Path{{400, 100}, {400, 300}}, pen);
                            ASSERT_EQ(layer->getElements().size(), 2);
                            history->clearContents();
                            draw(input, transform(stroke.getPointVector()), pen);
                            if (layer->getElements().size() == 3) {
                                const auto& failed = static_cast<const Stroke&>(*layer->getElements().back());
                                EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(failed.getPointVector()))
                                        << "Final input point count: " << failed.getPointCount();
                            }
                            EXPECT_EQ(layer->getElements().size(), 1) << "Erase crossed ink, preserve distant ink";
                            history->undo();
                            EXPECT_EQ(layer->getElements().size(), 2);
                            EXPECT_FALSE(history->canUndo());
                            history->redo();
                            EXPECT_EQ(layer->getElements().size(), 1);
                            ++recordedCases;
                        }
                    }
                }
            }
        }
        // Saved document points have already passed through stabilization.
        // Replay them without applying the filter a second time.
        control.getSettings()->setStabilizerPreprocessor(StrokeStabilizer::Preprocessor::NONE);
        control.getZoomControl()->setZoom(1);
        widget->layout->recalculate();
        for (bool pen: {false, true}) {
            SCOPED_TRACE(testing::Message() << "saved failed page, pen=" << pen);
            history->clearContents();
            layer->getElements().clear();
            control.selectTool(TOOL_PEN);
            control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
            control.getSettings()->setDeviceClassForDevice(
                    device, pen ? InputDeviceTypeOption::Pen : InputDeviceTypeOption::Mouse);
            auto& input = pen ? static_cast<AbstractInputHandler&>(stylus) : static_cast<AbstractInputHandler&>(mouse);
            control.getSettings()->setScribbleToEraseEnabled(false);
            const auto& saved = recorded->getPage(2)->getSelectedLayer()->getElements();
            for (const auto& element: saved) {
                draw(input, static_cast<const Stroke&>(*element).getPointVector(), pen);
            }
            ASSERT_EQ(layer->getElements().size(), saved.size());
            const auto* left = layer->getElements()[0].get();
            const auto* middle = layer->getElements()[1].get();
            const auto* right = layer->getElements()[2].get();
            history->clearContents();
            control.getSettings()->setScribbleToEraseEnabled(true);
            draw(input, static_cast<const Stroke&>(*saved.back()).getPointVector(), pen);
            EXPECT_EQ(layer->indexOf(middle), Element::InvalidIndex);
            EXPECT_NE(layer->indexOf(left), Element::InvalidIndex);
            EXPECT_NE(layer->indexOf(right), Element::InvalidIndex);
            const auto remaining = layer->getElements().size();
            EXPECT_LT(remaining, saved.size());
            history->undo();
            EXPECT_EQ(layer->getElements().size(), saved.size());
            EXPECT_NE(layer->indexOf(middle), Element::InvalidIndex);
            EXPECT_FALSE(history->canUndo());
            history->redo();
            EXPECT_EQ(layer->getElements().size(), remaining);
            ++recordedCases;
        }

        // Actual widget/GDK input: shape-created ink, moving scrubs and saved runtime options.
        unsigned advancedCases = 0;
        const auto originalShapeMinimum = control.getSettings()->getStrokeRecognizerMinSize();
        control.getSettings()->setStrokeRecognizerMinSize(10);
        control.getSettings()->setStabilizerPreprocessor(StrokeStabilizer::Preprocessor::NONE);
        for (double zoom: {0.25, 1.0, 4.0}) {
            control.getZoomControl()->setZoom(zoom);
            widget->layout->recalculate();
            for (bool pen: {false, true}) {
                control.getSettings()->setDeviceClassForDevice(
                        device, pen ? InputDeviceTypeOption::Pen : InputDeviceTypeOption::Mouse);
                auto& input =
                        pen ? static_cast<AbstractInputHandler&>(stylus) : static_cast<AbstractInputHandler&>(mouse);
                for (auto mode: {DRAWING_TYPE_DEFAULT, DRAWING_TYPE_SHAPE_RECOGNIZER}) {
                    for (int targetKind = 0; targetKind < 3; ++targetKind) {
                        SCOPED_TRACE(testing::Message() << "moving targets kind=" << targetKind << " mode=" << mode
                                                        << " pen=" << pen << " zoom=" << zoom);
                        history->clearContents();
                        layer->getElements().clear();
                        control.selectTool(TOOL_PEN);
                        control.getSettings()->setScribbleToEraseEnabled(false);
                        control.getSettings()->setScribbleSensitivity(50);
                        control.getSettings()->setScribbleMinimumPasses(4);
                        control.getToolHandler()->setDrawingType(targetKind ? DRAWING_TYPE_SHAPE_RECOGNIZER :
                                                                              DRAWING_TYPE_DEFAULT);
                        for (double x: {115, 135, 155, 175, 195, 215}) {
                            Path target;
                            const Path vertices = targetKind == 2 ? Path{{x - 10, 190},
                                                                         {x + 10, 190},
                                                                         {x + 10, 230},
                                                                         {x - 10, 230},
                                                                         {x - 10, 190}} :
                                                                    Path{{x - 10, 210}, {x + 10, 210}};
                            for (size_t edge = 1; edge < vertices.size(); ++edge) {
                                for (int i = 0; i < 30; ++i) {
                                    const double t = static_cast<double>(i) / 29;
                                    target.emplace_back(std::lerp(vertices[edge - 1].x, vertices[edge].x, t),
                                                        std::lerp(vertices[edge - 1].y, vertices[edge].y, t));
                                }
                            }
                            draw(input, target, pen);
                            if (targetKind > 0) {
                                EXPECT_LE(static_cast<const Stroke&>(*layer->getElements().back()).getPointCount(),
                                          targetKind == 1 ? 3 : 6)
                                        << "Target must actually pass through shape recognition";
                            }
                        }
                        control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
                        draw(input, Path{{350, 170}, {350, 260}}, pen);
                        ASSERT_EQ(layer->getElements().size(), 7);
                        const auto* untouched = layer->getElements().back().get();
                        history->clearContents();
                        control.getToolHandler()->setDrawingType(mode);
                        control.getSettings()->setScribbleToEraseEnabled(true);
                        Path moving;
                        for (int i = 0; i <= 12; ++i) moving.emplace_back(100.0 + 10 * i, i % 2 ? 240.0 : 180.0);
                        draw(input, moving, pen);
                        ASSERT_EQ(layer->getElements().size(), 1);
                        EXPECT_EQ(layer->getElements().front().get(), untouched);
                        history->undo();
                        EXPECT_EQ(layer->getElements().size(), 7);
                        EXPECT_FALSE(history->canUndo());
                        history->redo();
                        EXPECT_EQ(layer->getElements().size(), 1);
                        ++advancedCases;
                    }
                }
            }
        }
        control.getZoomControl()->setZoom(1);
        widget->layout->recalculate();
        for (bool pen: {false, true}) {
            control.getSettings()->setDeviceClassForDevice(
                    device, pen ? InputDeviceTypeOption::Pen : InputDeviceTypeOption::Mouse);
            auto& input = pen ? static_cast<AbstractInputHandler&>(stylus) : static_cast<AbstractInputHandler&>(mouse);
            for (auto mode: {DRAWING_TYPE_DEFAULT, DRAWING_TYPE_SHAPE_RECOGNIZER}) {
                for (int minimum: {4, 2}) {
                    SCOPED_TRACE(testing::Message()
                                 << "configured short scrub minimum=" << minimum << " mode=" << mode << " pen=" << pen);
                    history->clearContents();
                    layer->getElements().clear();
                    control.getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
                    control.getSettings()->setScribbleToEraseEnabled(false);
                    draw(input, Path{{220, 150}, {220, 250}}, pen);
                    history->clearContents();
                    control.getToolHandler()->setDrawingType(mode);
                    control.getSettings()->setScribbleSensitivity(75);
                    control.getSettings()->setScribbleMinimumPasses(minimum);
                    control.getSettings()->setScribbleToEraseEnabled(true);
                    draw(input, Path{{160, 200}, {280, 202}, {160, 204}}, pen);
                    EXPECT_EQ(layer->getElements().empty(), minimum == 2);
                    ++advancedCases;
                }
            }
        }
        control.getSettings()->setScribbleSensitivity(50);
        control.getSettings()->setScribbleMinimumPasses(4);
        control.getSettings()->setStrokeRecognizerMinSize(originalShapeMinimum);
        RecordProperty("advanced_gdk_scenarios", static_cast<int>(advancedCases));
        RecordProperty("recorded_gdk_scenarios", static_cast<int>(recordedCases));
        RecordProperty("positive_scenarios", static_cast<int>(positiveCases));
        RecordProperty("preservation_scenarios", static_cast<int>(preservationCases));
        history->clearContents();
    }
};
}  // namespace

TEST_F(ScribbleInputTest, deviceEventsDrawTargetsEraseAndUndoWithNaturalEndpoints) {}
