#include "SetsquareInputHandler.h"

#include <memory>  // for __shared_ptr_access, shar...

#include "control/Control.h"               // for Control
#include "control/SetsquareController.h"   // for SetsquareController, HYPOTENUSE
#include "control/ToolEnums.h"             // for TOOL_HAND, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"           // for ToolHandler
#include "gui/XournalView.h"               // for XournalView
#include "gui/inputdevices/InputEvents.h"  // for InputEvent
#include "model/Setsquare.h"               // for Setsquare::INITIAL_HEIGHT,...

constexpr double MIN_HEIGHT = 4.5;
constexpr double MAX_HEIGHT = 15.0;

SetsquareInputHandler::SetsquareInputHandler(XournalView* xournal, GeometryToolController* controller):
        GeometryToolInputHandler(xournal, controller, Setsquare::INITIAL_HEIGHT, Setsquare::INITIAL_X,
                                 Setsquare::INITIAL_Y) {}

SetsquareInputHandler::~SetsquareInputHandler() noexcept { this->unregisterFromPool(); }

auto SetsquareInputHandler::handlePointer(InputEvent const& event) -> bool {
    const auto coords = getCoords(event);
    SetsquareController* setsquareController = static_cast<SetsquareController*>(controller);

    const auto toolHandler = xournal->getControl()->getToolHandler();
    switch (toolHandler->getToolType()) {
        case TOOL_HIGHLIGHTER:
        case TOOL_PEN:
            if (event.type == BUTTON_PRESS_EVENT) {
                if (controller->isInsideGeometryTool(coords.x, coords.y, 0) &&
                    setsquareController->posRelToSide(HYPOTENUSE, coords.x, coords.y).y >= -0.5) {
                    // initialize range
                    const auto proj = setsquareController->posRelToSide(HYPOTENUSE, coords.x, coords.y).x;
                    setsquareController->createEdgeStroke(proj);
                    return true;
                } else if (controller->isInsideGeometryTool(coords.x, coords.y, 0) &&
                           (setsquareController->posRelToSide(LEFT_LEG, coords.x, coords.y).y >= -0.5 ||
                            setsquareController->posRelToSide(RIGHT_LEG, coords.x, coords.y).y >= -0.5)) {

                    // initialize point
                    setsquareController->createRadialStroke(coords.x, coords.y);
                    return true;
                } else {
                    return false;
                }

            } else if (event.type == MOTION_EVENT) {
                // update range and paint
                if (setsquareController->existsEdgeStroke()) {
                    const auto proj = setsquareController->posRelToSide(HYPOTENUSE, coords.x, coords.y).x;
                    setsquareController->updateEdgeStroke(proj);
                    return true;
                } else if (setsquareController->existsRadialStroke()) {
                    setsquareController->updateRadialStroke(coords.x, coords.y);
                    return true;
                }
                return false;
            } else if (event.type == BUTTON_RELEASE_EVENT) {
                // add stroke to layer and reset
                if (setsquareController->existsEdgeStroke()) {
                    setsquareController->finalizeEdgeStroke();
                    return true;
                } else if (setsquareController->existsRadialStroke()) {
                    setsquareController->finalizeRadialStroke();
                    return true;
                }
            }
            return false;
        case TOOL_HAND:
            if (event.type == BUTTON_PRESS_EVENT) {
                if (!controller->isInsideGeometryTool(coords.x, coords.y, 0)) {
                    return false;
                } else {
                    sequenceStart(event);
                    this->handScrolling = true;
                    return true;
                }
            } else if (event.type == MOTION_EVENT && this->handScrolling) {
                scrollMotion(event);
                return true;
            } else if (event.type == BUTTON_RELEASE_EVENT && this->handScrolling) {
                this->handScrolling = false;
            }
        default:
            return false;
    }
}

auto SetsquareInputHandler::getMinHeight() const -> double { return MIN_HEIGHT; }
auto SetsquareInputHandler::getMaxHeight() const -> double { return MAX_HEIGHT; }
