/*
 * Xournal++
 *
 * Helper function
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkWidget

class PageType;

namespace xoj::helper {
constexpr auto PREVIEW_WIDTH = 100;
constexpr auto PREVIEW_HEIGHT = 141;
/**
 * @brief Create a GtkImage containing a miniature of the given (standard) page type
 *      The returned widget is a floating ref.
 */
auto createPreviewImage(const PageType& pt) -> GtkWidget*;
};  // namespace xoj::helper
