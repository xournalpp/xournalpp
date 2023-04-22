/*
 * Xournal++
 *
 * Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_surface_t, cairo_t

#include "sidebar/previews/page/SidebarPreviewPageEntry.h"

class Control;

class PagePreviewDecoration {
public:
    static void drawDecoration(cairo_t* cr, SidebarPreviewPageEntry* pageEntry, Control* control);

private:
    static void drawPageNumberBelowPreview(cairo_t* cr, SidebarPreviewPageEntry* pageEntry, Control* control);
    static void drawPageNumberWithCircleBackground(cairo_t* cr, SidebarPreviewPageEntry* pageEntry, Control* control);
    static void drawPageNumberWithSquareBackground(cairo_t* cr, SidebarPreviewPageEntry* pageEntry, Control* control);

public:
    /* For the style with the page numbers below the preview the widget height has to be extended by this amount */
    static constexpr double MARGIN_BOTTOM = 20.0;

private:
    /* The font sized used for drawing the page number*/
    static constexpr double FONT_SIZE = 16.0;

    /* Line width for drawing the circle and the square background*/
    static constexpr double LINE_WIDTH = 1.0;

    /* Indention of the circle from the right border as percentage to the total widget width */
    static constexpr double CIRCLE_INDENT_RIGHT = 0.28;
    /* Indention of the circle from the bottom border as percentage to the total widget height */
    static constexpr double CIRCLE_INDENT_BOTTOM = 0.2;
    /* Transparency value for the circle background */
    static constexpr double CIRCLE_LABEL_OPACITY = 0.6;
    /* Circle radius in pixels */
    static constexpr double CIRCLE_RADIUS = 11;
    /* Vertical alignment of page number to circle center */
    static constexpr double CIRCLE_TEXT_Y = 6;
    /* Padding for the text in the circle */
    static constexpr double CIRCLE_TEXT_PADDING = 8;

    /* Height and minimum width of background square in pixels */
    static constexpr double SQUARE_SIZE = 20;
    /* Pixel-Distance form widgets right border to previews right border */
    static constexpr double SQUARE_INDENT_RIGHT = 12;
    /* Pixel-Distance from widgets bottom border to previews bottom border */
    static constexpr double SQUARE_INDENT_BOTTOM = 12;
    /* Vertical alignment of the page number in the square */
    static constexpr double SQUARE_TEXT_Y = 16;
    /* Padding for the text in the circle, to be applied on both sides*/
    static constexpr double SQUARE_TEXT_PADDING = 4;

    static constexpr double PREVIEW_BORDER_LINE_WIDTH = 2.0;
    static constexpr double PREVIEW_UPPER_LEFT = 8.5;
    static constexpr double PREVIEW_WIDTH_INDENT = 21;
    static constexpr double PREVIEW_HEIGHT_INDENT = 21;

    static constexpr char FONT_NAME[] = "Sans";
};
