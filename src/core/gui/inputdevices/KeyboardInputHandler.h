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

class InputContext;
struct KeyEvent;

class KeyboardInputHandler {
private:
public:
    explicit KeyboardInputHandler(InputContext* inputContext);
    ~KeyboardInputHandler();
    bool keyPressed(const KeyEvent& e) const;
    bool keyReleased(const KeyEvent& e) const;

private:
    InputContext* inputContext;
};
