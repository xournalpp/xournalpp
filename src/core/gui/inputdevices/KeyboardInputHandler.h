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

#include "AbstractInputHandler.h"  // for AbstractInputHandler

class InputContext;
struct InputEvent;


class KeyboardInputHandler: public AbstractInputHandler {
private:
public:
    explicit KeyboardInputHandler(InputContext* inputContext);
    ~KeyboardInputHandler() override;
    bool handleImpl(InputEvent const& event) override;
};
