/*
 * Xournal++
 *
 * An area to test input devices
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <gtk/gtk.h>  // for GtkWidget, ...

#include "util/raii/GObjectSPtr.h"
#include "util/raii/GSourceURef.h"

class Settings;
class InputContext;
struct InputEvent;
class DeviceClassConfigGui;
class GladeSearchpath;

class DeviceTestingArea {
public:
    DeviceTestingArea(GladeSearchpath* path, GtkBox* parent, Settings* settings);
    ~DeviceTestingArea();

    enum HandlerType { MOUSE = 0, STYLUS = 1, TOUCH = 2 };
    bool handle(const InputEvent& e, HandlerType t);

    void saveSettings() const;

private:
    Settings* settings;
    GladeSearchpath* gladeSearchPath;

    bool emulateTipContactOnButtonPress;

    std::unique_ptr<InputContext> inputContext;
    xoj::util::WidgetSPtr drawingArea;

    /// Indicators for button status
    std::array<xoj::util::WidgetSPtr, 4> mouseIndicators;   ///< 0 = in use, 1 = left, 2 = middle, 3 = right
    std::array<xoj::util::WidgetSPtr, 4> stylusIndicators;  ///< 0 = in use, 1 = tip, 2 = button-1, 3 = button-2
    std::array<xoj::util::WidgetSPtr, 2> eraserIndicators;  ///< 0 = in use, 1 = tip
    std::array<xoj::util::WidgetSPtr, 5> touchIndicators;

    /// Mouse "in use" timer: the mouse stays in use 100ms after its last event
    xoj::util::GSourceURef mouseInUseTimer;
    xoj::util::GSourceURef stylusInUseTimer;
    xoj::util::GSourceURef eraserInUseTimer;

    /// Used to disable kinetic scrolling in order to properly test touchscreens
    xoj::util::WidgetSPtr ancestorScrolledWindow;

    /// Show and edit the configuration of the last device used
    std::unique_ptr<DeviceClassConfigGui> lastDeviceClassConfig;

    struct Data;
    std::unique_ptr<Data> data;
};


template <typename Handler>
class TestingHandler: public Handler {
public:
    explicit TestingHandler(InputContext* context, DeviceTestingArea::HandlerType t, DeviceTestingArea& testing):
            Handler(context), deviceType(t), testing(testing) {}
    ~TestingHandler() = default;

    bool handleImpl(const InputEvent& e) override { return testing.handle(e, deviceType); }

private:
    const DeviceTestingArea::HandlerType deviceType;
    DeviceTestingArea& testing;
};
