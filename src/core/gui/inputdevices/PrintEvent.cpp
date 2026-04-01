#include "PrintEvent.h"

#include <bitset>
#include <iomanip>

using namespace xoj::input;


struct TimeToReadable {
    guint32 ts;
    guint32 reference;
};
template <typename StreamType>
static StreamType& operator<<(StreamType& str, TimeToReadable t) {
    if (t.ts > t.reference) {
        static constexpr guint32 SECOND = 1000;
        static constexpr guint32 MINUTE = 60 * SECOND;
        static constexpr guint32 HOUR = 60 * MINUTE;

        guint32 diff = t.ts - t.reference;
        auto h = diff / HOUR;
        auto m = (diff % HOUR) / MINUTE;
        auto s = (diff % MINUTE) / SECOND;
        auto ms = diff % SECOND;

        auto fill = str.fill();
        str.fill('0');
        str << h << ":" << std::setw(2) << m << ":" << std::setw(2) << s << "." << std::setw(3) << ms;
        str.fill(fill);
    } else {
        str << "Erronous value";
    }
    str << "  (" << t.ts << ")";
    return str;
}

void xoj::input::printEvent(std::ostream& str, const InputEvent& e, guint32 referenceTime) {
    static constexpr const char* deviceClassToString[] = {"INPUT_DEVICE_MOUSE", "INPUT_DEVICE_PEN",
                                                          "INPUT_DEVICE_ERASER", "INPUT_DEVICE_TOUCHSCREEN",
                                                          "INPUT_DEVICE_IGNORE"};

    static constexpr const char* inputEventTypeToString[] = {"UNKNOWN",
                                                             "BUTTON_PRESS_EVENT",
                                                             "BUTTON_2_PRESS_EVENT",
                                                             "BUTTON_3_PRESS_EVENT",
                                                             "BUTTON_RELEASE_EVENT",
                                                             "MOTION_EVENT",
                                                             "ENTER_EVENT",
                                                             "LEAVE_EVENT",
                                                             "PROXIMITY_IN_EVENT",
                                                             "PROXIMITY_OUT_EVENT",
                                                             "SCROLL_EVENT",
                                                             "GRAB_BROKEN_EVENT"};

    static constexpr const char* gdkInputSources[] = {
            "GDK_SOURCE_MOUSE",    "GDK_SOURCE_PEN",        "GDK_SOURCE_ERASER",
            "GDK_SOURCE_CURSOR",   "GDK_SOURCE_KEYBOARD",   "GDK_SOURCE_TOUCHSCREEN",
            "GDK_SOURCE_TOUCHPAD", "GDK_SOURCE_TRACKPOINT", "GDK_SOURCE_TABLET_PAD"};

    str << "Device " << e.device << ": " << deviceClassToString[e.deviceClass] << " - Name: " << e.deviceName << "\n";
    str << "   Timestamp: " << TimeToReadable{e.timestamp, referenceTime} << "\n";
    str << "   GdkInputSource: " << gdkInputSources[gdk_device_get_source(e.device)] << "\n";
    str << "   EvType: " << inputEventTypeToString[e.type] << "\n";
    str << "   Button: " << e.button << "\n";
    str << "   Touch Seq.: " << e.sequence << "\n";
    str << "   x: " << e.absolute.x << "  y: " << e.absolute.y << "  pressure: " << e.pressure << "\n";
    str << "   State: " << std::bitset<8 * sizeof(decltype(e.state))>(e.state) << std::endl;
}

void xoj::input::printGdkEvent(std::ostream& str, GdkEvent* e, guint32 referenceTime) {
    static constexpr const char* gdkInputSources[] = {
            "GDK_SOURCE_MOUSE      ", "GDK_SOURCE_PEN        ", "GDK_SOURCE_ERASER     ",
            "GDK_SOURCE_CURSOR     ", "GDK_SOURCE_KEYBOARD   ", "GDK_SOURCE_TOUCHSCREEN",
            "GDK_SOURCE_TOUCHPAD   ", "GDK_SOURCE_TRACKPOINT ", "GDK_SOURCE_TABLET_PAD "};

    /// Indexes are shifted by 1 because GDK_NOTHING == -1
    static constexpr const char* gdkEventTypeWithIndexShiftedByOne[] = {"GDK_NOTHING",
                                                                        "GDK_DELETE",
                                                                        "GDK_DESTROY",
                                                                        "GDK_EXPOSE",
                                                                        "GDK_MOTION_NOTIFY",
                                                                        "GDK_BUTTON_PRESS",
                                                                        "GDK_DOUBLE_BUTTON_PRESS",
                                                                        "GDK_TRIPLE_BUTTON_PRESS",
                                                                        "GDK_BUTTON_RELEASE",
                                                                        "GDK_KEY_PRESS",
                                                                        "GDK_KEY_RELEASE",
                                                                        "GDK_ENTER_NOTIFY",
                                                                        "GDK_LEAVE_NOTIFY",
                                                                        "GDK_FOCUS_CHANGE",
                                                                        "GDK_CONFIGURE",
                                                                        "GDK_MAP",
                                                                        "GDK_UNMAP",
                                                                        "GDK_PROPERTY_NOTIFY",
                                                                        "GDK_SELECTION_CLEAR",
                                                                        "GDK_SELECTION_REQUEST",
                                                                        "GDK_SELECTION_NOTIFY",
                                                                        "GDK_PROXIMITY_IN",
                                                                        "GDK_PROXIMITY_OUT",
                                                                        "GDK_DRAG_ENTER",
                                                                        "GDK_DRAG_LEAVE",
                                                                        "GDK_DRAG_MOTION",
                                                                        "GDK_DRAG_STATUS",
                                                                        "GDK_DROP_START",
                                                                        "GDK_DROP_FINISHED",
                                                                        "GDK_CLIENT_EVENT",
                                                                        "GDK_VISIBILITY_NOTIFY",
                                                                        "UNKNOWN_GAP_IN_GDK_EVENT_TYPE_ENUM",
                                                                        "GDK_SCROLL",
                                                                        "GDK_WINDOW_STATE",
                                                                        "GDK_SETTING",
                                                                        "GDK_OWNER_CHANGE",
                                                                        "GDK_GRAB_BROKEN",
                                                                        "GDK_DAMAGE",
                                                                        "GDK_TOUCH_BEGIN",
                                                                        "GDK_TOUCH_UPDATE",
                                                                        "GDK_TOUCH_END",
                                                                        "GDK_TOUCH_CANCEL",
                                                                        "GDK_TOUCHPAD_SWIPE",
                                                                        "GDK_TOUCHPAD_PINCH",
                                                                        "GDK_PAD_BUTTON_PRESS",
                                                                        "GDK_PAD_BUTTON_RELEASE",
                                                                        "GDK_PAD_RING",
                                                                        "GDK_PAD_STRIP",
                                                                        "GDK_PAD_GROUP_MODE"};

    static constexpr const char* gdkScrollDirection[] = {
            "GDK_SCROLL_UP", "GDK_SCROLL_DOWN", "GDK_SCROLL_LEFT", "GDK_SCROLL_RIGHT", "GDK_SCROLL_SMOOTH",
    };

    str << "Event:          " << e << "\n";
    str << "   Timestamp:   " << TimeToReadable{gdk_event_get_time(e), referenceTime} << "\n";
    str << "   Type:        " << gdkEventTypeWithIndexShiftedByOne[gdk_event_get_event_type(e) + 1] << "\n";
    if (GdkDevice* d = gdk_event_get_device(e); d) {
        str << "   Device:      " << d << " : " << gdkInputSources[gdk_device_get_source(d)] << " -- "
            << gdk_device_get_name(d) << "\n";
    }
    if (GdkDevice* d = gdk_event_get_source_device(e); d) {
        str << "   Source dev:  " << d << " : " << gdkInputSources[gdk_device_get_source(d)] << " -- "
            << gdk_device_get_name(d) << "\n";
    }
    if (GdkDeviceTool* t = gdk_event_get_device_tool(e); t) {
        str << "   DeviceTool:  " << t << "\n";
    }
    if (guint btn; gdk_event_get_button(e, &btn)) {
        str << "   Button:      " << btn << "\n";
    }
    if (guint key; gdk_event_get_keyval(e, &key)) {
        str << "   Key:         " << key << "\n";
    }
    if (guint clickcount; gdk_event_get_click_count(e, &clickcount)) {
        str << "   Click count: " << clickcount << "\n";
    }
    if (auto* seq = gdk_event_get_event_sequence(e); seq) {
        str << "   Sequence:    " << seq << "\n";
    }
    if (GdkModifierType state; gdk_event_get_state(e, &state)) {
        str << "   State:       " << std::bitset<8 * sizeof(decltype(state))>(state) << std::endl;
    }
    if (double x, y; gdk_event_get_coords(e, &x, &y)) {
        str << "   Coords:      " << x << "   " << y << "\n";
    }
    if (double p; gdk_event_get_axis(e, GDK_AXIS_PRESSURE, &p)) {
        str << "   Pressure:    " << p << "\n";
    }
    if (GdkScrollDirection dir; gdk_event_get_scroll_direction(e, &dir)) {
        str << "   ScrollDir:   " << gdkScrollDirection[dir] << "\n";
    }
    if (double dx, dy; gdk_event_get_scroll_deltas(e, &dx, &dy)) {
        str << "   ScrollDelta: " << dx << "   " << dy << "\n";
    }
}
