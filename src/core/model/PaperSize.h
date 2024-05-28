/*
 * Xournal++
 *
 * Paper size type and paper orientation enum
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
#include "gui/PaperFormatUtils.h"

#pragma once

enum class PaperOrientation : bool { HORIZONTAL = false, VERTICAL = true };

class PaperSize {
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
    [[nodiscard]] auto orientation() const -> PaperOrientation;

    // Constructors
    explicit PaperSize(const xoj::util::GtkPaperSizeUPtr& gtkPaperSize);
    explicit PaperSize(const PageTemplateSettings& model);
    PaperSize(double width, double height);
};