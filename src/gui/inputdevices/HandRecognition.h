/*
 * Xournal++
 *
 * Helper class for Touch specific fixes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gdk/gdk.h>

#include "InputContext.h"
#include "InputEvents.h"
#include "XournalType.h"

class Settings;
class TouchDisableInterface;

class HandRecognition {
public:
    HandRecognition(GtkWidget* widget, InputContext* inputContext, Settings* settings);
    virtual ~HandRecognition();

public:
    /**
     * Reload settings
     */
    void reload();

    /**
     * An event from a device occurred
     */
    void event(GdkDevice* device);

    /**
     * An event from a device occurred
     */
    void event(InputDeviceClass device);

    /**
     * Unblock the touchscreen immediately
     */
    void unblock();

private:
    /**
     * Called after the timeout
     *
     * @return true to call again
     */
    static bool enableTimeout(HandRecognition* self);

    /**
     * There was a pen event, restart the timer
     */
    void penEvent();

    /**
     * Enable touchscreen
     */
    void enableTouch();

    /**
     * Disable touchscreen
     */
    void disableTouch();

private:
    /**
     * If touch disabling is enabled
     */
    bool enabled = false;

    /**
     * true if touch is enabled, false if disabled
     */
    bool touchState = true;

    /**
     * When the pen last was seen
     */
    gint64 lastPenAction = -1;

    /**
     * Timeout in ms
     */
    int disableTimeout = 500;

    /**
     * True if an X11 session is running
     */
    bool x11Session = false;

    /**
     * Implementation for touch disabling
     */
    TouchDisableInterface* touchImpl = nullptr;

    /**
     * XournalView
     */
    GtkWidget* widget;

    /**
     * InputContext
     */
    InputContext* inputContext;

    /**
     * Settings
     */
    Settings* settings = nullptr;
};
