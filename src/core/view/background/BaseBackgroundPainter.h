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
