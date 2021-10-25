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

    if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_PRESS) {
        auto keyEvent = reinterpret_cast<GdkKeyEvent*>(gdkEvent);
        EditSelection* selection = xournal->selection;
        if (selection) {
            int d = 3;
            auto event_state = gdk_event_get_modifier_state(gdkEvent);
            if (event_state & GDK_ALT_MASK) {
                d = 1;
            } else if (event_state & GDK_SHIFT_MASK) {
                d = 20;
            }

            int xdir = 0;
            int ydir = 0;
            if (auto event_keyval = gdk_key_event_get_keyval(gdkEvent); event_keyval == GDK_KEY_Left) {
                xdir = -1;
            } else if (event_keyval == GDK_KEY_Up) {
                ydir = -1;
            } else if (event_keyval == GDK_KEY_Right) {
                xdir = 1;
            } else if (event_keyval == GDK_KEY_Down) {
                ydir = 1;
            }
            if (xdir != 0 || ydir != 0) {
                selection->moveSelection(d * xdir, d * ydir);
                selection->ensureWithinVisibleArea();
                return true;
            }
        }
        return xournal->view->onKeyPressEvent(gdkEvent);
    } else if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_RELEASE) {
        return inputContext->getView()->onKeyReleaseEvent(gdkEvent);
    }

    return false;
}
