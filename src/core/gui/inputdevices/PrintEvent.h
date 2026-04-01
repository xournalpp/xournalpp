/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <ostream>

#include <gdk/gdk.h>

#include "InputEvents.h"

namespace xoj::input {
void printEvent(std::ostream& str, const InputEvent& e, guint32 timeRef);
void printGdkEvent(std::ostream& str, GdkEvent* e, guint32 timeRef);
};  // namespace xoj::input
