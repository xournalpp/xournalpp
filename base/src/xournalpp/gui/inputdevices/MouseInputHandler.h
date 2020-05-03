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
    bool changeTool(InputEvent const& event) override;
    void onBlock() override;
};
