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


#include <set>
#include <string>
#include <vector>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "gui/XournalView.h"
#include "gui/scroll/ScrollHandling.h"
#include "gui/widgets/XournalWidget.h"

#include "AbstractInputHandler.h"
#include "HandRecognition.h"
#include "KeyboardInputHandler.h"
#include "MouseInputHandler.h"
#include "StylusInputHandler.h"
#include "TouchDrawingInputHandler.h"
#include "TouchInputHandler.h"
#include "XournalType.h"
#include "config-debug.h"

class InputContext {

private:
    StylusInputHandler* stylusHandler;
    MouseInputHandler* mouseHandler;
    TouchDrawingInputHandler* touchDrawingHandler;
    KeyboardInputHandler* keyboardHandler;
    TouchInputHandler* touchHandler;

    GtkWidget* widget = nullptr;
    XournalView* view;
    ScrollHandling* scrollHandling;

    GdkModifierType modifierState = (GdkModifierType)0;

    bool touchWorkaroundEnabled = false;

    std::set<string> knownDevices;

public:
    enum DeviceType {
        MOUSE,
        STYLUS,
        TOUCHSCREEN,
    };

public:
    InputContext(XournalView* view, ScrollHandling* scrollHandling);
    ~InputContext();

private:
    /**
     * Callback used by Glib to notify for new events
     * @param widget The widget the event happened in
     * @param event The event
     * @param self A pointer to our handler
     * @return Whether the event was handled
     */
    static bool eventCallback(GtkWidget* widget, GdkEvent* event, InputContext* self);

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
     */
    void connect(GtkWidget* widget);

    GtkXournal* getXournal();
    XournalView* getView();
    ToolHandler* getToolHandler();
    Settings* getSettings();
    ScrollHandling* getScrollHandling();

    GdkModifierType getModifierState();
    void focusWidget();
    void blockDevice(DeviceType deviceType);
    void unblockDevice(DeviceType deviceType);
    bool isBlocked(DeviceType deviceType);
};
