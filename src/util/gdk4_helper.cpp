#include "util/gdk4_helper.h"

#include "util/Assert.h"

/*** GdkEvent ***/
GdkModifierType gdk_event_get_modifier_state(GdkEvent* event) {
    GdkModifierType state;
    gdk_event_get_state(event, &state);
    return state;
}


/*** GdkKeyEvent ***/
GdkModifierType gdk_key_event_get_consumed_modifiers(GdkEvent* event) {
    auto keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    GdkModifierType consumed;
    /*
     According to https://docs.gtk.org/gdk3/method.Keymap.translate_keyboard_state.html    *  *
     consumed modifiers should be masked out. For instance, on a US keyboard, the plus symbol is shifted, so when
     comparing a key press to a <Control>plus accelerator <Shift> should be masked out.
     */
    gdk_keymap_translate_keyboard_state(keymap, event->key.hardware_keycode,
                                        static_cast<GdkModifierType>(event->key.state), event->key.group, nullptr,
                                        nullptr, nullptr, &consumed);
    return consumed;
}

guint gdk_key_event_get_keyval(GdkEvent* event) {
    guint res = 0;
    gdk_event_get_keyval(event, &res);
    return res;
}
