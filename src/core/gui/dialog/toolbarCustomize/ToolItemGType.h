/*
 * Xournal++
 *
 * Custom GType for toolbar customization drag'n'drop operations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib-object.h>

namespace xoj::dnd {
GType get_tool_item_gtype();
const char* g_value_get_tool_item_string(GValue* value);
}  // namespace xoj::dnd
