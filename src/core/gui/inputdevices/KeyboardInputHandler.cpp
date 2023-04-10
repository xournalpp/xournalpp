//
// Created by ulrich on 12.04.19.
//

#include "KeyboardInputHandler.h"

#include <gdk/gdk.h>         // for _GdkEventKey, gdk...
#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Down, GDK...

#include "control/tools/EditSelection.h"            // for EditSelection
#include "gui/XournalView.h"                        // for XournalView
#include "gui/inputdevices/AbstractInputHandler.h"  // for AbstractInputHandler
#include "gui/inputdevices/InputEvents.h"           // for GdkEventGuard
#include "gui/widgets/XournalWidget.h"              // for GtkXournal

#include "InputContext.h"  // for InputContext

KeyboardInputHandler::KeyboardInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

KeyboardInputHandler::~KeyboardInputHandler() = default;

auto KeyboardInputHandler::handleImpl(InputEvent const& event) -> bool {
    GdkEvent* gdkEvent = event.sourceEvent;

    if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_PRESS) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        return inputContext->getView()->onKeyPressEvent(keyEvent);
    } else if (gdk_event_get_event_type(gdkEvent) == GDK_KEY_RELEASE) {
        auto keyEvent = reinterpret_cast<GdkEventKey*>(gdkEvent);
        return inputContext->getView()->onKeyReleaseEvent(keyEvent);
    }

    return false;
}
