//
// Created by ulrich on 12.04.19.
//

#include "KeyboardInputHandler.h"

#include "gui/widgets/XournalWidget.h"

#include "InputContext.h"

KeyboardInputHandler::KeyboardInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

KeyboardInputHandler::~KeyboardInputHandler() = default;

auto KeyboardInputHandler::handleImpl(InputEvent const& event) -> bool {
    GtkXournal* xournal = inputContext->getXournal();
    GdkEvent* gdkEvent = event.sourceEvent;

    if (gdkEvent->type == GDK_KEY_PRESS) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        EditSelection* selection = xournal->selection;
        if (selection) {
            int d = 3;
            if (keyEvent->state & GDK_MOD1_MASK) {
                d = 1;
            } else if (keyEvent->state & GDK_SHIFT_MASK) {
                d = 20;
            }

            int xdir = 0;
            int ydir = 0;
            if (keyEvent->keyval == GDK_KEY_Left) {
                xdir = -1;
            } else if (keyEvent->keyval == GDK_KEY_Up) {
                ydir = -1;
            } else if (keyEvent->keyval == GDK_KEY_Right) {
                xdir = 1;
            } else if (keyEvent->keyval == GDK_KEY_Down) {
                ydir = 1;
            }
            if (xdir != 0 || ydir != 0) {
                selection->moveSelection(d * xdir, d * ydir);
                selection->ensureWithinVisibleArea();
                return true;
            }
        }
        return xournal->view->onKeyPressEvent(keyEvent);
    } else if (gdkEvent->type == GDK_KEY_RELEASE) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        return inputContext->getView()->onKeyReleaseEvent(keyEvent);
    }

    return false;
}
