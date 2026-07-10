#include "KeyboardInputHandler.h"

#include "gui/XournalView.h"                        // for XournalView
#include "gui/inputdevices/GeometryToolInputHandler.h"  // for GeometryToolInputHandler

#include "InputContext.h"  // for InputContext
#include "InputEvents.h"   // for KeyEvent

KeyboardInputHandler::KeyboardInputHandler(InputContext* inputContext): inputContext(inputContext) {}

KeyboardInputHandler::~KeyboardInputHandler() = default;

bool KeyboardInputHandler::keyPressed(const KeyEvent& e) const { return inputContext->getView()->onKeyPressEvent(e); }

bool KeyboardInputHandler::keyReleased(const KeyEvent& e) const {
    return inputContext->getView()->onKeyReleaseEvent(e);
}
