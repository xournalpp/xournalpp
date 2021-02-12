/**
 * @file NamedColor.h
 * @author idotobi
 * @brief Struct for NamedColor
 *
 */

#pragma once
#include <string>

#include "util/Color.h"

struct Palette;
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
     * @brief Construct a new NamedColor instance
     * This automatically sets the isPaletteColor to negative as NamedColors from a color palette can only be
     * constructed using the input operator.
     * As it is no paletteColor the Name of the Color will be set to "Custom Color" and the paletteIndex to 0.
     *
     * @param color color to set for the custom NamedColor instance
     */
    NamedColor(const Color& color);

    /**
     * @brief Input operator for parsing NamedColor from input string stream
     *
     * @param str input string stream
     * @param namedColor output for parsed NamedColor
     * @return std::istream& which is the rest of the input string stream after parsing
     */
    friend std::istream& operator>>(std::istream& str, NamedColor& namedColor);

    /**
     * @brief Get the color formatted as ColorU16 object
     *
     * @return ColorU16
     */
    ColorU16 getColorU16() const;

    /**
     * @brief Get the color formatted as Color object
     *
     * @return Color
     */
    Color getColor() const;

    /**
     * @brief Get the Index of the NamedColor inside the Palette
     *
     * @return size_t index
     */
    size_t getIndex() const;

    /**
     * @brief Get the Name of the NamedColor
     *
     * @return std::string name
     */
    std::string getName() const;

private:
    /**
     * @brief Palette is defined as friend class to allow it to set the paletteIndex
     *
     */
    friend struct Palette;

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
