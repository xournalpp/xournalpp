//
// Created by ulrich on 08.04.19.
//

#include "TouchDrawingInputHandler.h"

#include <string>  // for operator==, basic_string

#include "control/Control.h"                   // for Control
#include "control/DeviceListHelper.h"          // for getSourceMapping
#include "control/ToolEnums.h"                 // for TOOL_HAND, ToolType
#include "control/ToolHandler.h"               // for ToolHandler
#include "control/settings/ButtonConfig.h"     // for ButtonConfig
#include "control/settings/Settings.h"         // for Settings
#include "control/settings/SettingsEnums.h"    // for BUTTON_TOUCH, Button
#include "gui/MainWindow.h"                    // for MainWindow
#include "gui/PageView.h"                      // for XojPageView
#include "gui/XournalView.h"                   // for XournalView
#include "gui/XournalppCursor.h"               // for XournalppCursor
#include "gui/inputdevices/InputEvents.h"      // for InputEvent, BUTTON_PRE...
#include "gui/inputdevices/PenInputHandler.h"  // for PenInputHandler
#include "gui/widgets/XournalWidget.h"         // for GtkXournal

#include "InputContext.h"  // for InputContext
#include "InputUtils.h"    // for InputUtils

TouchDrawingInputHandler::TouchDrawingInputHandler(InputContext* inputContext): PenInputHandler(inputContext) {
    inputContext->getToolHandler()->addToolChangedListener([&](ToolType newToolType) {
        InputDeviceClass touchscreenClass =
                DeviceListHelper::getSourceMapping(GDK_SOURCE_TOUCHSCREEN, this->inputContext->getSettings());

        // If using the touchscreen as a touchscreen...
        if (touchscreenClass == INPUT_DEVICE_TOUCHSCREEN) {
            updateKineticScrollingEnabled();
        }
    });
}

TouchDrawingInputHandler::~TouchDrawingInputHandler() = default;

auto TouchDrawingInputHandler::handleImpl(InputEvent const& event) -> bool {
    auto* mainWindow = inputContext->getView()->getControl()->getWindow();
    ToolHandler* toolHandler = this->inputContext->getToolHandler();

    // Do we need to end the touch sequence?
    bool mustEnd = event.type == BUTTON_RELEASE_EVENT;
    mustEnd = mustEnd || (event.type == GRAB_BROKEN_EVENT && this->deviceClassPressed);

    // Notify if finger enters/leaves widget
    // Note: Drawing outside window doesn't seem to work
    //  if this is put later in handleImpl.
    if (event.type == ENTER_EVENT) {
        this->actionEnterWindow(event);
        return false;
    }

    if (event.type == LEAVE_EVENT) {
        this->actionLeaveWindow(event);
        return false;
    }

    // Multitouch
    if ((this->primarySequence && this->primarySequence != event.sequence) || this->secondarySequence) {
        if (!this->secondarySequence) {
            this->secondarySequence = event.sequence;

            // Let Gtk's touchscreen-based scrolling do
            // momentum-based scrolling!
            mainWindow->setGtkTouchscreenScrollingEnabled(true);

            XojPageView* currentPage = this->getPageAtCurrentPosition(event);

            if (currentPage) {
                currentPage->onSequenceCancelEvent(event.deviceId);
            }
        }

        if (mustEnd) {
            // Ending two-finger panning/zooming? We should now only have a primary sequence.
            if (event.sequence == this->primarySequence) {
                this->primarySequence = this->secondarySequence;
                this->secondarySequence = nullptr;

                // Only scrolling now if using the hand tool.
                mainWindow->setGtkTouchscreenScrollingEnabled(toolHandler->getToolType() == TOOL_HAND);
            } else if (event.sequence == this->secondarySequence) {
                this->secondarySequence = nullptr;
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

        this->actionStart(event);

        updateKineticScrollingEnabled();

        return false;
    }

    // Trigger motion action when finger is pressed and moved,
    // unless a hand tool, in which case, we return false to
    // let an alternate handler (e.g. TouchInputHandler) decide
    // what to do...
    if (this->deviceClassPressed && event.type == MOTION_EVENT && toolHandler->getToolType() != TOOL_HAND) {
        GtkXournal* xournal = inputContext->getXournal();

        this->inputContext->getView()->getCursor()->setRotationAngle(event.relative.x);

        this->actionMotion(event);
        XournalppCursor* cursor = xournal->view->getCursor();
        cursor->updateCursor();

        return true;
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

    this->updateKineticScrollingEnabled();

    if (toolChanged) {
        toolHandler->fireToolChanged();
    }

    return true;
}

void TouchDrawingInputHandler::updateKineticScrollingEnabled() {
    auto* control = inputContext->getView()->getControl();
    auto* mainWindow = control->getWindow();
    auto* toolHandler = this->inputContext->getToolHandler();

    //  Kinetic scrolling is nice; however, we need to disable it so we can draw (it steals
    // single-finger input).
    if (mainWindow != nullptr && control->getSettings()->getTouchDrawingEnabled()) {
        mainWindow->setGtkTouchscreenScrollingEnabled(toolHandler->getToolType() == TOOL_HAND);
    }
}
