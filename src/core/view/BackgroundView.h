/*
 * Xournal++
 *
 * Displays a background
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "View.h"

class xoj::view::BackgroundView {
public:
    BackgroundView(double pageWidth, double pageHeight): pageWidth(pageWidth), pageHeight(pageHeight) {}

    /**
     * @brief Draws the background on the entire mask represented by the cairo context ctx.cr
     */
    virtual void draw(cairo_t* cr) const = 0;

protected:
    double pageWidth;
    double pageHeight;
};
