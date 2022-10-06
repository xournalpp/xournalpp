//
// Created by ulrich on 06.04.19.
//

#include "AbstractInputHandler.h"

#include <cmath>  // for round

#include <glib.h>  // for gdouble, g_assert

#include "control/settings/Settings.h"           // for Settings
#include "gui/Layout.h"                          // for Layout
#include "gui/PageView.h"                        // for XojPageView
#include "gui/XournalView.h"                     // for XournalView
#include "gui/XournalppCursor.h"                 // for XournalppCursor
#include "gui/inputdevices/InputEvents.h"        // for InputEvent
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "gui/widgets/XournalWidget.h"           // for GtkXournal
#include "model/Point.h"                         // for Point, Point::NO_PRE...

#include "InputContext.h"  // for InputContext

AbstractInputHandler::AbstractInputHandler(InputContext* inputContext) { this->inputContext = inputContext; }

AbstractInputHandler::~AbstractInputHandler() = default;

void AbstractInputHandler::block(bool block) {
    if (block == this->blocked) {
        return;
    }
    this->blocked = block;
    if (!this->blocked) {
        this->onUnblock();
    } else {
        this->onBlock();
    }
}

auto AbstractInputHandler::isBlocked() const -> bool { return this->blocked; }

auto AbstractInputHandler::handle(InputEvent const& event) -> bool {
    if (!this->blocked) {
        this->inputContext->getXournal()->view->getCursor()->setInputDeviceClass(event.deviceClass);
        return this->handleImpl(event);
    }
    return true;
}

/**
 * Get Page at current position
 *
 * @return page or nullptr if none
 */
auto AbstractInputHandler::getPageAtCurrentPosition(InputEvent const& event) -> XojPageView* {
    if (!event) {
        return nullptr;
    }

    GtkXournal* xournal = this->inputContext->getXournal();

    int x = static_cast<int>(std::round(event.relativeX));
    int y = static_cast<int>(std::round(event.relativeY));

    return xournal->layout->getPageViewAt(x, y);
}

/**
 * Get input data relative to current input page
 */
auto AbstractInputHandler::getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent const& event)
        -> PositionInputData {
    g_assert(page != nullptr);

    gdouble eventX = event.relativeX;
    gdouble eventY = event.relativeY;

    PositionInputData pos = {};
    pos.x = eventX - static_cast<double>(page->getX());
    pos.y = eventY - static_cast<double>(page->getY());
    pos.pressure = Point::NO_PRESSURE;

    if (this->inputContext->getSettings()->isPressureSensitivity()) {
        pos.pressure = event.pressure;
    }

    pos.state = this->inputContext->getModifierState();
    pos.timestamp = event.timestamp;

    return pos;
}

void AbstractInputHandler::onBlock() {}

void AbstractInputHandler::onUnblock() {}
