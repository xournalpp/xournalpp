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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "model/PageRef.h"

#include "BackgroundConfig.h"
#include "XournalType.h"

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
    /**
     * Primary foreground color:
     * f1=XXXXXX
     * Primary foreground color used with dark backgrounds:
     * af1=XXXXXX
     * e.g. FF0000 for Red
     */
    int foregroundColor1 = 0;
    int alternativeForegroundColor1 = 0;

    /**
     * Secondary foreground color:
     * f2=XXXXXX
     * Secondary foreground color used with dark backgrounds:
     * af2=XXXXXX
     * e.g. FF0000 for Red
     */
    int foregroundColor2 = 0;
    int alternativeForegroundColor2 = 0;

    /**
     * Line width:
     * lw=1.23
     * Eg: With of lines used in graph view
     */
    double lineWidth = 0;

    /**
     *  With of the boxes the gaph view
     * r1=1.23
     */
    double drawRaster1 = 1;

    /**
     * Margin size used in various views:
     * m1=40
     */
    double margin1 = 0;

    /**
     * rm=1
     * Round margin = 0 => No rounding
     * Round margin = 1 => Round to next grid etc.
     */
    int roundMargin = 0;

    /**
     * Line width factor, to use to draw Previews
     */
    double lineWidthFactor = 1;
    int alternativeColor(int color1, int color2);
};
