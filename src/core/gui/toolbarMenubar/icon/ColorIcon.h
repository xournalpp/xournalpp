/*
 * Xournal++
 *
 * Icon for color buttons
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget

#include "util/Color.h"  // for Color
#include "util/raii/GObjectSPtr.h"

namespace ColorIcon {
/**
 * @brief Create a new GtkImage with preview color
 * @return The pointer is a floating ref
 */
GtkWidget* newGtkImage(Color color, bool circle = true, std::optional<Color> secondaryColor = std::nullopt);

/**
 * @brief Create a new GdkPixbuf* with preview color
 */
xoj::util::GObjectSPtr<GdkPixbuf> newGdkPixbuf(Color color, bool circle = true,
                                               std::optional<Color> secondaryColor = std::nullopt);
};  // namespace ColorIcon
