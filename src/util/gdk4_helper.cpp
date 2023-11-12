#include "util/gdk4_helper.h"

#include "util/Assert.h"

GdkModifierType gdk_event_get_modifier_state(GdkEvent* event) {
    GdkModifierType state;
    gdk_event_get_state(event, &state);
    return state;
}
