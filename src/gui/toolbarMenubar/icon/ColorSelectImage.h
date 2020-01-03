/*
 * Xournal++
 *
 * Icon for color buttons
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

#include "XournalType.h"

enum ColorIconState {
    /**
     * Draw color icon enabled
     */
    COLOR_ICON_STATE_ENABLED,

    /**
     * Draw color icon disabled
     */
    COLOR_ICON_STATE_DISABLED,

    /**
     * Draw color icon with a pen symbol, switch to pen when clicked
     */
    COLOR_ICON_STATE_PEN
};

class IconConfig {
public:
    /**
     * Color of the icon
     */
    int color = 0;

    /**
     * Size of the icon
     */
    int size = 16;

    /**
     * Draw as circle
     */
    bool circle = true;

    /**
     * Size of the widget
     */
    int width = 16;

    /**
     * Size of the widget
     */
    int height = 16;

    /**
     * State of the icon
     */
    ColorIconState state = COLOR_ICON_STATE_ENABLED;
};

class ColorSelectImage {
public:
    ColorSelectImage(int color, bool circle);
    virtual ~ColorSelectImage();

public:
    /**
     * @return The widget which is drawn
     */
    GtkWidget* getWidget();

    /**
     * Color of the icon
     */
    void setColor(int color);

    /**
     * Set State of the Icon
     */
    void setState(ColorIconState state);

    /**
     * Create a new GtkImage with preview color
     */
    static GtkWidget* newColorIcon(int color, int size = 22, bool circle = true);

    /**
     * Create a new cairo_surface_t* with preview color
     */
    static cairo_surface_t* newColorIconSurface(int color, int size = 22, bool circle = true);

    /**
     * Create a new GdkPixbuf* with preview color
     */
    static GdkPixbuf* newColorIconPixbuf(int color, int size = 22, bool circle = true);

private:
    /**
     * Draw the widget
     */
    void drawWidget(cairo_t* cr);

    /**
     * Draw the widget
     */
    static void drawWidget(cairo_t* cr, const IconConfig& config);

private:
    /**
     * The widget which is drawn
     */
    GtkWidget* widget = nullptr;

    /**
     * Color of the icon
     */
    int color = 0;

    /**
     * Size of the icon
     */
    int size = 16;

    /**
     * Draw as circle
     */
    bool circle = true;

    /**
     * State of the icon
     */
    ColorIconState state = COLOR_ICON_STATE_ENABLED;
};
