/*
 * Xournal++
 *
 * header for missing gdk4 functions (part of the gtk4 port)
 * will be removed later
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gdk/gdk.h>

#if GDK_MAJOR_VERSION == 3

/*** GdkEvent ***/
GdkModifierType gdk_event_get_modifier_state(GdkEvent* event);

/*** GdkKeyEvent ***/
GdkModifierType gdk_key_event_get_consumed_modifiers(GdkEvent* event);
guint gdk_key_event_get_keyval(GdkEvent* event);
GdkModifierType gdk_event_get_modifier_state(GdkEvent* event);

#endif
