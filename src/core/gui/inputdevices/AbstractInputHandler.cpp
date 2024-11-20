//
// Created by ulrich on 06.04.19.
//

#include "AbstractInputHandler.h"

#include <string_view>

#include <glib.h>  // for gdouble

#include "control/settings/Settings.h"           // for Settings
#include "gui/Layout.h"                          // for Layout
#include "gui/PageView.h"                        // for XojPageView
#include "gui/XournalView.h"                     // for XournalView
#include "gui/XournalppCursor.h"                 // for XournalppCursor
#include "gui/inputdevices/InputEvents.h"        // for InputEvent
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "gui/widgets/XournalWidget.h"           // for GtkXournal
#include "model/Point.h"                         // for Point, Point::NO_PRE...
#include "util/Assert.h"                         // for xoj_assert
#include "util/safe_casts.h"                     // for round_cast

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
auto AbstractInputHandler::getPageAtCurrentPosition(InputEvent const& event) const -> XojPageView* {
    if (!event) {
        return nullptr;
    }

    GtkXournal* xournal = this->inputContext->getXournal();

    int x = round_cast<int>(event.relative.x);
    int y = round_cast<int>(event.relative.y);

    return xournal->layout->getPageViewAt(x, y);
}

/**
 * Get input data relative to current input page
 */
auto AbstractInputHandler::getInputDataRelativeToCurrentPage(XojPageView* page, InputEvent const& event) const
        -> PositionInputData {
    xoj_assert(page != nullptr);

    double eventX = event.relative.x;
    double eventY = event.relative.y;

    PositionInputData pos = {};
    pos.x = eventX - static_cast<double>(page->getX());
    pos.y = eventY - static_cast<double>(page->getY());
    pos.pressure = Point::NO_PRESSURE;

    if (this->inputContext->getSettings()->isPressureSensitivity()) {
        pos.pressure = event.pressure;
    }

    pos.state = this->inputContext->getModifierState();
    pos.timestamp = event.timestamp;

    pos.deviceId = event.deviceId;

    return pos;
}

void AbstractInputHandler::onBlock() {}

void AbstractInputHandler::onUnblock() {}
