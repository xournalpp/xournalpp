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

#include "PenInputHandler.h"
#include "XournalType.h"

class InputContext;

class TouchDrawingInputHandler: public PenInputHandler {
private:
    GdkEventSequence* currentSequence = nullptr;

public:
    explicit TouchDrawingInputHandler(InputContext* inputContext);
    ~TouchDrawingInputHandler() override;

    bool handleImpl(InputEvent const& event) override;

protected:
    /**
     * @brief Change the tool according to Touch Settings
     *
     * @param event some Inputevent
     * @return true if tool was changed successfully
     * @return false if tool was not changed successfully
     */
    bool changeTool(InputEvent const& event) override;
};
