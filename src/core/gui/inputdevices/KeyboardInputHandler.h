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

class KeyboardInputHandler final {
private:
public:
    explicit KeyboardInputHandler(InputContext* inputContext);
    ~KeyboardInputHandler();
    bool keyPressed(KeyEvent e) const;
    bool keyReleased(KeyEvent e) const;

private:
    InputContext* inputContext;
};
