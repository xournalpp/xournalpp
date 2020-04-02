/*
 * Xournal++
 *
 * A presentation laser that follows the mouse cursor.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "model/Point.h"
#include "util/Rectangle.h"

class PresentationLaser {
public:
    /**
     * Creates a presentation laser to be displayed on the widget. Will automatically set up timers and regularly
     * update.
     *
     * @param widget The widget to display the presentation laser on.
     */
    PresentationLaser(GtkWidget* widget);
    PresentationLaser(const PresentationLaser&) = delete;
    PresentationLaser(const PresentationLaser&&) = delete;
    virtual ~PresentationLaser();

    /** Moves the presentation laser based on the current target. */
    void step();

    /** Speed per update (should be positive) */
    double speed = 7.5;

    /** The target position in canvas coordinates */
    Point target{};

    /** Laser color in RGBA (default: red) */
    unsigned int color = 0xFF0000FF;

    void draw(cairo_t* cr);

    /** Bounds of the lasers in canvas coordinates */
    Rectangle<double> getDrawBounds();

private:
    GtkWidget* widget;

    /** The current x coordinate in canvas coordinates */
    double x = 0.0;
    /** The current y coordinate in canvas coordinates */
    double y = 0.0;

    /** The ID of the timeout event */
    guint timeoutId = -1;

    /** Updates per second */
    static constexpr guint DELTA = 1000 * (1.0 / 60.0);

    /** Radius of laser */
    static constexpr double LASER_RADIUS = 5.0;
};
