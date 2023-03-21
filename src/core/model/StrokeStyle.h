/*
 * Xournal++
 *
 * Definition for StrokeStyles
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "LineStyle.h"  // for LineStyle

namespace StrokeStyle {
/** Parse LineStyle from string.
 *
 * @return LineStyle deserialized from string
 */
LineStyle parseStyle(const std::string& style);
/** Convert a LineStyle to a string.
 *
 * @param style to be serialized
 * @return string containing serialized version of LineStyle
 */
std::string formatStyle(const LineStyle& style);
}  // namespace StrokeStyle
