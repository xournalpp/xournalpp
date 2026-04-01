/*
 * Xournal++
 *
 * Abstract class for creating popovers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkWidget

class PopoverFactory {
public:
    PopoverFactory() = default;
    virtual ~PopoverFactory() = default;

    /// Creates a GtkPopover. The returned ref is floating
    virtual GtkWidget* createPopover() const = 0;
};
