/*
 * Xournal++
 *
 * Base class for Background paints (This class fills the background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "model/PageRef.h"
#include "util/Color.h"

#include "BackgroundConfig.h"

class BaseBackgroundPainter {
public:
    BaseBackgroundPainter();
    virtual ~BaseBackgroundPainter();

public:
    virtual void paint(cairo_t* cr, PageRef page, BackgroundConfig* config);
    virtual void paint();

    /**
     * Reset all used configuration values
     */
    virtual void resetConfig();

    /**
     * Set a factor to draw the lines bolder, for previews
     */
    void setLineWidthFactor(double factor);

protected:
    void paintBackgroundColor();

    /**
     * Choose between color1 and color2 based on the page's background brightness.
     *
     * @param color1 A color intended for light backgrounds.
     * @param color2 A color intended for dark backgrounds.
     *
     * @return color1 if the page background is light, else, color2.
     */
    Color alternativeColor(Color color1, Color color2) const;

    /**
     * Determines and returns the primary foreground color to use for this page.
     *
     * @return the primary foreground color for the page.
     */
    Color getForegroundColor1() const;

    /**
     * Determines and returns the secondary foreground color to use for this page.
     *
     * @return the secondary foreground color for the page.
     */
    Color getForegroundColor2() const;

private:
protected:
    BackgroundConfig* config = nullptr;
    PageRef page;
    cairo_t* cr = nullptr;

    double width = 0;
    double height = 0;

    // Drawing attributes
    // ParserKey=Value
protected:
    Color defaultForegroundColor1{0U};
    Color defaultAlternativeForegroundColor1{0U};

    Color defaultForegroundColor2{0U};
    Color defaultAlternativeForegroundColor2{0U};

    Color foregroundColor1{0U};
    Color foregroundColor2{0U};

    double lineWidth = 0;

    double drawRaster1 = 1;

    double margin1 = 0;

    /**
     * rm=1
     * Round margin = 0 => No rounding
     * Round margin = 1 => Round to next grid etc.
     * Todo(fabian): use enum RoundMargin : bool{ DoNotRound = false, RoundToNextGrid = true};
     */
    int roundMargin = 0;

    /**
     * Line width factor, to use to draw Previews
     */
    double lineWidthFactor = 1;
};
