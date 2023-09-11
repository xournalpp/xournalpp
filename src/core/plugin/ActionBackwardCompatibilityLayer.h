/*
 * Xournal++
 *
 * Compat. layer for mapping old plugin action triggers to new actions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "config-features.h"  // for ENABLE_PLUGINS

#ifdef ENABLE_PLUGINS

#include <string_view>

class Control;

namespace ActionBackwardCompatibilityLayer {
/**
 * @brief Translates the old ActionType type to a new Action and fires it.
 * @param enabled Used for boolean actions only
 */
void actionPerformed(Control* ctrl, std::string_view type, bool enabled);
};  // namespace ActionBackwardCompatibilityLayer
#endif
