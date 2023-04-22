#include "PagePreviewDecoration.h"

#include <math.h>

#include "control/Control.h"
#include "control/settings/Settings.h"
#include "util/raii/CairoWrappers.h"

void PagePreviewDecoration::drawDecoration(cairo_t* cr, SidebarPreviewPageEntry* pageEntry, Control* control) {
    switch (control->getSettings()->getSidebarNumberingStyle()) {
        case SidebarNumberingStyle::NUMBER_BELOW_PREVIEW:
            PagePreviewDecoration::drawPageNumberBelowPreview(cr, pageEntry, control);
            break;
        case SidebarNumberingStyle::NUMBER_WITH_CIRCULAR_BACKGROUND:
            PagePreviewDecoration::drawPageNumberWithCircleBackground(cr, pageEntry, control);
            break;
        case SidebarNumberingStyle::NUMBER_WITH_SQUARE_BACKGROUND:
            PagePreviewDecoration::drawPageNumberWithSquareBackground(cr, pageEntry, control);
            break;
        default:
            break;
    }
}

void PagePreviewDecoration::drawPageNumberBelowPreview(cairo_t* cr, SidebarPreviewPageEntry* pageEntry,
                                                       Control* control) {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_text_extents_t extents;
    std::string pageNumber = std::to_string(pageEntry->getIndex() + 1);
    Color color = control->getSettings()->isDarkTheme() ? Colors::white : Colors::xopp_darkslategray;
    if (pageEntry->isSelected()) {
        color = control->getSettings()->getBorderColor();
    }
    Util::cairo_set_source_rgbi(cr, color);
    cairo_select_font_face(cr, FONT_NAME, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);
    cairo_text_extents(cr, pageNumber.c_str(), &extents);
    double x = pageEntry->getWidth() / 2 - (extents.width / 2 + extents.x_bearing);
    double y = pageEntry->getHeight() - MARGIN_BOTTOM - (extents.height / 2 + extents.y_bearing);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, pageNumber.c_str());
}

void PagePreviewDecoration::drawPageNumberWithCircleBackground(cairo_t* cr, SidebarPreviewPageEntry* pageEntry,
                                                               Control* control) {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_text_extents_t extents;
    std::string pageNumber = std::to_string(pageEntry->getIndex() + 1);
    cairo_select_font_face(cr, FONT_NAME, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);
    cairo_text_extents(cr, pageNumber.c_str(), &extents);

    // Set the color for drawing the circle. This circle is required to provide good contrast for the page number.
    if (pageEntry->isSelected()) {
        Color color = control->getSettings()->getBorderColor();
        Util::cairo_set_source_rgbi(cr, color, CIRCLE_LABEL_OPACITY);
    } else {
        Util::cairo_set_source_rgbi(cr, Colors::xopp_darkslategray, CIRCLE_LABEL_OPACITY);
    }

    // How many pixels to indent the circle center from the widget border
    double indentRight = pageEntry->getWidth() * CIRCLE_INDENT_RIGHT;
    double indentBottom = pageEntry->getHeight() * CIRCLE_INDENT_BOTTOM;

    // Pixel position of circle center
    double x = pageEntry->getWidth() - indentRight;
    double y = pageEntry->getHeight() - indentBottom;

    // Inner circle width depends on the actual text width
    double addititionalWidth = std::max((extents.width - 2 * CIRCLE_RADIUS) + CIRCLE_TEXT_PADDING, 0.0);

    // Draw an slightly transparent circle in the lower right corner of the preview
    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_arc(cr, x - addititionalWidth, y, CIRCLE_RADIUS, M_PI * 0.5, M_PI * 1.5);
    cairo_arc(cr, x, y, CIRCLE_RADIUS, M_PI * 1.5, M_PI * 0.5);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    // Draw the page number in the center of the previously drawn circle
    cairo_move_to(cr, x - addititionalWidth / 2 - (extents.width / 2 + extents.x_bearing), y + CIRCLE_TEXT_Y);
    Util::cairo_set_source_argb(cr, Colors::white);
    cairo_show_text(cr, pageNumber.c_str());
}

void PagePreviewDecoration::drawPageNumberWithSquareBackground(cairo_t* cr, SidebarPreviewPageEntry* pageEntry,
                                                               Control* control) {
    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_text_extents_t extents;
    std::string pageNumber = std::to_string(pageEntry->getIndex() + 1);
    cairo_select_font_face(cr, FONT_NAME, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);
    cairo_text_extents(cr, pageNumber.c_str(), &extents);

    // Select the color for painting the square background
    if (pageEntry->isSelected()) {
        Util::cairo_set_source_rgbi(cr, control->getSettings()->getBorderColor());
    } else {
        Util::cairo_set_source_rgbi(cr, Colors::xopp_darkslategray);

        // In case the page is not selected draw a border around the preview to match the selected
        // See discussion: <https://github.com/xournalpp/xournalpp/issues/4624#issue-1557719574>
        cairo_set_line_width(cr, PREVIEW_BORDER_LINE_WIDTH);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        cairo_rectangle(cr, PREVIEW_UPPER_LEFT, PREVIEW_UPPER_LEFT, pageEntry->getWidth() - PREVIEW_WIDTH_INDENT,
                        pageEntry->getHeight() - PREVIEW_HEIGHT_INDENT);
        cairo_stroke(cr);
    }

    // Calculate the rectangle width depending on the page index number of digits
    double recWidth = std::max(extents.width + 8, SQUARE_SIZE);

    // Calculate the position of the upper left corner of the square
    double x = pageEntry->getWidth() - SQUARE_INDENT_RIGHT - recWidth;
    double y = pageEntry->getHeight() - SQUARE_INDENT_BOTTOM - SQUARE_SIZE;

    // Draw an square in the lower right corner
    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_rectangle(cr, x, y, recWidth, SQUARE_SIZE);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    // Draw the page number in the center of the square
    cairo_move_to(cr, x + recWidth / 2 - (extents.width / 2 + extents.x_bearing), y + SQUARE_TEXT_Y);
    Util::cairo_set_source_argb(cr, Colors::white);
    cairo_show_text(cr, pageNumber.c_str());
}
