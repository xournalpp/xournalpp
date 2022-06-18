#pragma once

#include <cstddef>    // for size_t
#include <exception>  // for exception
#include <iosfwd>     // for istream
#include <map>        // for map
#include <string>     // for string, basic_string
#include <vector>     // for vector

#include "util/NamedColor.h"  // for NamedColor

#include "filesystem.h"  // for path

/**
 * @brief Header entity of a gpl file
 * This is simply a key value pair. The main reason to define a struct for this is to define a custom input operator.
 *
 */
struct Header {
    /**
     * @brief Input operator for parsing Header from input string stream
     *
     * @param str input string stream
     * @param header output for parsed Header
     * @return std::istream& which is the rest of the input string stream after parsing
     */
    friend std::istream& operator>>(std::istream& str, Header& header);

    std::string getAttribute() const;
    std::string getValue() const;

private:
    /**
     * @brief Attribute name given in gpl file
     *
     */
    std::string attribute;

    /**
     * @brief Value for corresponding attribute in gpl file
     *
     */
    std::string value;

    /**
     * @brief Swap all properties between two Header instances
     *
     * @param other other Header instance to swap properties with
     */
    void swap(Header& other);
};

/**
 * @brief ColorPalette coming from a parsed gpl file
 *
 */
struct Palette {
    /**
     * @brief Construct a new Palette
     * This does not yet parse the file. It simply sets up the class.
     *
     * @param path to the gpl file
     */
    Palette(fs::path path);

    /**
     * @brief Create default gpl file string
     * Default xournalpp color scheme
     *   0 0 0 Black
     *   0 128 0 Green
     *   0 192 255 Light Blue
     *   0 255 0 Light Green
     *   51 51 204 Blue
     *   128 128 128 Gray
     *   255 0 0 Red
     *   255 0 255 Magenta
     *   255 128 0 Orange
     *   255 255 0 Yellow
     *   255 255 255 White
     *
     * @return const std::string
     */
    static const std::string default_palette();

    /**
     * @brief Create a default gpl file for the xournalpp color scheme
     * See default_palette() for further information.
     *
     * @param path to write gpl file to
     */
    static void create_default(fs::path path);

    /**
     * @brief Parse the gpl file in filepath
     * This fills the header map and the namedColors vector after clearing them first
     *
     * @throw exception in case of wrong formatted file
     *
     */
    void load();

    /**
     * @brief Load the default Palette
     *
     */
    void load_default();

    /**
     * @brief Show a Dialog with only an OK button
     *
     * @param message to be displayed in the dialog
     */
    void parseErrorDialog(const std::exception& e) const;

    /**
     * @brief Get the number of namedColors part of the palette
     *
     * @return size_t number of namedColors in palette
     */
    size_t size() const;

    /**
     * @brief Get a NamedColor from the palette
     *
     * @param i palette index
     * @return NamedColor of palette at palette index
     */
    NamedColor const& getColorAt(size_t i) const;


private:
    /**
     * @brief filepath to the gpl file the palette is coming from
     *
     */
    fs::path filepath;

    /**
     * @brief Vector containing all colors of the palette
     *
     */
    std::vector<NamedColor> namedColors;

    /**
     * @brief Map containing the key value pairs of the gpl header
     * e.g. "Palette Name", "Description"
     *
     */
    std::map<std::string, std::string> header;

    /**
     * @brief Verify if line contains the expected "GIMP Palette" string
     *
     * @param line first line of gpl file
     * @return true if line is "GIMP Palette"
     * @return false otherwise
     */
    bool parseFirstGimpPaletteLine(const std::string& line) const;

    /**
     * @brief Try to parse a color and add it to namedColors
     *
     *
     * @param line line of gpl file
     * @return true if line is of form "255 255 255 White" (r g b name)
     * @return false otherwise
     */
    bool parseColorLine(const std::string& line);

    /**
     * @brief Try to parse a header key value pair and add it to header
     *
     * @param line line of gpl file
     * @return true if line is of form "key:value"
     * @return false otherwise
     */
    bool parseHeaderLine(const std::string& line);

    /**
     * @brief Parse a Comment line
     * Every line starting with "#" will be considered a comment
     *
     * @param line
     * @return true if line starts with "#"
     * @return false else
     */
    bool parseCommentLine(const std::string& line) const;

    /**
     * @brief Fallback for line parsing
     * This function is the fallback in case no other parsing attempt works.
     * In this case this function throws an error containing the non-parseable line number
     *
     * This should be used in the following way
     *
     *     firstLineParseAttempt(line) || secondLineParseAttempt(line) || parseLineFallback(lineNumber)
     *
     * @param lineNumber of the erroneous line
     * @return nothing as it always throws an error
     * @throw invalid_argument exception if not a comment
     */
    const bool parseLineFallback(int lineNumber) const;
};
