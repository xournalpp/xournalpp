/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>     // for array
#include <optional>  // for optional

#include <gdk/gdk.h>  // for GdkRGBA
#include <gtk/gtk.h>  // for GtkWindow

#include "util/Color.h"  // for Color

class Control;

/**
 * Count of last background colors
 */
const int LAST_BACKGROUND_COLOR_COUNT = 9;


class SelectBackgroundColorDialog {
public:
    explicit SelectBackgroundColorDialog(Control* control);

public:
    void show(GtkWindow* parent);

    /**
     * Return the selected color as RGB, nullopt if no color is selected
     */
    auto getSelectedColor() const -> std::optional<Color>;

private:
    void storeLastUsedValuesInSettings();

private:
    Control* control{nullptr};

    /**
     * Last used background colors (stored in settings)
     */
    std::array<GdkRGBA, LAST_BACKGROUND_COLOR_COUNT> lastBackgroundColors{};

    /**
     * Selected color
     */
    std::optional<Color> selected;
};
