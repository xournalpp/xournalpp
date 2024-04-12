//
// Created by ulrich on 06.04.19.
//

#include "PenInputHandler.h"

#include <algorithm>  // for max, min
#include <cmath>      // for abs, atan, sqrt
#include <thread>     // for thread
#include <utility>    // for move

#include <glib.h>     // for gdouble, gint, g_message
#include <gtk/gtk.h>  // for gtk_adjustment_get_value

#include "control/ToolEnums.h"                  // for TOOL_HAND, TOOL_IMAGE
#include "control/ToolHandler.h"                // for ToolHandler
#include "control/settings/Settings.h"          // for Settings
#include "control/tools/CursorSelectionType.h"  // for CursorSelectionType
#include "control/tools/EditSelection.h"        // for EditSelection
#include "gui/Layout.h"                         // for Layout
#include "gui/PageView.h"                       // for XojPageView
#include "gui/XournalView.h"                    // for XournalView
#include "gui/XournalppCursor.h"                // for XournalppCursor
#include "gui/scroll/ScrollHandling.h"          // for ScrollHandling
#include "gui/widgets/XournalWidget.h"          // for GtkXournal
#include "model/Point.h"                        // for Point, Point::NO_PRES...
#include "util/Assert.h"                        // for xoj_assert
#include "util/Point.h"                         // for Point
#include "util/Util.h"                          // for execInUiThread
#include "util/safe_casts.h"

#include "AbstractInputHandler.h"  // for AbstractInputHandler
#include "InputContext.h"          // for InputContext
#include "PositionInputData.h"     // for PositionInputData
#include "config-debug.h"          // for DEBUG_INPUT

#define WIDGET_SCROLL_BORDER 25

PenInputHandler::PenInputHandler(InputContext* inputContext): AbstractInputHandler(inputContext) {}

PenInputHandler::~PenInputHandler() = default;

void PenInputHandler::updateLastEvent(InputEvent const& event) {
    if (!event) {
        return;
    }
    this->lastEvent = event;
    if (getPageAtCurrentPosition(event)) {
        this->lastHitEvent = event;
    }
}

void PenInputHandler::handleScrollEvent(InputEvent const& event) {
    // use root coordinates as reference point because
    // scrolling changes window relative coordinates
    // see github Gnome/evince@1adce5486b10e763bed869

    // GTK handles event compression/filtering differently between versions - this may be needed on certain hardware/GTK
    // combinations.
    if (std::abs((this->scrollStartX - event.absolute.x)) < 0.1 &&
        std::abs((this->scrollStartY - event.absolute.y)) < 0.1) {
        return;
    }

    if (this->scrollOffsetX == 0 && this->scrollOffsetY == 0) {
        this->scrollOffsetX = this->scrollStartX - event.absolute.x;
        this->scrollOffsetY = this->scrollStartY - event.absolute.y;

        Util::execInUiThread([&]() {
            this->inputContext->getXournal()->layout->scrollRelative(this->scrollOffsetX, this->scrollOffsetY);

            // Scrolling done, so reset our counters
            this->scrollOffsetX = 0;
            this->scrollOffsetY = 0;
        });

        // Update the reference for the scroll-offset
        this->scrollStartX = event.absolute.x;
        this->scrollStartY = event.absolute.y;
    }
}

auto PenInputHandler::actionStart(InputEvent const& event) -> bool {
    this->inputContext->focusWidget();

    this->lastActionStartTimeStamp = event.timestamp;
    this->sequenceStartPosition = {event.absolute.x, event.absolute.y};

    XojPageView* currentPage = this->getPageAtCurrentPosition(event);
    // set reference data for handling of entering/leaving page
    this->updateLastEvent(event);

    // Change the tool depending on the key
    if (!changeTool(event))
        return false;

    // Used for pressure inference
    this->lastPressure = 0.0;

    // Flag running input
    ToolHandler* toolHandler = this->inputContext->getToolHandler();
    ToolType toolType = toolHandler->getToolType();

    //
    if (toolType != TOOL_IMAGE) {
        this->inputRunning = true;
    } else {
        this->deviceClassPressed = false;
    }

    this->penInWidget = true;

    GtkXournal* xournal = this->inputContext->getXournal();

    XournalppCursor* cursor = xournal->view->getCursor();
    cursor->setMouseDown(true);


    // Save the starting offset when hand-tool is selected to get a reference for the scroll-offset
    if (toolType == TOOL_HAND) {
        this->scrollStartX = event.absolute.x;
        this->scrollStartY = event.absolute.y;
    }

    this->sequenceStartPage = currentPage;

    // hand tool don't change the selection, so you can scroll e.g. with your touchscreen without remove the selection
    bool changeSelection = xournal->selection && toolHandler->getToolType() != TOOL_HAND;
    if ((event.state & GDK_SHIFT_MASK)) {
        // When tap single selection is enabled, selections can happen with the Pen tool
        if (toolHandler->getToolType() == TOOL_PEN && isCurrentTapSelection(event)) {
            changeSelection = false;
        }
        // Selection tools does not change selection with Shift pressed
        if (isSelectToolTypeSingleLayer(toolType)) {
            changeSelection = false;
        }
    }
    if (changeSelection) {
        EditSelection* selection = xournal->selection;

        XojPageView* view = selection->getView();
        PositionInputData selectionPos = this->getInputDataRelativeToCurrentPage(view, event);

        // Check if event modifies selection instead of page
        CursorSelectionType selType =
                selection->getSelectionTypeForPos(selectionPos.x, selectionPos.y, xournal->view->getZoom());
        if (selType) {

            if (selType == CURSOR_SELECTION_MOVE && modifier3) {
                selection->copySelection();
            }

            xournal->selection->mouseDown(selType, selectionPos.x, selectionPos.y);
            return true;
        }

        xournal->view->clearSelection();
        changeTool(event);
        // stop early to prevent drawing when clicking outside of the selection with the intention of deselecting
        if (toolHandler->isDrawingTool())
            return true;
    }

    // Forward event to page
    if (currentPage) {
        PositionInputData pos = this->getInputDataRelativeToCurrentPage(currentPage, event);
        if (pos.pressure != Point::NO_PRESSURE) {
            pressureMode = PressureMode::DEVICE_PRESSURE;
        } else if (this->inputContext->getSettings()->isPressureGuessingEnabled()) {
            pressureMode = PressureMode::INFERRED_PRESSURE;
        } else {
            pressureMode = PressureMode::NO_PRESSURE;
        }
        pos.pressure = this->filterPressure(pos, currentPage);

        return currentPage->onButtonPressEvent(pos);
    }

    return true;
}

double PenInputHandler::inferPressureValue(PositionInputData const& pos, XojPageView* page) {
    PositionInputData lastPos = getInputDataRelativeToCurrentPage(page, this->lastEvent);

    double dt = (pos.timestamp - lastPos.timestamp) / 10.0;
    double distance = xoj::util::Point<double>(pos.x, pos.y).distance(xoj::util::Point<double>(lastPos.x, lastPos.y));
    double inverseSpeed = dt / (distance + 0.001);

    // This doesn't have to be exact. Arctan is used here for its sigmoid-like shape,
    // so that lim inverseSpeed->infinity (newPressure) is some finite value.
    double newPressure = 3.142 / 2.0 + std::atan(inverseSpeed * 3.14 - 1.3);

    // This weighted average both smooths abrupt changes in newPressure caused
    // by changes to inverseSpeed and causes an initial increase in pressure.
    newPressure = std::min(newPressure, 2.0) / 5.0 + this->lastPressure * 4.0 / 5.0;

    // Handle the single-point case.
    if (distance == 0) {
        newPressure = std::sqrt(dt / 10.0) - 0.1;
    }

    this->lastPressure = newPressure;

    // Final pressure tweaks...
    return (newPressure * 1.1 + 0.8) / 2.0;
}

double PenInputHandler::filterPressure(PositionInputData const& pos, XojPageView* page) {
    if (pressureMode == PressureMode::NO_PRESSURE) {
        return Point::NO_PRESSURE;
    }

    double filteredPressure;
    if (pressureMode == PressureMode::INFERRED_PRESSURE) {
        filteredPressure = inferPressureValue(pos, page);
    } else {
        xoj_assert(pressureMode == PressureMode::DEVICE_PRESSURE);
        /**
         * On some devices, the pressure value of some events is missing. Use the last recorded pressure value then.
         */
        if (pos.pressure == Point::NO_PRESSURE) {
            g_debug("Pressure-sensitive device omitted pressure this time");
            filteredPressure = lastPressure;
        } else {
            filteredPressure = pos.pressure;
            lastPressure = pos.pressure;  // Record in case the pressure value is omitted in the next event
        }
    }

    Settings* settings = this->inputContext->getSettings();
    xoj_assert(settings->getMinimumPressure() >= 0.01);
    return std::max(settings->getMinimumPressure(), filteredPressure * settings->getPressureMultiplier());
}

bool PenInputHandler::isCurrentTapSelection(InputEvent const& event) const {

    ToolHandler* toolHandler = inputContext->getToolHandler();
    if (!toolHandler->supportsTapFilter()) {
        return false;
    }

    auto* settings = inputContext->getSettings();
    if (!settings->getStrokeFilterEnabled()) {
        return false;
    }

    int tapMaxDuration = 0, filterRepetitionTime = 0;
    double tapMaxDistance = NAN;  // in mm

    settings->getStrokeFilter(&tapMaxDuration, &tapMaxDistance, &filterRepetitionTime);

    const double dpmm = settings->getDisplayDpi() / 25.4;
    const double dist = std::hypot(this->sequenceStartPosition.x - event.absolute.x,
                                   this->sequenceStartPosition.y - event.absolute.y);

    const bool noMovement = dist < tapMaxDistance * dpmm;
    const bool fastEnoughTap = event.timestamp - this->lastActionStartTimeStamp < as_unsigned(tapMaxDuration);
    const bool notAnAftershock = event.timestamp - this->lastActionEndTimeStamp > as_unsigned(filterRepetitionTime);
    if (noMovement && fastEnoughTap && notAnAftershock) {
        return true;
    }
    return false;
}

auto PenInputHandler::actionMotion(InputEvent const& event) -> bool {
    /*
     * Workaround for misbehaving devices where Enter events are not published every time
     * This is required to disable outside scrolling again
     */
    gdouble eventX = event.relative.x;
    gdouble eventY = event.relative.y;

    GtkAdjustment* adjHorizontal = this->inputContext->getScrollHandling()->getHorizontal();
    GtkAdjustment* adjVertical = this->inputContext->getScrollHandling()->getVertical();
    double h = gtk_adjustment_get_value(adjHorizontal);
    double v = gtk_adjustment_get_value(adjVertical);
    eventX -= h;
    eventY -= v;

    GtkWidget* widget = gtk_widget_get_parent(this->inputContext->getView()->getWidget());
    gint width = gtk_widget_get_allocated_width(widget);
    gint height = gtk_widget_get_allocated_height(widget);

    if (!this->penInWidget && eventX > WIDGET_SCROLL_BORDER && eventY > WIDGET_SCROLL_BORDER &&
        eventX < width - WIDGET_SCROLL_BORDER && eventY < height - WIDGET_SCROLL_BORDER) {
        this->penInWidget = true;
    }


    GtkXournal* xournal = this->inputContext->getXournal();
    ToolHandler* toolHandler = this->inputContext->getToolHandler();

    this->changeTool(event);

    if (toolHandler->getToolType() == TOOL_HAND) {
        if (this->deviceClassPressed) {
            this->handleScrollEvent(event);
            return true;
        }
        return false;
    }

    bool isShiftDown = (event.state & GDK_SHIFT_MASK);
    bool handleSelectionMove = xournal->selection != nullptr;

    if (xournal->selection && isSelectToolTypeSingleLayer(toolHandler->getToolType()) &&
        !xournal->selection->isMoving()) {
        if (isShiftDown || this->deviceClassPressed) {
            handleSelectionMove = false;
            // Cursor mode to match the multiple-selection mode
            xournal->view->getCursor()->setMouseSelectionType(CURSOR_SELECTION_NONE);
        }
    }

    if (handleSelectionMove) {
        EditSelection* selection = xournal->selection;
        XojPageView* view = selection->getView();

        PositionInputData pos = this->getInputDataRelativeToCurrentPage(view, event);

        if (xournal->selection->isMoving()) {
            selection->mouseMove(pos.x, pos.y, pos.isAltDown());
        } else if (!isShiftDown) {
            CursorSelectionType selType = selection->getSelectionTypeForPos(pos.x, pos.y, xournal->view->getZoom());
            xournal->view->getCursor()->setMouseSelectionType(selType);
        }
        return true;
    }

    // Check if page was left / entered
    XojPageView* lastEventPage = getPageAtCurrentPosition(this->lastEvent);
    XojPageView* lastHitEventPage = getPageAtCurrentPosition(this->lastHitEvent);
    XojPageView* currentPage = getPageAtCurrentPosition(event);

    if (!toolHandler->isSinglePageTool()) {
        /*
         * Get all events where the input sequence moved from one page to another without stopping the input.
         * Only trigger once the new page was entered to ensure that an input device can leave the page temporarily.
         * For these events we need to fake an end point in the old page and a start point in the new page.
         */
        if (this->deviceClassPressed && currentPage && currentPage != sequenceStartPage && lastHitEventPage) {
#ifdef DEBUG_INPUT
            g_message("PenInputHandler: Start new input on switching page...");
#endif
            this->actionEnd(this->lastHitEvent);
            this->updateLastEvent(event);

            bool result = this->actionStart(event);
            this->updateLastEvent(event);
            return result;
        }
        /*
         * Get all events where the input sequence started outside of a page and moved into one.
         * For these events we need to fake a start point in the current page.
         */
        if (this->deviceClassPressed && currentPage && !lastEventPage && !lastHitEventPage) {
#ifdef DEBUG_INPUT
            g_message("PenInputHandler: Start new input on entering page...");
#endif
            bool result = this->actionStart(event);
            this->updateLastEvent(event);
            return result;
        }
    }

    // Update the cursor
    xournal->view->getCursor()->setInsidePage(currentPage != nullptr);

    // Selections and single-page elements will always work on one page so we need to handle them differently
    if (this->sequenceStartPage && toolHandler->isSinglePageTool()) {
        // Relay the event to the page
        PositionInputData pos = getInputDataRelativeToCurrentPage(sequenceStartPage, event);

        if (!toolHandler->acceptsOutOfPageEvents()) {
            // Enforce input to stay within page
            pos.x = std::clamp(pos.x, 0.0, static_cast<double>(sequenceStartPage->getDisplayWidth()));
            pos.y = std::clamp(pos.y, 0.0, static_cast<double>(sequenceStartPage->getDisplayHeight()));
        }

        pos.pressure = this->filterPressure(pos, sequenceStartPage);

        bool result = sequenceStartPage->onMotionNotifyEvent(pos);

        this->updateLastEvent(event);  // Update the last position of the input device
        return result;
    }

    if (currentPage && this->penInWidget) {
        // Relay the event to the page
        PositionInputData pos = getInputDataRelativeToCurrentPage(currentPage, event);
        pos.pressure = this->filterPressure(pos, currentPage);

        bool result = currentPage->onMotionNotifyEvent(pos);

        this->updateLastEvent(event);  // Update the last position of the input device
        return result;
    }

    this->updateLastEvent(event);  // Update the last position of the input device
    return false;
}

auto PenInputHandler::actionEnd(InputEvent const& event) -> bool {
    GtkXournal* xournal = inputContext->getXournal();
    XournalppCursor* cursor = xournal->view->getCursor();
    ToolHandler* toolHandler = inputContext->getToolHandler();

    cursor->setMouseDown(false);

    bool cancelAction = isCurrentTapSelection(event);

    // Holding shift (with selections) also does not imply drawing
    if (toolHandler->supportsTapFilter()) {
        auto* settings = inputContext->getSettings();
        if (settings->getStrokeFilterEnabled()) {
            cancelAction |= (event.state & GDK_SHIFT_MASK) && xournal->selection != nullptr;
        }
    }

    if (cancelAction) {
        // Cancel the sequence and trigger the necessary action
        XojPageView* pageUnderTap = this->sequenceStartPage ? this->sequenceStartPage : getPageAtCurrentPosition(event);
        if (pageUnderTap) {
            PositionInputData pos = getInputDataRelativeToCurrentPage(pageUnderTap, event);
            pageUnderTap->onSequenceCancelEvent(pos.deviceId);
            pageUnderTap->onTapEvent(pos);
        }
        this->sequenceStartPage = nullptr;
        this->inputRunning = false;
        this->lastActionEndTimeStamp = event.timestamp;
        return false;
    }
    this->lastActionEndTimeStamp = event.timestamp;

    EditSelection* sel = xournal->view->getSelection();
    if (sel) {
        sel->mouseUp();
    }

    // Selections and single-page elements will always work on one page so we need to handle them differently
    if (this->sequenceStartPage && toolHandler->isSinglePageTool()) {
        PositionInputData pos = getInputDataRelativeToCurrentPage(this->sequenceStartPage, event);
        pos.pressure = this->filterPressure(pos, this->sequenceStartPage);

        this->sequenceStartPage->onButtonReleaseEvent(pos);
    } else {
        // Relay the event to the page
        XojPageView* currentPage = getPageAtCurrentPosition(event);

        /*
         * Use the last active page if you can't find a page under the cursor position.
         * This is a workaround for input leaving the page while being active and then stopping outside.
         */
        if (!currentPage) {
            if (!this->lastHitEvent) {
                return false;
            }
            currentPage = getPageAtCurrentPosition(this->lastHitEvent);
        }

        if (currentPage) {
            PositionInputData pos = getInputDataRelativeToCurrentPage(currentPage, event);
            pos.pressure = this->filterPressure(pos, currentPage);

            currentPage->onButtonReleaseEvent(pos);
        }
    }

    // Reset the selection
    EditSelection* tmpSelection = xournal->selection;
    xournal->selection = nullptr;
    this->sequenceStartPage = nullptr;

    if (toolHandler->pointActiveToolToToolbarTool()) {
        toolHandler->fireToolChanged();
    }

    // we need this workaround so it's possible to select something with the middle button
    if (tmpSelection) {
        xournal->view->setSelection(tmpSelection);
    }

    this->inputRunning = false;

    return false;
}

void PenInputHandler::actionPerform(InputEvent const& event) {
#ifdef DEBUG_INPUT
    g_message("Discrete input action; modifier1=%s, modifier2=%s", this->modifier2 ? "true" : "false",
              this->modifier3 ? "true" : "false");
#endif

    XojPageView* currentPage = this->getPageAtCurrentPosition(event);
    if (currentPage == nullptr) {
        return;
    }

    PositionInputData pos = this->getInputDataRelativeToCurrentPage(currentPage, event);
    if (event.type == BUTTON_2_PRESS_EVENT) {
        currentPage->onButtonDoublePressEvent(pos);
    } else if (event.type == BUTTON_3_PRESS_EVENT) {
        currentPage->onButtonTriplePressEvent(pos);
    }
}

void PenInputHandler::actionLeaveWindow(InputEvent const& event) {
    if (!this->penInWidget) {
        return;
    }

    if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled()) {
        this->penInWidget = false;
    }

    // Stop input sequence if the tool is not a selection tool
    ToolHandler* toolHandler = this->inputContext->getToolHandler();
    if (this->inputRunning && !toolHandler->isSinglePageTool()) {
        if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled()) {
            this->actionEnd(this->lastHitEvent);
        }
    } else if (this->deviceClassPressed) {
        // scroll if we have an active selection
        gdouble eventX = event.relative.x;
        gdouble eventY = event.relative.y;

        GtkAdjustment* adjHorizontal = this->inputContext->getScrollHandling()->getHorizontal();
        GtkAdjustment* adjVertical = this->inputContext->getScrollHandling()->getVertical();
        double h = gtk_adjustment_get_value(adjHorizontal);
        double v = gtk_adjustment_get_value(adjVertical);
        eventX -= h;
        eventY -= v;

        GtkWidget* widget = gtk_widget_get_parent(this->inputContext->getView()->getWidget());
        gint width = gtk_widget_get_allocated_width(widget);
        gint height = gtk_widget_get_allocated_height(widget);

        new std::thread([&, eventX, eventY, width, height]() {
            int offsetX = 0, offsetY = 0;

            // TODO(fabian): make offset dependent on how big the distance between pen and view is
            if (eventX < WIDGET_SCROLL_BORDER) {
                offsetX = -10;
            }

            if (eventY < WIDGET_SCROLL_BORDER) {
                offsetY = -10;
            }

            if (eventX > width - WIDGET_SCROLL_BORDER) {
                offsetX = 10;
            }

            if (eventY > height - WIDGET_SCROLL_BORDER) {
                offsetY = 10;
            }

#ifdef DEBUG_INPUT
            g_message("Offset: X:%d\tY:%d", offsetX, offsetY);
#endif

            while (!this->penInWidget) {
                Util::execInUiThread([&]() {
                    GtkXournal* xournal = this->inputContext->getXournal();
                    xournal->layout->scrollRelative(offsetX, offsetY);
                });

                // sleep for half a second until we scroll again
                g_usleep(static_cast<gulong>(0.5 * G_USEC_PER_SEC));
            }
        });
    }
}

void PenInputHandler::actionEnterWindow(InputEvent const& event) {
    this->penInWidget = true;

    if (!this->inputContext->getSettings()->getInputSystemDrawOutsideWindowEnabled()) {
        // Restart input sequence if the tool is pressed and not a single-page tool
        ToolHandler* toolHandler = this->inputContext->getToolHandler();
        if (this->deviceClassPressed && !toolHandler->isSinglePageTool()) {
            this->actionStart(event);
        }
    }
}
