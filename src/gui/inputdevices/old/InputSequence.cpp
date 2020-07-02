#include "InputSequence.h"

#include <cmath>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/settings/ButtonConfig.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"
#include "gui/Layout.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "model/Point.h"

#include "NewGtkInputDevice.h"
#include "Util.h"


InputSequence::InputSequence(NewGtkInputDevice* inputHandler): inputHandler(inputHandler) {
    this->presureSensitivity = inputHandler->getSettings()->isPressureSensitivity();
}

InputSequence::~InputSequence() {
    if (inputRunning) {
        actionEnd(__UINT32_MAX__);
    }
    clearAxes();

    // Make 100% sure there is no input running
    stopInput();
}

/**
 * Set current input device
 */
void InputSequence::setDevice(GdkDevice* device) { this->device = device; }

/**
 * Clear the last stored axes
 */
void InputSequence::clearAxes() {
    // Todo(fabian): find a better solution for the following
    g_clear_pointer(&axes, g_free);  // NOLINT
}

/**
 * Set the axes
 *
 * @param axes Will be handed over, and freed by InputSequence
 */
void InputSequence::setAxes(gdouble* axes) {
    clearAxes();
    this->axes = axes;
}

/**
 * Copy axes from event
 */
void InputSequence::copyAxes(GdkEvent* event) {
    clearAxes();
    setAxes(static_cast<gdouble*>(g_memdup(event->motion.axes, sizeof(gdouble) * gdk_device_get_n_axes(device))));
}

/**
 * Set Position
 */
void InputSequence::setCurrentPosition(double x, double y) {
    this->x = x;
    this->y = y;
}

/**
 * Set Root Position
 */
void InputSequence::setCurrentRootPosition(double x, double y) {
    this->rootX = x;
    this->rootY = y;
}

/**
 * Set (mouse)button
 */
void InputSequence::setButton(guint button, guint time) {
    this->button = button;
    this->eventTime = time;
}

/**
 * Set state flags from GDKevent (Shift down etc.)
 */
void InputSequence::setState(GdkModifierType state) { this->state = state; }

/**
 * Get Page at current position
 *
 * @return page or nullptr if none
 */
auto InputSequence::getPageAtCurrentPosition() -> XojPageView* {
    GtkXournal* xournal = inputHandler->getXournal();

    double x = this->x + xournal->x;
    double y = this->y + xournal->y;

    return xournal->layout->getViewAt(x, y);
}

/**
 * Do the scrolling with the hand tool
 */
void InputSequence::handleScrollEvent() {
    // use root coordinates as reference point because
    // scrolling changes window relative coordinates
    // see github Gnome/evince@1adce5486b10e763bed869

    // GTK handles event compression/filtering differently between versions - this may be needed on certain hardware/GTK
    // combinations.
    if (std::abs(lastMousePositionX - rootX) < 0.1 && std::abs(lastMousePositionY - rootY) < 0.1) {
        return;
    }


    if (scrollOffsetX == 0 && scrollOffsetY == 0) {
        scrollOffsetX = lastMousePositionX - rootX;
        scrollOffsetY = lastMousePositionY - rootY;

        Util::execInUiThread([=]() {
            inputHandler->getXournal()->layout->scrollRelative(scrollOffsetX, scrollOffsetY);

            // Scrolling done, so reset our counters
            scrollOffsetX = 0;
            scrollOffsetY = 0;
        });

        lastMousePositionX = rootX;
        lastMousePositionY = rootY;
    }
}

/**
 * Mouse / Pen / Touch move
 */
auto InputSequence::actionMoved(guint32 time) -> bool {
    GtkXournal* xournal = inputHandler->getXournal();
    ToolHandler* h = inputHandler->getToolHandler();

    this->eventTime = time;

    changeTool();

    if (h->getToolType() == TOOL_HAND) {
        if (inScrolling) {
            handleScrollEvent();
            return true;
        }
        return false;
    }
    if (xournal->selection) {
        EditSelection* selection = xournal->selection;
        XojPageView* view = selection->getView();

        PositionInputData pos = getInputDataRelativeToCurrentPage(view);

        if (xournal->selection->isMoving()) {
            selection->mouseMove(pos.x, pos.y, pos.isAltDown());
        } else {
            CursorSelectionType selType = selection->getSelectionTypeForPos(pos.x, pos.y, xournal->view->getZoom());
            xournal->view->getCursor()->setMouseSelectionType(selType);
        }
        return true;
    }

    XojPageView* pv = nullptr;

    if (current_view) {
        pv = current_view;
    } else {
        pv = getPageAtCurrentPosition();
    }

    xournal->view->getCursor()->setInsidePage(pv != nullptr);

    if (pv && inputRunning) {
        // allow events only to a single page!
        if (currentInputPage == nullptr || pv == currentInputPage) {
            PositionInputData pos = getInputDataRelativeToCurrentPage(pv);
            return pv->onMotionNotifyEvent(pos);
        }
    }

    return false;
}

/**
 * Mouse / Pen down / touch start
 */
auto InputSequence::actionStart(guint32 time) -> bool {
    this->eventTime = time;

    inputHandler->focusWidget();

    checkCanStartInput();

    if (!inputRunning) {
        return false;
    }

    // none button release event was sent, send one now
    // only for this device, other devices may still have unfinished input
    if (currentInputPage) {
        PositionInputData pos = getInputDataRelativeToCurrentPage(currentInputPage);
        currentInputPage->onButtonReleaseEvent(pos);
    }

    // Change the tool depending on the key or device
    if (changeTool()) {
        return true;
    }

    GtkXournal* xournal = inputHandler->getXournal();

    // hand tool don't change the selection, so you can scroll e.g.
    // with your touchscreen without remove the selection
    ToolHandler* h = inputHandler->getToolHandler();
    if (h->getToolType() == TOOL_HAND) {
        XournalppCursor* cursor = xournal->view->getCursor();
        cursor->setMouseDown(true);
        inScrolling = true;
        // set reference
        lastMousePositionX = rootX;
        lastMousePositionY = rootY;

        return true;
    }
    if (xournal->selection) {
        EditSelection* selection = xournal->selection;

        XojPageView* view = selection->getView();

        PositionInputData pos = getInputDataRelativeToCurrentPage(view);

        CursorSelectionType selType = selection->getSelectionTypeForPos(pos.x, pos.y, xournal->view->getZoom());
        if (selType) {

            if (selType == CURSOR_SELECTION_MOVE && button == 3) {
                selection->copySelection();
            }

            xournal->view->getCursor()->setMouseDown(true);
            xournal->selection->mouseDown(selType, pos.x, pos.y);
            return true;
        }


        xournal->view->clearSelection();
        if (changeTool()) {
            return true;
        }
    }

    XojPageView* pv = getPageAtCurrentPosition();

    current_view = pv;

    if (pv) {
        currentInputPage = pv;

        PositionInputData pos = getInputDataRelativeToCurrentPage(pv);
        return pv->onButtonPressEvent(pos);
    }

    // not handled
    return false;
}


/**
 * Check if input is still running, or if there an event was missed
 *
 * @return true if input is stopped now
 */
auto InputSequence::checkStillRunning() -> bool {
    if (!inputRunning) {
        // Already stopped
        return true;
    }

    auto mask = static_cast<GdkModifierType>(0);
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(inputHandler->getXournal()));
    gdk_device_get_state(device, window, nullptr, &mask);

    if ((GDK_BUTTON1_MASK & mask) || (GDK_BUTTON2_MASK & mask) || (GDK_BUTTON3_MASK & mask) ||
        (GDK_BUTTON4_MASK & mask) || (GDK_BUTTON5_MASK & mask)) {
        // Button still down
        // Input is still running, probably everything OK
        return false;
    }

    // Button is not down, stop input now!
    // So the new input can start
    actionEnd(__UINT32_MAX__);

    return true;
}

/**
 * Mouse / Pen up / touch end
 */
void InputSequence::actionEnd(guint32 time) {
    if (!inputRunning) {
        return;
    }

    this->eventTime = time;

    // Mouse button not pressed anymore
    this->button = 0;

    current_view = nullptr;

    GtkXournal* xournal = inputHandler->getXournal();
    XournalppCursor* cursor = xournal->view->getCursor();
    ToolHandler* h = inputHandler->getToolHandler();

    cursor->setMouseDown(false);

    inScrolling = false;

    EditSelection* sel = xournal->view->getSelection();
    if (sel) {
        sel->mouseUp();
    }

    if (currentInputPage) {
        PositionInputData pos = getInputDataRelativeToCurrentPage(currentInputPage);
        currentInputPage->onButtonReleaseEvent(pos);
        currentInputPage = nullptr;
    }

    EditSelection* tmpSelection = xournal->selection;
    xournal->selection = nullptr;

    h->pointCurrentToolToToolbarTool();

    // we need this workaround so it's possible to select something with the middle button
    if (tmpSelection) {
        xournal->view->setSelection(tmpSelection);
    }

    stopInput();
}

/**
 * Get input data relative to current input page
 */
auto InputSequence::getInputDataRelativeToCurrentPage(XojPageView* page) -> PositionInputData {
    GtkXournal* xournal = inputHandler->getXournal();

    PositionInputData pos{};
    pos.x = x - page->getX() - xournal->x;
    pos.y = y - page->getY() - xournal->y;
    pos.pressure = Point::NO_PRESSURE;
    pos.timestamp = this->eventTime;

    if (presureSensitivity) {
        gdk_device_get_axis(device, axes, GDK_AXIS_PRESSURE, &pos.pressure);
    }

    pos.state = this->state;

    return pos;
}


/**
 * Check if this input can be started (don't do two inputs at the same time)
 */
void InputSequence::checkCanStartInput() { inputRunning = inputHandler->startInput(this); }

/**
 * Stop the running input, if running
 */
void InputSequence::stopInput() {
    inputRunning = false;
    inputHandler->stopInput(this);
}

/**
 * Change the tool according to the device and button
 * @return true to ignore event
 */
auto InputSequence::changeTool() -> bool {
    Settings* settings = inputHandler->getSettings();
    ButtonConfig* cfgTouch = settings->getTouchButtonConfig();
    ToolHandler* h = inputHandler->getToolHandler();
    GtkXournal* xournal = inputHandler->getXournal();

    ButtonConfig* cfg = nullptr;
    if (gdk_device_get_source(device) == GDK_SOURCE_PEN) {
        penDevice = true;
        if (button == 2) {
            cfg = settings->getStylusButton1Config();
        } else if (button == 3) {
            cfg = settings->getStylusButton2Config();
        }
    } else if (button == 2 /* Middle Button */ && !xournal->selection) {
        cfg = settings->getMiddleButtonConfig();
    } else if (button == 3 /* Right Button */ && !xournal->selection) {
        cfg = settings->getRightButtonConfig();
    } else if (gdk_device_get_source(device) == GDK_SOURCE_ERASER) {
        penDevice = true;
        cfg = settings->getEraserButtonConfig();
    } else if (cfgTouch->device == gdk_device_get_name(device)) {
        cfg = cfgTouch;

        // If an action is defined we do it, even if it's a drawing action...
        if (cfg->getDisableDrawing() && cfg->getAction() == TOOL_NONE) {
            ToolType tool = h->getToolType();
            if (tool == TOOL_PEN || tool == TOOL_ERASER || tool == TOOL_HILIGHTER) {
                g_message("ignore touchscreen for drawing!\n");
                return true;
            }
        }
    }

    if (cfg && cfg->getAction() != TOOL_NONE) {
        h->pointCurrentToolToButtonTool();
        cfg->acceptActions(h);
    } else {
        h->pointCurrentToolToToolbarTool();
    }

    return false;
}

/**
 * Free an input sequence, used as callback for GTK
 */
void InputSequence::free(InputSequence* sequence) { delete sequence; }
