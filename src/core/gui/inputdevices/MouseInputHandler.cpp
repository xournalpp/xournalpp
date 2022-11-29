//
// Created by ulrich on 08.04.19.
//

#include "MouseInputHandler.h"

#include "control/ToolHandler.h"               // for ToolHandler
#include "control/settings/SettingsEnums.h"    // for Button, BUTTON_MOUSE_M...
#include "gui/XournalView.h"                   // for XournalView
#include "gui/XournalppCursor.h"               // for XournalppCursor
#include "gui/inputdevices/InputEvents.h"      // for InputEvent, BUTTON_PRE...
#include "gui/inputdevices/PenInputHandler.h"  // for PenInputHandler
#include "gui/widgets/XournalWidget.h"         // for GtkXournal

#include "InputContext.h"  // for InputContext
#include "InputUtils.h"    // for InputUtils

class Settings;
class XojPageView;

MouseInputHandler::MouseInputHandler(InputContext* inputContext): PenInputHandler(inputContext) {}

MouseInputHandler::~MouseInputHandler() = default;

auto MouseInputHandler::handleImpl(InputEvent const& event) -> bool {
    // Only handle events when there is no active gesture
    GtkXournal* xournal = inputContext->getXournal();

    // Determine the pressed states of devices and associate them to the current event
    setPressedState(event);

    /*
     * Trigger start action
     */
    // Trigger start of action when pen/mouse is pressed
    if (event.type == BUTTON_PRESS_EVENT) {
        this->actionStart(event);
        return true;
    }

    if (event.type == BUTTON_2_PRESS_EVENT || event.type == BUTTON_3_PRESS_EVENT) {
        this->actionPerform(event);
        return true;
    }

    /*
     * Trigger motion actions
     */
    // Trigger motion action when pen/mouse is pressed and moved
    if (event.type == MOTION_EVENT)  // mouse or pen moved
    {
        this->actionMotion(event);
        XournalppCursor* cursor = xournal->view->getCursor();
        cursor->setInvisible(false);
        cursor->updateCursor();
    }

    // Notify if mouse enters/leaves widget
    if (event.type == ENTER_EVENT) {
        this->actionEnterWindow(event);
    }
    if (event.type == LEAVE_EVENT) {
        // this->inputContext->unblockDevice(InputContext::TOUCHSCREEN);
        // this->inputContext->getView()->getHandRecognition()->unblock();
        this->actionLeaveWindow(event);
    }

    // Trigger end of action if mouse button is released
    if (event.type == BUTTON_RELEASE_EVENT) {
        this->actionEnd(event);
        return true;
    }

    // If we loose our Grab on the device end the current action
    if (event.type == GRAB_BROKEN_EVENT && this->deviceClassPressed) {
        // TODO(fabian): We may need to update pressed state manually here
        this->actionEnd(event);
        return true;
    }

    return false;
}

void MouseInputHandler::setPressedState(InputEvent const& event) {
    XojPageView* currentPage = getPageAtCurrentPosition(event);

    this->inputContext->getXournal()->view->getCursor()->setInsidePage(currentPage != nullptr);

    if (event.type == BUTTON_PRESS_EVENT)  // mouse button pressed or pen touching surface
    {
        this->deviceClassPressed = true;

        switch (event.button) {
            case 2:
                this->modifier2 = true;
                break;
            case 3:
                this->modifier3 = true;
            default:
                break;
        }
    }
    if (event.type == BUTTON_RELEASE_EVENT)  // mouse button released or pen not touching surface anymore
    {
        this->deviceClassPressed = false;

        switch (event.button) {
            case 2:
                this->modifier2 = false;
                break;
            case 3:
                this->modifier3 = false;
            default:
                break;
        }
    }
}

auto MouseInputHandler::changeTool(InputEvent const& event) -> bool {
    Settings* settings = this->inputContext->getSettings();
    ToolHandler* toolHandler = this->inputContext->getToolHandler();
    bool isClickOnSelection = this->inputContext->getXournal()->selection;
    bool toolChanged = false;

    if (modifier2 && !isClickOnSelection)
        toolChanged = InputUtils::applyButton(toolHandler, settings, Button::BUTTON_MOUSE_MIDDLE);
    else if (modifier3 && !isClickOnSelection)
        toolChanged = InputUtils::applyButton(toolHandler, settings, Button::BUTTON_MOUSE_RIGHT);
    else
        toolChanged = toolHandler->pointActiveToolToToolbarTool();

    if (toolChanged) {
        ToolType toolType = toolHandler->getToolType();
        if (toolType == TOOL_TEXT)
            toolHandler->selectTool(toolType);
        toolHandler->fireToolChanged();
    }

    return true;
}

void MouseInputHandler::onBlock() {}
