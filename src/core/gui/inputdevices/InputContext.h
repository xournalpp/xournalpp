/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <memory>  // for unique_ptr
#include <optional>
#include <set>     // for set
#include <string>  // for string

#include <gdk/gdk.h>  // for GdkEvent, GdkModifierType
#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget

#include "gui/widgets/XournalWidget.h"  // for GtkXournal

class GeometryToolInputHandler;
class KeyboardInputHandler;
class MouseInputHandler;
class ScrollHandling;
class Settings;
class StylusInputHandler;
class ToolHandler;
class TouchDrawingInputHandler;
class TouchInputHandler;
class XournalView;
class DeviceTestingArea;
class HandRecognition;

class InputContext final {

private:
    XournalView* view;
    ScrollHandling* scrollHandling;
    Settings* settings;

    gulong signal_id{0};
    std::unique_ptr<StylusInputHandler> stylusHandler;
    std::unique_ptr<MouseInputHandler> mouseHandler;
    std::unique_ptr<TouchDrawingInputHandler> touchDrawingHandler;
    std::unique_ptr<KeyboardInputHandler> keyboardHandler;
    std::unique_ptr<TouchInputHandler> touchHandler;
    std::unique_ptr<GeometryToolInputHandler> geometryToolInputHandler;

    /**
     * Helper class for Touch specific fixes
     */
    std::unique_ptr<HandRecognition> handRecognition;

    GtkWidget* widget = nullptr;

    std::set<std::string> knownDevices;

public:
    enum DeviceType {
        MOUSE,
        STYLUS,
        TOUCHSCREEN,
    };

public:
    InputContext(XournalView* view, ScrollHandling* scrollHandling);
    ~InputContext();

    /// Make an input context for testing purposes
    InputContext(Settings* settings, DeviceTestingArea& testing);

private:
    /**
     * Handle the events
     * @param event The event to handle
     * @return Whether the event was handled
     */
    bool handle(GdkEvent* event);

    /**
     * Print debug output
     */
    void printDebug(GdkEvent* event);

public:
    /**
     * Connect the input handling to the window to receive events
     *
     * If logfunction is provided, it will be called on all the received events
     */
    void connect(GtkWidget* widget, bool connectKeyboardHandler = true,
                 std::optional<std::function<void(GdkEvent*)>> logfunction = std::nullopt);

    GtkXournal* getXournal() const;
    XournalView* getView() const;
    ToolHandler* getToolHandler() const;
    Settings* getSettings() const;
    HandRecognition* getHandRecognition() const;
    ScrollHandling* getScrollHandling() const;
    void setGeometryToolInputHandler(std::unique_ptr<GeometryToolInputHandler> handler);
    GeometryToolInputHandler* getGeometryToolInputHandler() const;
    void resetGeometryToolInputHandler();

    void focusWidget();
    void blockDevice(DeviceType deviceType);
    void unblockDevice(DeviceType deviceType);
    bool isBlocked(DeviceType deviceType);
};
