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

#include "gui/inputdevices/InputEvents.h"  // for InputEvent
#include "util/Point.h"

#include "AbstractInputHandler.h"  // for AbstractInputHandler

class InputContext;
class PositionInputData;
class XojPageView;

enum class PressureMode { NO_PRESSURE, DEVICE_PRESSURE, INFERRED_PRESSURE };

class PenInputHandler: public AbstractInputHandler {
protected:
    /**
     * Whether the current device class has a device with button 1 in pressed state
     */
    bool deviceClassPressed = false;
    /**
     * modifier variables storing whether button 2 and 3 are pressed
     */
    bool modifier2 = false;
    bool modifier3 = false;

    /**
     * Reference to the last event
     */
    InputEvent lastEvent{};

    PressureMode pressureMode{PressureMode::NO_PRESSURE};

    /**
     * Last pressure measured or inferred.
     */
    double lastPressure{0.0};

    /**
     * Reference to the last event actually hitting a page
     */
    InputEvent lastHitEvent{};

    /**
     * Start position to reference scroll offset
     */
    xoj::util::Point<double> scrollStartPosition{0., 0.};

    /**
     * The scroll offset vector
     */
    xoj::util::Point<double> scrollOffsetVector{0., 0.};

    /**
     * Flag whether pen is within the widget
     */
    bool penInWidget = false;

    /**
     * Page a selection started at as we require this for motion updates
     */
    XojPageView* sequenceStartPage = nullptr;

    /**
     * For tap event filtering. See Preferences->Drawing Area->Action on Tool Tap
     */
    guint32 lastActionEndTimeStamp = 0U;
    guint32 lastActionStartTimeStamp = 0U;
    xoj::util::Point<double> sequenceStartPosition;

public:
    explicit PenInputHandler(InputContext* inputContext);
    ~PenInputHandler() override;

protected:
    /**
     * Action for the start of an input
     * @param event The event triggering the action
     */
    bool actionStart(InputEvent const& event);

    /**
     * Action for motion during an input
     * @param event The event triggering the action
     */
    bool actionMotion(InputEvent const& event);

    /**
     * Action for a discrete input.
     */
    void actionPerform(InputEvent const& event);

    void actionLeaveWindow(InputEvent const& event);
    void actionEnterWindow(InputEvent const& event);

    /**
     * Action for the end of an input
     * @param event The event triggering the action
     */
    bool actionEnd(InputEvent const& event);

    /**
     * @brief Change the tool according to the InputHandler and buttons pressed
     *
     * @param event some Inputevent
     * @return true if tool was changed successfully
     * @return false if tool was not changed successfully (currently only in TouchDrawingInputHandler)
     */
    virtual bool changeTool(InputEvent const& event) = 0;

    /**
     * Do the scrolling with the hand tool
     */
    void handleScrollEvent(InputEvent const& event);

    /**
     * Stores references to the provided event in lastEvent and lastHitEvent
     * Use this to update the references for the last event.
     * lastHitEvent will only be updated when the event did hit a page
     * @param event
     */
    void updateLastEvent(InputEvent const& event);

    /**
     * Estimate pressure based on pen speed using the previous event.
     *
     * @param pos The position of the current event
     * @param page The page the event is relative to.
     * @return The filtered pressure.
     */
    double inferPressureValue(PositionInputData const& pos, XojPageView* page);

    /**
     * @brief Apply filters (e.g. minimum pressure, pressure inference, etc.) to
     * the pressure at pos.
     *
     * @param pos The position of the current event
     * @param page The page the event is relative to
     * @return The filtered pressure.
     */
    double filterPressure(PositionInputData const& pos, XojPageView* page);

    /**
     * @brief Detects whether the current input event is to be used as a selection
     * rather than drawing
     */
    bool isCurrentTapSelection(InputEvent const& event) const;
};
