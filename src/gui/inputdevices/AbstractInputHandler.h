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

#include <string>
#include <vector>

#include <gdk/gdk.h>

#include "control/settings/ButtonConfig.h"
#include "gui/PageView.h"
#include "gui/inputdevices/PositionInputData.h"
#include "model/Point.h"

#include "InputEvents.h"
#include "XournalType.h"

class InputContext;

/**
 * Abstract class for a specific input state
 */
class AbstractInputHandler {
private:
    bool blocked = false;

protected:
    InputContext* inputContext;

    /**
     *  Whether we're in a press-move-up event sequence.
     * E.g. the user has clicked and is currently moving the mouse.
     */
    bool inputRunning = false;

protected:
    XojPageView* getPageAtCurrentPosition(InputEvent const& event);
    PositionInputData getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent const& event);

public:
    explicit AbstractInputHandler(InputContext* inputContext);
    virtual ~AbstractInputHandler();

    void block(bool block);
    bool isBlocked() const;
    virtual void onBlock();
    virtual void onUnblock();
    bool handle(InputEvent const& event);
    virtual bool handleImpl(InputEvent const& event) = 0;
};
