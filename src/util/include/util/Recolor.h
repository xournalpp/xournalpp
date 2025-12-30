/*
 * Xournal++
 *
 * Color utility, does color conversions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once
#include "util/Color.h"


class Recolor {

public:
    Recolor(const ColorU8& light, const ColorU8& dark);
    Recolor() = default;

public:
    const ColorU8& getDark() const;
    const ColorU8& getLight() const;

    ColorU8 convertColor(const ColorU8& other) const;

    /**
     * recolors the current cairo region (may be preceeded by clip)
     * recoloring is done by
     * 1. invert the whole background
     * 2. scale (multiply) the color spectrum to fit between low/dark and high/light
     * 3. move (add) the color spectrum to lay exactly in range low/dark and high/light
     *
     * @param cr the cairo context to work on
     */
    void recolorCurrentCairoRegion(cairo_t* cr) const;

private:
    constexpr friend bool operator==(Recolor const& lhs, Recolor const& rhs) {
        return lhs.difference == rhs.difference && lhs.offset == rhs.offset;
    }

    void recalcDiffAndOff();

private:
    // parameters set by the user also needed to save the settings to file
    ColorU8 dark = {};
    ColorU8 light = {};

    // calculated from above parameters to avoid having to calculate them for every recoloring all over again
    ColorU8 difference = {};  // abs(dark - light)
    ColorU8 offset = {};      // min(dark, light)
};
