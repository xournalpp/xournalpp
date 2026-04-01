#include "DeviceId.h"

#include <gdk/gdk.h>

DeviceId::DeviceId(const GdkDevice* id): id(id) {
    if (id) {
        auto source = gdk_device_get_source(const_cast<GdkDevice*>(id));
        trackpointOrTouchpad = source == GDK_SOURCE_TRACKPOINT || source == GDK_SOURCE_TOUCHPAD;
    }
}
void DeviceId::reset(const GdkDevice* id) { *this = DeviceId(id); }
DeviceId::operator bool() const { return id != nullptr; }
bool DeviceId::operator==(const DeviceId& o) const {
    // For laptops with both touchpad and trackpoint, allow to use the trackpoint buttons above the touchpad
    return (id == o.id) || (trackpointOrTouchpad && o.trackpointOrTouchpad);
}
bool DeviceId::operator!=(const DeviceId& o) const { return !(*this == o); }
