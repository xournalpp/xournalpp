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
    auto* mainWindow = inputContext->getView()->getControl()->getWindow();
    ToolHandler* toolHandler = this->inputContext->getToolHandler();

    // Do we need to end the touch sequence?
    bool mustEnd = event.type == BUTTON_RELEASE_EVENT;
    mustEnd = mustEnd || event.type == GRAB_BROKEN_EVENT && this->deviceClassPressed;

    // Multitouch
    if (this->primarySequence && this->primarySequence != event.sequence || this->secondarySequence) {
        if (!this->secondarySequence) {
            this->secondarySequence = event.sequence;

            // Let Gtk's touchscreen-based scrolling do
            // momentum-based scrolling!
            mainWindow->setGtkTouchscreenScrollingEnabled(true);

            if (this->startedSingleInput) {
                XojPageView* currentPage = this->getPageAtCurrentPosition(event);

                if (currentPage) {
                    currentPage->onMotionCancelEvent();
                }
            }
        }

        if (mustEnd) {
            if (event.sequence == this->primarySequence) {
                this->primarySequence = nullptr;

                // Only scrolling now if using the hand tool.
                mainWindow->setGtkTouchscreenScrollingEnabled(toolHandler->getToolType() == TOOL_HAND);
            } else if (event.sequence == this->secondarySequence) {
                this->secondarySequence = this->primarySequence;
                this->primarySequence = nullptr;
            }
        }

        // false lets another input handler (e.g. TouchInputHandler)
        // handle the event.
        return false;
    }

    // Trigger start of action when pen/mouse is pressed
    if (event.type == BUTTON_PRESS_EVENT && this->primarySequence == nullptr) {
        this->primarySequence = event.sequence;
        this->deviceClassPressed = true;

        // Defer starting the single-touch action --
        // this lets us avoid calling onMotionCancel event
        // if this becomes a multi-touch event.
        this->startedSingleInput = false;

        // Only enable kinetic scrolling if using the hand tool.
        mainWindow->setGtkTouchscreenScrollingEnabled(toolHandler->getToolType() == TOOL_HAND);

        return false;
    }

    // If we defered the single-touch action,
    if (this->deviceClassPressed && !this->startedSingleInput) {
        this->actionStart(event);
        this->startedSingleInput = true;
    }

    // Trigger motion action when finger is pressed and moved,
    // unless a hand tool, in which case, we return false to
    // let an alternate handler (e.g. TouchInputHandler) decide
    // what to do...
    if (this->deviceClassPressed && event.type == MOTION_EVENT && toolHandler->getToolType() != TOOL_HAND) {
        GtkXournal* xournal = inputContext->getXournal();

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

    if (toolChanged) {
        toolHandler->fireToolChanged();
    }

    auto* control = inputContext->getView()->getControl();
    auto* mainWindow = control->getWindow();

    if (mainWindow != nullptr && control->getSettings()->getTouchDrawingEnabled()) {
        //  GtkTouchscreenScrolling -- this makes it easier to use the hand tool; however
        // Gtk steals all single-touch input when enabled, so if drawing, we want this disabled.
        mainWindow->setGtkTouchscreenScrollingEnabled(toolHandler->getToolType() == TOOL_HAND);
    }

    return true;
}
