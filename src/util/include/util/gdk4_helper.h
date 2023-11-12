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

GdkModifierType gdk_event_get_modifier_state(GdkEvent* event);
