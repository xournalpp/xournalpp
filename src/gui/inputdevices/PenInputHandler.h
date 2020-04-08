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

#include "AbstractInputHandler.h"

class InputContext;

class PenInputHandler: public AbstractInputHandler {
private:
    XojPageView* lastActivePage = nullptr;

protected:
    /**
     * Whether the current device class has a device with button 1 in pressed state
     */
    bool deviceClassPressed = false;
    /**
     * modifier varaibles storing whether button 2 and 3 are pressed
     */
    bool modifier2 = false;
    bool modifier3 = false;

    /**
     * Reference to the last event
     */
    InputEvent lastEvent{};

    /**
     * Reference to the last event actually hitting a page
     */
    InputEvent lastHitEvent{};

    /**
     * Start position to reference scroll offset
     */
    double scrollStartX = 0;
    double scrollStartY = 0;

    /**
     * The offset to scroll in x-direction
     */
    double scrollOffsetX = 0;

    /**
     * The offset to scroll in y-direction
     */
    double scrollOffsetY = 0;

    /**
     * Flag whether pen is within the widget
     */
    bool penInWidget = false;

    /**
     * Page a selection started at as we require this for motion updates
     */
    XojPageView* sequenceStartPage = nullptr;

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
};
