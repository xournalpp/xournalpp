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

class MouseInputHandler: public PenInputHandler {
private:
    void setPressedState(InputEvent const& event);

public:
    explicit MouseInputHandler(InputContext* inputContext);
    ~MouseInputHandler() override;

    bool handleImpl(InputEvent const& event) override;

    /**
     * @brief Change the tool based on the settings and the Button pressed.
     * In case
     *  - no button is pressed
     *  - or the settings for that button are "No Toolchange"
     *  - or the click is happening on top of a selection on the canvas
     * this function ensures that the tool selected in the toolbar resmains selected.
     *
     * @param event
     */
    void changeTool(InputEvent const& event) override;
    void onBlock() override;
};
