/*
 * Xournal++
 *
 * Entry point for the "Zoom Draw" tool: click on a page at a point P while
 * the tool is active to pop up a magnified drawing area centered on P. Any
 * strokes the user draws there are, upon confirmation, transferred back onto
 * the original page at their true position and scale.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"  // for PageRef

class Control;

class ZoomDrawController {
public:
    ZoomDrawController() = delete;

    /**
     * Called by XojPageView when the user releases the mouse button after clicking on the
     * page with the Zoom Draw tool active. Computes the popup's geometry and shows it.
     *
     * @param page The page that was clicked
     * @param control The application's Control instance
     * @param x X coordinate of the click, in page coordinate units (pt), i.e. NOT scaled by
     *          the on-screen zoom level of the main window
     * @param y Y coordinate of the click, in page coordinate units (pt)
     */
    static void activate(const PageRef& page, Control* control, double x, double y);
};
