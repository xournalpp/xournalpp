//
// Created by ulrich on 08.04.19.
//

#include "TouchDrawingInputHandler.h"

#include "gui/XournalppCursor.h"
#include "gui/inputdevices/InputUtils.h"
#include "gui/widgets/XournalWidget.h"
#include "model/Stroke.h"

#include "InputContext.h"

TouchDrawingInputHandler::TouchDrawingInputHandler(InputContext* inputContext): PenInputHandler(inputContext) {}

TouchDrawingInputHandler::~TouchDrawingInputHandler() = default;

auto TouchDrawingInputHandler::handleImpl(InputEvent const& event) -> bool {
    // Only handle events when there is no active gesture
    GtkXournal* xournal = inputContext->getXournal();

    // Trigger end of action if mouse button is released
    bool mustEnd = event.type == BUTTON_RELEASE_EVENT;

    // If we loose our Grab on the device end the current action
    mustEnd = mustEnd || event.type == GRAB_BROKEN_EVENT && this->deviceClassPressed;

    // Multitouch
    if (this->primarySequence && this->primarySequence != event.sequence || this->secondarySequence) {
        if (!this->secondarySequence) {
            this->secondarySequence = event.sequence;

            if (this->startedSingleInput) {
                XojPageView* currentPage = this->getPageAtCurrentPosition(event);
                currentPage->onMotionCancelEvent();
            }
        }

        if (mustEnd) {
            if (event.sequence == this->primarySequence) {
                this->primarySequence = nullptr;
            } else if (event.sequence == this->secondarySequence) {
                this->secondarySequence = this->primarySequence;
                this->primarySequence = nullptr;
            }
        }

        return false;
    }

    /*
     * Trigger start action
     */
    // Trigger start of action when pen/mouse is pressed
    if (event.type == BUTTON_PRESS_EVENT && this->primarySequence == nullptr) {
        this->primarySequence = event.sequence;
        this->deviceClassPressed = true;
        this->startedSingleInput = false;

        return false;
    }

    /*
     * Trigger motion actions
     */
    if (this->deviceClassPressed && !this->startedSingleInput) {
        this->actionStart(event);
        this->startedSingleInput = true;
    }

    // Trigger motion action when finger is pressed and moved
    if (this->deviceClassPressed && event.type == MOTION_EVENT) {
        this->inputContext->getView()->getCursor()->setRotationAngle(event.relativeX);

        this->actionMotion(event);
        XournalppCursor* cursor = xournal->view->getCursor();
        cursor->updateCursor();

        return true;
    }

    // Notify if finger enters/leaves widget
    if (event.type == ENTER_EVENT) {
        this->actionEnterWindow(event);
    }
    if (event.type == LEAVE_EVENT) {
        this->actionLeaveWindow(event);
    }

    if (mustEnd) {
        this->primarySequence = nullptr;
        this->actionEnd(event);

        this->deviceClassPressed = false;
        return false;  // Any alternates should also end.
    }

    return false;
}

auto TouchDrawingInputHandler::changeTool(InputEvent const& event) -> bool {
    Settings* settings = this->inputContext->getSettings();
    ButtonConfig* cfg = settings->getButtonConfig(Button::BUTTON_TOUCH);
    ToolHandler* toolHandler = this->inputContext->getToolHandler();
    bool toolChanged = false;

    if (cfg->device == event.deviceName) {
        if (InputUtils::touchDrawingDisallowed(toolHandler, settings))
            return false;
        toolChanged = InputUtils::applyButton(toolHandler, settings, Button::BUTTON_TOUCH);
    } else {
        toolChanged = toolHandler->pointActiveToolToToolbarTool();
    }

    if (toolChanged)
        toolHandler->fireToolChanged();
    return true;
}
