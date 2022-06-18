/**
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 * @brief Struct for NamedColor
 */

#pragma once
#include <cstddef>  // for size_t
#include <iosfwd>   // for istream
#include <string>   // for string, basic_string

#include "util/Color.h"  // for Color, ColorU16, ColorU8

/**
 * @brief NamedColor is an object for a color with name and an optional index which refers to the index of the color
 * inside a given palette.
 * If the NamedColor does not come from a Color palette isColorPalette is set to false and the palettIndex is 0.
 *
 */
struct NamedColor {
    /**
     * @brief Default Construct a new Named Color instance
     * This is only needed for the input operator implementation
     *
     */
    NamedColor();

    /**
     * @brief Construct a new NamedColor instance for a Color Palette
     * The paletteIndex should be the index which the color will have in the palette.
     * This is to be used as:
     *
     *     NamedColor color{namedColors.size()};
     *     std::istringstream iss{line};
     *     if (iss >> color) {
     *        ...
     *     }
     *
     * @param paletteIndex
     */
    explicit NamedColor(size_t paletteIndex);

    /**
     * @brief Construct a new NamedColor instance
     * This automatically sets the isPaletteColor to negative as NamedColors from a color palette can only be
     * constructed using the input operator.
     * As it is no paletteColor the Name of the Color will be set to "Custom Color" and the paletteIndex to 0.
     *
     * @param color color to set for the custom NamedColor instance
     */
    explicit NamedColor(Color color);

    /**
     * @brief Input operator for parsing NamedColor from input string stream
     *
     * @param str input string stream
     * @param namedColor output for parsed NamedColor
     * @return std::istream& which is the rest of the input string stream after parsing
     */
    friend auto operator>>(std::istream& str, NamedColor& namedColor) -> std::istream&;

    /**
     * @brief Get the color formatted as ColorU16 object
     *
     * @return ColorU16
     */
    auto getColorU16() const -> ColorU16;

    /**
     * @brief Get the color formatted as Color object
     *
     * @return Color
     */
    auto getColor() const -> Color;

    /**
     * @brief Get the Index of the NamedColor inside the Palette
     *
     * @return size_t index
     */
    auto getIndex() const -> size_t;

    /**
     * @brief Get the Name of the NamedColor
     *
     * @return std::string name
     */
    auto getName() const -> std::string const&;

private:
    /**
     * @brief Index of the Namedcolor inside the palette
     * This is useful for loading the toolbar from the toolbar.ini
     *
     */
    size_t paletteIndex;

    std::string name;
    ColorU16 colorU16;
    Color color;

    /**
     * @brief Flag whether a color comes from a palette or is a custom color.
     *
     */
    bool isPaletteColor;
};
