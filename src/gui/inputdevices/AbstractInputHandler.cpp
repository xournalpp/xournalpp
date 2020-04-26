//
// Created by ulrich on 06.04.19.
//

#include "AbstractInputHandler.h"

#include "gui/XournalppCursor.h"

#include "InputContext.h"

AbstractInputHandler::AbstractInputHandler(InputContext* inputContext) { this->inputContext = inputContext; }

AbstractInputHandler::~AbstractInputHandler() = default;

void AbstractInputHandler::block(bool block) {
    this->blocked = block;
    if (block == false) {
        this->onUnblock();
    } else {
        this->onBlock();
    }
}

auto AbstractInputHandler::isBlocked() const -> bool { return this->blocked; }

auto AbstractInputHandler::handle(InputEvent* event) -> bool {
    if (!this->blocked) {
        this->inputContext->getView()->getCursor()->setInputDeviceClass(event->deviceClass);
        return this->handleImpl(event);
    }
    return true;
}

/**
 * Get Page at current position
 *
 * @return page or nullptr if none
 */
auto AbstractInputHandler::getPageAtCurrentPosition(InputEvent* event) -> XojPageView* {
    if (event == nullptr) {
        return nullptr;
    }

    gdouble eventX = event->relativeX;
    gdouble eventY = event->relativeY;

    auto viewport = this->inputContext->getView()->getViewport();

    double x = eventX + viewport->getX();
    double y = eventY + viewport->getY();

    return this->inputContext->getView()->getLayout()->getViewAt(x, y);
}

/**
 * Get input data relative to current input page
 */
auto AbstractInputHandler::getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent* event)
        -> PositionInputData {
    g_assert(page != nullptr);
    auto viewport = this->inputContext->getView()->getViewport();

    gdouble eventX = event->relativeX;
    gdouble eventY = event->relativeY;

    PositionInputData pos = {};
    pos.x = eventX - page->getX() - viewport->getX();
    pos.y = eventY - page->getY() - viewport->getY();
    pos.pressure = Point::NO_PRESSURE;

    if (this->inputContext->getSettings()->isPressureSensitivity()) {
        pos.pressure = event->pressure;
    }

    pos.state = this->inputContext->getModifierState();
    pos.timestamp = event->timestamp;

    return pos;
}

void AbstractInputHandler::onBlock() {}

void AbstractInputHandler::onUnblock() {}
