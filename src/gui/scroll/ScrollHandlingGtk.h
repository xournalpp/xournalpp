/*
 * Xournal++
 *
 * Scroll handling for GTK standard implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "ScrollHandling.h"

/**
 * Scroll handling based on a GtkScrolledWindow.
 *
 * In this scroll handler, the GtkDrawingArea is placed inside of a
 * GtkScrolledWindow. When the layout size changes, the GtkDrawingArea is
 * resized to the new layout size.
 */
class ScrollHandlingGtk: public ScrollHandling {
public:
    ScrollHandlingGtk(GtkScrollable* scrollable);
    ~ScrollHandlingGtk() override;

public:
    void setLayoutSize(int width, int height) override;

    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2) override;
    void translate(double& x, double& y) override;

    bool fullRepaint() override;

    void scrollChanged() override;

private:
};
