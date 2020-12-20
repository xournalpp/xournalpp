#include "NewGtkInputDevice.h"

#include <cmath>

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "gui/XournalppCursor.h"
#include "gui/inputdevices/HandRecognition.h"
#include "gui/scroll/ScrollHandling.h"
#include "model/Point.h"
#include "util/DeviceListHelper.h"

#include "InputSequence.h"


NewGtkInputDevice::NewGtkInputDevice(GtkWidget* widget, XournalView* view, ScrollHandling* scrollHandling):
        AbstractInputDevice(widget, view), scrollHandling(scrollHandling) {
    pointerInputList =
            g_hash_table_new_full(nullptr, nullptr, nullptr, reinterpret_cast<GDestroyNotify>(InputSequence::free));
    touchInputList =
            g_hash_table_new_full(nullptr, nullptr, nullptr, reinterpret_cast<GDestroyNotify>(InputSequence::free));

    ignoreTouch = !view->getControl()->getSettings()->isTouchWorkaround();
}

NewGtkInputDevice::~NewGtkInputDevice() {
    g_hash_table_destroy(pointerInputList);
    pointerInputList = nullptr;
    g_hash_table_destroy(touchInputList);
    touchInputList = nullptr;
}

/**
 * Focus the widget
 */
void NewGtkInputDevice::focusWidget() { gtk_widget_grab_focus(widget); }

auto NewGtkInputDevice::getSettings() -> Settings* { return view->getControl()->getSettings(); }

auto NewGtkInputDevice::getToolHandler() -> ToolHandler* { return view->getControl()->getToolHandler(); }

auto NewGtkInputDevice::getXournal() -> GtkXournal* { return GTK_XOURNAL(widget); }

auto NewGtkInputDevice::getView() -> XournalView* { return view; }

/**
 * Try to start input
 *
 * @return true if it should start
 */
auto NewGtkInputDevice::startInput(InputSequence* input) -> bool {
    if (inputRunning == input) {
        g_warning("Input for the same device started twice!");
        return true;
    }


    if (inputRunning == nullptr) {
        inputRunning = input;
        return true;
    }


    if (inputRunning->checkStillRunning()) {
        g_warning("Input was not stopped correctly!");
        inputRunning = input;
        return true;
    }


    return false;
}

/**
 * Stop input of this sequence
 */
void NewGtkInputDevice::stopInput(InputSequence* input) {
    if (inputRunning == input) {
        inputRunning = nullptr;
    }
}

/**
 * Initialize the input handling, set input events
 */
void NewGtkInputDevice::initWidget() {
    gtk_widget_set_support_multidevice(widget, true);

    int mask =
            // Key handling
            GDK_KEY_PRESS_MASK |

            // Allow scrolling
            GDK_SCROLL_MASK |

            // Touch / Pen / Mouse
            GDK_TOUCH_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
            GDK_SMOOTH_SCROLL_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK;

    gtk_widget_add_events(widget, mask);

    g_signal_connect(widget, "event", G_CALLBACK(eventCallback), this);
}

auto NewGtkInputDevice::eventCallback(GtkWidget* widget, GdkEvent* event, NewGtkInputDevice* self) -> bool {
    return self->eventHandler(event);
}

/**
 * Handle Key Press event
 */
auto NewGtkInputDevice::eventKeyPressHandler(GdkEventKey* event) -> bool {
    GtkXournal* xournal = GTK_XOURNAL(widget);

    EditSelection* selection = xournal->selection;
    if (selection) {
        int d = 3;

        if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_SHIFT_MASK)) {
            if (event->state & GDK_MOD1_MASK) {
                d = 1;
            } else {
                d = 20;
            }
        }

        if (event->keyval == GDK_KEY_Left) {
            selection->moveSelection(-d, 0);
            return true;
        }
        if (event->keyval == GDK_KEY_Up) {
            selection->moveSelection(0, -d);
            return true;
        }
        if (event->keyval == GDK_KEY_Right) {
            selection->moveSelection(d, 0);
            return true;
        }
        if (event->keyval == GDK_KEY_Down) {
            selection->moveSelection(0, d);
            return true;
        }
    }

    return xournal->view->onKeyPressEvent(event);
}

/**
 * Handle all GTK Events
 */
auto NewGtkInputDevice::eventHandler(GdkEvent* event) -> bool {
    if (event->type == GDK_KEY_PRESS) {
        return eventKeyPressHandler(&event->key);
    }

    if (event->type == GDK_KEY_RELEASE) {
        return view->onKeyReleaseEvent(&event->key);
    }

    if (event->type == GDK_SCROLL) {
        // Hand over to standard GTK Scroll / Zoom handling
        return false;
    }

    GdkDevice* device = gdk_event_get_device(event);
    GdkDevice* sourceDevice = gdk_event_get_source_device(event);
    GdkEventSequence* sequence = gdk_event_get_event_sequence(event);

    if (sourceDevice == nullptr) {
        sourceDevice = device;
    }

    this->getView()->getHandRecognition()->event(sourceDevice);

    if (ignoreTouch && GDK_SOURCE_TOUCHSCREEN == gdk_device_get_source(sourceDevice)) {
        return false;
    }

    if (event->type == GDK_TOUCH_END || event->type == GDK_TOUCH_CANCEL) {
        auto* input = static_cast<InputSequence*>(g_hash_table_lookup(touchInputList, sequence));

        if (input != nullptr) {
            input->actionEnd((reinterpret_cast<GdkEventTouch*>(event))->time);
        }

        g_hash_table_remove(touchInputList, sequence);
        return true;
    }
    if (event->type == GDK_LEAVE_NOTIFY) {
        g_hash_table_remove(pointerInputList, sourceDevice);
        return true;
    }

    InputSequence* input = nullptr;
    if (sequence == nullptr) {
        input = static_cast<InputSequence*>(g_hash_table_lookup(pointerInputList, sourceDevice));

        if (input == nullptr) {
            input = new InputSequence(this);
            g_hash_table_insert(pointerInputList, sourceDevice, input);
        }
    } else {
        input = static_cast<InputSequence*>(g_hash_table_lookup(touchInputList, sequence));

        if (input == nullptr) {
            input = new InputSequence(this);
            g_hash_table_insert(touchInputList, sequence, input);
        }
    }

    // Apply the correct device if not yet set, should not change
    // But GTK decides which inputs are get together
    input->setDevice(sourceDevice);
    input->clearAxes();

    gdouble x = NAN, y = NAN;
    if (gdk_event_get_coords(event, &x, &y)) {
        scrollHandling->translate(x, y);
        input->setCurrentPosition(x, y);
    }

    if (gdk_event_get_root_coords(event, &x, &y)) {
        input->setCurrentRootPosition(x, y);
    }

    guint button = 0;
    if (gdk_event_get_button(event, &button)) {
        input->setButton(button, gdk_event_get_time(event));
    }

    auto state = static_cast<GdkModifierType>(0);
    if (gdk_event_get_state(event, &state)) {
        input->setState(state);
    }

    if (event->type == GDK_MOTION_NOTIFY || event->type == GDK_TOUCH_UPDATE) {
        input->copyAxes(event);
        guint32 time = event->type == GDK_MOTION_NOTIFY ?
                               (reinterpret_cast<GdkEventMotion*>(event))->time :
                               (reinterpret_cast<GdkEventTouch*>(event))->time;  // or call gdk_event_get_time(event)
        input->actionMoved(time);

        XournalppCursor* cursor = view->getControl()->getWindow()->getXournal()->getCursor();
        cursor->setInvisible(false);
        cursor->updateCursor();

        view->getHandRecognition()->event(sourceDevice);
    } else if (event->type == GDK_BUTTON_PRESS || event->type == GDK_TOUCH_BEGIN) {
        input->copyAxes(event);
        guint32 time = event->type == GDK_BUTTON_PRESS ?
                               (reinterpret_cast<GdkEventButton*>(event))->time :
                               (reinterpret_cast<GdkEventTouch*>(event))->time;  // or call gdk_event_get_time()
        input->actionStart(time);
    } else if (event->type == GDK_BUTTON_RELEASE) {
        input->copyAxes(event);
        input->actionEnd((reinterpret_cast<GdkEventButton*>(event))->time);
    }

    return true;
}
