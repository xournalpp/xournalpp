/*
 * Xournal++
 *
 * constexpr names of the attributes used in .xoj and .xopp files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>


// Names of attributes
namespace xoj::xml_attrs {

// xournal
constexpr auto CREATOR_STR = u8"creator";
constexpr auto VERSION_STR = u8"version";  // also in MrWriter
constexpr auto FILEVERSION_STR = u8"fileversion";

// page
constexpr auto WIDTH_STR = u8"width";  // also in stroke
constexpr auto HEIGHT_STR = u8"height";

// background
constexpr auto NAME_STR = u8"name";  // also in layer
constexpr auto TYPE_STR = u8"type";
constexpr auto STYLE_STR = u8"style";  // also in stroke
constexpr auto CONFIG_STR = u8"config";
constexpr auto COLOR_STR = u8"color";  // also in stroke and text
constexpr auto DOMAIN_STR = u8"domain";
constexpr auto FILENAME_STR = u8"filename";
constexpr auto PAGE_NUMBER_STR = u8"pageno";

// timestamp
constexpr auto AUDIO_FILENAME_STR = u8"fn";  // also in stroke, text and audio
constexpr auto TIMESTAMP_STR = u8"ts";       // also in stroke and text

// stroke
constexpr auto TOOL_STR = u8"tool";
constexpr auto PRESSURES_STR = u8"pressures";
constexpr auto FILL_STR = u8"fill";
constexpr auto CAPSTYLE_STR = u8"capStyle";

// text
constexpr auto FONT_STR = u8"font";
constexpr auto SIZE_STR = u8"size";
constexpr auto X_COORD_STR = u8"x";
constexpr auto Y_COORD_STR = u8"y";

// image
constexpr auto LEFT_POS_STR = u8"left";      // also in teximage
constexpr auto TOP_POS_STR = u8"top";        // also in teximage
constexpr auto RIGHT_POS_STR = u8"right";    // also in teximage
constexpr auto BOTTOM_POS_STR = u8"bottom";  // also in teximage

// teximage
constexpr auto TEXT_STR = u8"text";

// attachment
constexpr auto PATH_STR = u8"path";


class Domain {
public:
    enum Value { ABSOLUTE, ATTACH, CLONE };
    static constexpr std::array<const char8_t*, 3> NAMES = {u8"absolute", u8"attach", u8"clone"};
    Domain(Value v): value(v) {}

    // Implicit conversion to underlying enum type
    operator const Value&() const { return value; }
    operator Value&() { return value; }

private:
    Value value;
};

// Named enum for the values that the "type" attribute can take in .xopp and
// .xoj files. It is less detailed than the PageType class.
class BackgroundType {
public:
    enum Value { SOLID, PIXMAP, PDF };
    static constexpr std::array<const char8_t*, 3> NAMES = {u8"solid", u8"pixmap", u8"pdf"};
    BackgroundType(Value v): value(v) {}

    // Implicit conversion to underlying enum type
    operator const Value&() const { return value; }
    operator Value&() { return value; }

private:
    Value value;
};

}  // namespace xoj::xml_attrs
