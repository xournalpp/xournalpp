/*
 * Xournal++
 *
 * Dialog for selecting a background color
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>     // for array
#include <functional>

#include <gdk/gdk.h>  // for GdkRGBA
#include <gtk/gtk.h>  // for GtkWindow

#include "util/Color.h"
#include "util/raii/GtkWindowUPtr.h"

class Control;

/**
 * Count of last background colors
 */
const int LAST_BACKGROUND_COLOR_COUNT = 9;


class SelectBackgroundColorDialog {
public:
    SelectBackgroundColorDialog(Control* control, std::function<void(Color)> callback);

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void storeLastUsedValuesInSettings(GdkRGBA color);

private:
    Control* control{nullptr};

    /**
     * Last used background colors (stored in settings)
     */
    std::array<GdkRGBA, LAST_BACKGROUND_COLOR_COUNT> lastBackgroundColors{};

    xoj::util::GtkWindowUPtr window;
    std::function<void(Color)> callback;
};
