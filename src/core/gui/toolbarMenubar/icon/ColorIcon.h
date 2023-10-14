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

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget

#include "util/Color.h"  // for Color
#include "util/raii/GObjectSPtr.h"

namespace ColorIcon {
/**
 * @brief Create a new GtkImage with preview color
 * @return The pointer is a floating ref
 */
GtkWidget* newGtkImage(Color color, int size = 22, bool circle = true);

/**
 * @brief Create a new GdkPixbuf* with preview color
 */
xoj::util::GObjectSPtr<GdkPixbuf> newGdkPixbuf(Color color, int size = 22, bool circle = true);
};  // namespace ColorIcon
