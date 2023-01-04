/*
 * Xournal++
 *
 * Paper size type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <memory>
#include <variant>
#include <vector>

#include <gtk/gtk.h>

#include "control/settings/PageTemplateSettings.h"
#include "util/PaperFormatUtils.h"

#pragma once

class PaperSize {
private:
    static bool areDoublesEqual(double x, double y);

public:
    /*
     * Width of the paper stored in the standard unit "points"
     */
    double width;
    /*
     * Height of the paper stored in the standard unit "points"
     */
    double height;

    /**
     * @brief Checks that the dimensions and orientation equal (width and height cannot be swapped)
     * @param other The second PaperSize struct
     * @return Whether both PaperSize structs are exactly equal
     */
    auto operator==(const PaperSize& other) const -> bool;
    auto operator!=(const PaperSize& other) const -> bool;

    /**
     * @brief Checks that the dimensions are equal, not checking for equality of orientation (width and height can be
     * swapped)
     * @param other The second PaperSize struct
     * @return Whether both PaperSize structs have equal dimensions
     */
    auto equalDimensions(const PaperSize& other) const -> bool;

    /**
     * @brief Swapping width and height, effectively reversing orientation
     */
    void swapWidthHeight();
    auto orientation() const -> GtkOrientation;

    // Constructors
    explicit PaperSize(const PaperFormatUtils::GtkPaperSizeUniquePtr_t& gtkPaperSize);
    explicit PaperSize(const PageTemplateSettings& model);
    PaperSize(double width, double height);
};