#include "CompassInputHandler.h"

#include <memory>  // for __shared_ptr_access, shar...

#include "control/CompassController.h"  // for CompassController, HYPOTENUSE
#include "control/Control.h"
#include "control/ToolEnums.h"    // for TOOL_HAND, TOOL_HIGHLIGHTER
#include "control/ToolHandler.h"  // for ToolHandler
#include "gui/XournalView.h"      // for XournalView
#include "gui/inputdevices/InputEvents.h"
#include "model/Compass.h"

constexpr double MIN_HEIGHT = 0.5;
constexpr double MAX_HEIGHT = 10.0;

CompassInputHandler::CompassInputHandler(XournalView* xournal, GeometryToolController* controller):
        GeometryToolInputHandler(xournal, controller, Compass::INITIAL_HEIGHT, Compass::INITIAL_X, Compass::INITIAL_Y) {
}

CompassInputHandler::~CompassInputHandler() noexcept { this->unregisterFromPool(); }

auto CompassInputHandler::handlePointer(InputEvent const& event) -> bool {
    const auto coords = getCoords(event);
    CompassController* compassController = static_cast<CompassController*>(controller);
    const auto p = compassController->posRelToSide(coords.x, coords.y);

    const auto toolHandler = xournal->getControl()->getToolHandler();
    switch (toolHandler->getToolType()) {
        case TOOL_HIGHLIGHTER:
        case TOOL_PEN:
            if (event.type == BUTTON_PRESS_EVENT) {
                if (controller->isInsideGeometryTool(coords.x, coords.y, 0) &&
                    !controller->isInsideGeometryTool(coords.x, coords.y, -0.5)) {
                    // initialize range
                    lastProj = std::atan2(-p.y, p.x);
                    compassController->createOutlineStroke(lastProj);
                    return true;
                } else if (controller->isInsideGeometryTool(coords.x, coords.y, 0.) &&
                           std::abs(compassController->posRelToSide(coords.x, coords.y).y) <= 0.5 &&
                           std::abs(compassController->posRelToSide(coords.x, coords.y).x - 0.5 * height) <=
                                   0.5 * height) {
                    compassController->createRadialStroke(std::hypot(p.x, p.y));
                    return true;
                }
                return false;
            } else if (event.type == MOTION_EVENT) {
                // update range and paint
                if (compassController->existsOutlineStroke()) {
                    auto proj = std::atan2(-p.y, p.x);
                    proj = lastProj + std::remainder(proj - lastProj, 2 * M_PI);
                    compassController->updateOutlineStroke(proj);
                    lastProj = proj;
                    return true;
                } else if (compassController->existsRadialStroke()) {
                    compassController->updateRadialStroke(std::hypot(p.x, p.y));
                }
                return false;
            } else if (event.type == BUTTON_RELEASE_EVENT) {
                // add stroke to layer and reset
                if (compassController->existsOutlineStroke()) {
                    compassController->finalizeOutlineStroke();
                    lastProj = NAN;
                    return true;
                } else if (compassController->existsRadialStroke()) {
                    compassController->finalizeRadialStroke();
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

auto CompassInputHandler::getMinHeight() const -> double { return MIN_HEIGHT; }
auto CompassInputHandler::getMaxHeight() const -> double { return MAX_HEIGHT; }
