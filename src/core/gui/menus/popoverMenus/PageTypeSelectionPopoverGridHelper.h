/*
 * Xournal++
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <gtk/gtk.h>  // for GtkWidget

class PageTypeInfo;

namespace xoj::helper::PageTypeGrid {
static constexpr auto PAGE_TYPES_PER_ROW = 4;

GtkGrid* createEmptyGrid();

/**
 * @brief Create a grid containing a miniature for each one the given (standard) page types
 *      The returned widget is a floating ref.
 */
GtkWidget* createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                             const std::string_view& prefixedActionName);
}  // namespace xoj::helper::PageTypeGrid
