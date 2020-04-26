//
// Created by ulrich on 12.04.19.
//

#include "KeyboardInputHandler.h"

#include "gui/widgets/XournalWidget.h"

#include "InputContext.h"

KeyboardInputHandler::KeyboardInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

KeyboardInputHandler::~KeyboardInputHandler() = default;

auto KeyboardInputHandler::handleImpl(InputEvent* event) -> bool {
    auto keyEvent = reinterpret_cast<GdkEventKey*>(event->sourceEvent);

    if (keyEvent->type == GDK_KEY_PRESS) {
        auto selection = inputContext->getView()->getSelections()->getSelection();
        if (selection) {
            int d = 3;

            if ((keyEvent->state & GDK_MOD1_MASK) || (keyEvent->state & GDK_SHIFT_MASK)) {
                if (keyEvent->state & GDK_MOD1_MASK) {
                    d = 1;
                } else {
                    d = 20;
                }
            }

            if (keyEvent->keyval == GDK_KEY_Left) {
                selection->moveSelection(d, 0);
                return true;
            }
            if (keyEvent->keyval == GDK_KEY_Up) {
                selection->moveSelection(0, d);
                return true;
            }
            if (keyEvent->keyval == GDK_KEY_Right) {
                selection->moveSelection(-d, 0);
                return true;
            }
            if (keyEvent->keyval == GDK_KEY_Down) {
                selection->moveSelection(0, -d);
                return true;
            }
        }

        return inputContext->getView()->onKeyPressEvent(keyEvent);
    }

    return inputContext->getView()->onKeyReleaseEvent(keyEvent);
}
