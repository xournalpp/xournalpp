//
// Created by ulrich on 08.04.19.
//

#include "TouchDrawingInputHandler.h"

#include "gui/XournalppCursor.h"
#include "gui/widgets/XournalWidget.h"

#include "InputContext.h"

TouchDrawingInputHandler::TouchDrawingInputHandler(InputContext* inputContext): PenInputHandler(inputContext) {}

TouchDrawingInputHandler::~TouchDrawingInputHandler() = default;

auto TouchDrawingInputHandler::handleImpl(InputEvent const& event) -> bool {
    // Only handle events when there is no active gesture
    GtkXournal* xournal = inputContext->getXournal();

    // Disallow multitouch
    if (this->currentSequence && this->currentSequence != event.sequence) {
        return false;
    }

    /*
     * Trigger start action
     */
    // Trigger start of action when pen/mouse is pressed
    if (event.type == BUTTON_PRESS_EVENT && this->currentSequence == nullptr) {
        this->currentSequence = event.sequence;
        this->deviceClassPressed = true;
        this->actionStart(event);
        return true;
    }

    /*
     * Trigger motion actions
     */
    // Trigger motion action when finger is pressed and moved
    if (this->deviceClassPressed && event.type == MOTION_EVENT) {
        this->actionMotion(event);
        XournalppCursor* cursor = xournal->view->getCursor();
        cursor->updateCursor();
    }

    // Notify if finger enters/leaves widget
    if (event.type == ENTER_EVENT) {
        this->actionEnterWindow(event);
    }
    if (event.type == LEAVE_EVENT) {
        this->actionLeaveWindow(event);
    }

    // Trigger end of action if mouse button is released
    if (event.type == BUTTON_RELEASE_EVENT) {
        this->currentSequence = nullptr;
        this->actionEnd(event);
        this->deviceClassPressed = false;
        return true;
    }

    // If we loose our Grab on the device end the current action
    if (event.type == GRAB_BROKEN_EVENT && this->deviceClassPressed) {
        this->currentSequence = nullptr;
        this->actionEnd(event);
        this->deviceClassPressed = false;
        return true;
    }

    return false;
}

auto TouchDrawingInputHandler::changeTool(InputEvent const& event) -> bool {
    Settings* settings = this->inputContext->getSettings();
    ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
    ToolHandler* toolHandler = this->inputContext->getToolHandler();

    ButtonConfig* cfg = nullptr;
    if (cfgTouch->device == event.deviceName) {
        cfg = cfgTouch;

        // If an action is defined we do it, even if it's a drawing action...
        if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE) {
            ToolType tool = toolHandler->getToolType();
            if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER) {
                g_message("ignore touchscreen for drawing!\n");
                return true;
            }
        }
    }

    if (cfg && cfg->getAction() != TOOL_NONE) {
        toolHandler->pointCurrentToolToButtonTool();
        cfg->acceptActions(toolHandler);
    } else {
        toolHandler->pointCurrentToolToToolbarTool();
    }

    return false;
}
