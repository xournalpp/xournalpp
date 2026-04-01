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

#include <gdk/gdk.h>  // for GdkEventSequence

#include "PenInputHandler.h"  // for PenInputHandler

class InputContext;
struct InputEvent;

class TouchDrawingInputHandler: public PenInputHandler {
private:
    GdkEventSequence* primarySequence = nullptr;
    GdkEventSequence* secondarySequence = nullptr;

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

private:
    void updateKineticScrollingEnabled();
};
