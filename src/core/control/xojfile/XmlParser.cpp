#include "control/xojfile/XmlParser.h"

#include <algorithm>    // for all_of
#include <cctype>       // for isspace
#include <cstddef>      // for size_t
#include <string>       // for stod, string
#include <string_view>  // for string_view
#include <utility>      // for move
#include <vector>       // for vector

#include <glib.h>  // for GMarkupParseContext, g_warning...

#include "control/pagetype/PageTypeHandler.h"          // for PageTypeHandler
#include "control/xojfile/DocumentBuilderInterface.h"  // for DocumentBuilderInterface
#include "control/xojfile/XmlAttrs.h"                  // for XmlAttrs
#include "control/xojfile/XmlParserHelper.h"           // for getAttrib...
#include "control/xojfile/XmlTags.h"                   // for XmlTags
#include "model/PageType.h"                            // for PageType
#include "model/Point.h"                               // for Point
#include "model/Stroke.h"                              // for StrokeTool, StrokeCapStyle
#include "util/Assert.h"                               // for xoj_assert
#include "util/Color.h"                                // for Color
#include "util/EnumIndexedArray.h"                     // for EnumIndexedArray
#include "util/StringUtils.h"                          // for char_cast, SV_FMT, ...
#include "util/safe_casts.h"                           // for as_unsigned
#include "util/utf8_view.h"                            // for xoj::util::utf8

#include "filesystem.h"  // for path


static constexpr auto& TAG_NAMES = xoj::xml_tags::NAMES;
using TagType = xoj::xml_tags::Type;


template <typename T>
static auto isAllWhitespace(T string) -> bool {
    return std::all_of(string.begin(), string.end(), [](unsigned char ch) { return std::isspace(ch); });
}

/**
 * Attempt parsing a double and print parsing error warnings
 *
 * Skips any leading whitespace and parses the string in the range [it, end) for
 * a floating point value. On success, `it` is updated to point to the first
 * parsed character and the function returns true. When the end of the string is
 * reached before a value was parsed, the function returns false. If a parsing
 * error occured, the function prints an error message and returns false.
 *
 * When available, we use std::from_chars, beacuse it is about 10x faster than
 * streams and about 5x faster than g_ascii_strtod.
 *
 * @param it    Pointer to the beginning of the string, modified to point to the
 *              first unparsed character
 * @param end   Pointer to one character past the end of the string
 * @param value Output parameter for the parsed value
 */
static bool parseDouble(const char*& it, const char* end, double& value) {
    // Skip any leading whitespace
    while (it != end && *it == ' ') {
        ++it;
    }

    // Return false if reached end of string
    if (it == end) {
        return false;
    }

#if HAS_FLOAT_FROM_CHARS
    // Parse double
    const auto [ptr, ec] = std::from_chars(it, end, value);
    if (ec != std::errc{}) {
        g_warning("XML parser: Error parsing a double:\n"
                  "\"%s\"\n"
                  "Remaining string: \"" SV_FMT "\"",
                  std::make_error_condition(ec).message().c_str(), SV_ARG(std::string_view(it, end)));
        return false;
    }
#else
    // g_ascii_strtod expects a null-terminated string. This is always the case
    // with the current implementation of GMarkup, which is unlikely to change.
    xoj_assert(*end == '\0');
    char* ptr = nullptr;
    value = g_ascii_strtod(it, &ptr);
    if (ptr == it) {
        g_warning("XML parser: Error parsing a double. Remaining string: \"" SV_FMT "\"",
                  SV_ARG(std::string_view(it, as_unsigned(end - it))));
        return false;
    }
#endif  // HAS_FLOAT_FROM_CHARS

    // Update start pointer and return
    it = ptr;
    return true;
}

void XmlParser::parserStartElement(GMarkupParseContext* context, const gchar* elementName, const gchar** attributeNames,
                                   const gchar** attributeValues, gpointer userdata, GError** error) {
    auto self = static_cast<XmlParser*>(userdata);
    xoj_assert(self);

    const auto tagType = self->getTagType(elementName | xoj::util::utf8);

    // Check for unknown tags
    if (tagType == TagType::UNKNOWN) {
        if (!self->lastValidTag) {
            if (!self->hierarchy.empty()) {
                g_warning("Ignoring unexpected %s tag at document root.", elementName);
            }
            // If the hierarchy is empty, we will attempt parsing it anyways
        } else {
            g_warning("Ignoring unexpected %s tag under " SV_FMT, elementName,
                      U8SV_ARG(TAG_NAMES[*self->lastValidTag]));
        }
    }

    // Call parsing function
    if (parsingTable[tagType].start) {
        const auto attributes = XmlParserHelper::AttributeMap{attributeNames, attributeValues};
        (self->*parsingTable[tagType].start)(attributes);
    }

    self->hierarchy.push_back(tagType);
    if (tagType != TagType::UNKNOWN) {
        self->lastValidTag = tagType;
    }
}

void XmlParser::parserEndElement(GMarkupParseContext* context, const gchar* elementName, gpointer userdata,
                                 GError** error) {
    auto self = static_cast<XmlParser*>(userdata);
    xoj_assert(self);

    // GMarkup should have already risen an error if there was an error in the document structure.
    xoj_assert(!self->hierarchy.empty());
    const auto tagType = self->hierarchy.back();
    xoj_assert(TAG_NAMES[tagType] == xoj::util::utf8(elementName) || tagType == TagType::UNKNOWN);

    // Check for unknown tags
    if (tagType == TagType::UNKNOWN && self->hierarchy.size() == 1) {
        // We are closing an unknown top level node. Assume it's the end of the document.
        self->builder.finalizeDocument();
    }

    // Call parsing function
    if (parsingTable[tagType].end) {
        (self->builder.*parsingTable[tagType].end)();
    }

    self->hierarchy.pop_back();

    // Track last valid tag
    self->lastValidTag.reset();  // Default for empty hierarchy or no valid tag found
    for (auto it = self->hierarchy.rbegin(); it != self->hierarchy.rend(); ++it) {
        if (*it != TagType::UNKNOWN) {
            self->lastValidTag = *it;
            break;
        }
    }
}

void XmlParser::parserText(GMarkupParseContext* context, const gchar* text, gsize textLen, gpointer userdata,
                           GError** error) {
    auto self = static_cast<XmlParser*>(userdata);
    xoj_assert(self);

    const auto textSV = std::string_view{text, textLen};

    // Check for text at document root
    if (self->hierarchy.empty()) {
        if (!isAllWhitespace(textSV)) {
            g_warning("Ignoring unexpected text at document root: \"" SV_FMT "\"", SV_ARG(textSV));
        }
        return;
    }

    const auto tagType = self->hierarchy.back();

    if (self->parsingTable[tagType].text) {
        // Text may come in separated chunks only if it contains comments or other
        // special instances starting with '<', which we do not expect. This means
        // we always get the whole text in a single callback.
        (self->*parsingTable[tagType].text)(textSV);
    } else if (tagType != TagType::TITLE && tagType != TagType::PREVIEW && !isAllWhitespace(textSV)) {
        g_warning("Unexpected text in " SV_FMT " node: \"" SV_FMT "\"", U8SV_ARG(TAG_NAMES[tagType]), SV_ARG(textSV));
    }
}


XmlParser::XmlParser(DocumentBuilderInterface& builder): builder(builder) {}


void XmlParser::parseUnknownTag(const XmlParserHelper::AttributeMap& attributeMap) {
    if (this->hierarchy.empty()) {
        // Unknown tag at document root. Assume it's another application (like Xournal++ or MrWriter) that has
        // its own tag name, but a similar structure. Attempt parsing anyways.
        this->builder.addDocument(u8"Unknown", 1);
        g_warning("Attempting to parse unknown document type.");
    }
}

void XmlParser::parseXournalTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto optCreator = XmlParserHelper::getAttrib<c_string_utf8_view>(xoj::xml_attrs::CREATOR_STR, attributeMap);
    std::u8string creator;
    if (optCreator) {
        creator = optCreator->str();
    } else {
        // Compatibility: the creator attribute exists since 7017b71. Before that, only a version string was written
        const auto optVersion = XmlParserHelper::getAttrib<string_utf8_view>(xoj::xml_attrs::VERSION_STR, attributeMap);
        if (optVersion) {
            creator = u8"Xournal ";
            creator.append(optVersion->begin(), optVersion->end());
        } else {
            creator = u8"Unknown";
        }
    }

    const auto fileVersion =
            XmlParserHelper::getAttribMandatory<int>(xoj::xml_attrs::FILEVERSION_STR, attributeMap, 1,
                                                     /*do not warn: attribute does not exist in zip files*/ false);

    this->builder.addDocument(std::move(creator), fileVersion);
}

void XmlParser::parseMrWriterTag(const XmlParserHelper::AttributeMap& attributeMap) {
    auto optVersion = XmlParserHelper::getAttrib<string_utf8_view>(xoj::xml_attrs::VERSION_STR, attributeMap);
    std::u8string creator;
    if (optVersion) {
        creator = u8"MrWriter ";
        creator.append(optVersion->begin(), optVersion->end());
    } else {
        creator = u8"Unknown";
    }

    this->builder.addDocument(std::move(creator), 1);
}

void XmlParser::parsePageTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto width = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::WIDTH_STR, attributeMap);
    const auto height = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::HEIGHT_STR, attributeMap);

    this->builder.addPage(width, height);
}

void XmlParser::parseAudioTag(const XmlParserHelper::AttributeMap& attributeMap) {
    auto filename = XmlParserHelper::getAttribMandatory<fs::path>(xoj::xml_attrs::AUDIO_FILENAME_STR, attributeMap);

    this->builder.addAudioAttachment(filename);
}

void XmlParser::parseBackgroundTag(const XmlParserHelper::AttributeMap& attributeMap) {
    auto name = XmlParserHelper::getAttrib<std::string_view>(xoj::xml_attrs::NAME_STR, attributeMap);
    using xoj::xml_attrs::BackgroundType;
    const auto optType = XmlParserHelper::getAttrib<BackgroundType>(xoj::xml_attrs::TYPE_STR, attributeMap);

    if (name) {
        this->builder.setBgName(std::string{*name});
    }
    if (optType) {
        switch (*optType) {
            case BackgroundType::SOLID:
                parseBgSolid(attributeMap);
                break;
            case BackgroundType::PIXMAP:
                parseBgPixmap(attributeMap);
                break;
            case BackgroundType::PDF:
                parseBgPdf(attributeMap);
                break;
            default:
                xoj_assert_message(false, "All background types must be covered");
        }
    } else {
        // It's not possible to assume a default type as other attributes have to be set in fuction of this. Not setting
        // a background will leave the default-constructed one.
        g_warning("XML parser: Attribute \"type\" not found in background tag. Ignoring tag.");
    }
}

void XmlParser::parseBgSolid(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto optStyle = XmlParserHelper::getAttrib<std::string_view>(xoj::xml_attrs::STYLE_STR, attributeMap);
    const auto config =
            XmlParserHelper::getAttribMandatory<std::string_view>(xoj::xml_attrs::CONFIG_STR, attributeMap, "", false);
    PageType bg;
    if (optStyle) {
        bg.format = PageTypeHandler::getPageTypeFormatForString(*optStyle);
    }
    bg.config = config;

    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::white, true);

    this->builder.setBgSolid(bg, color);
}

void XmlParser::parseBgPixmap(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto domain = XmlParserHelper::getAttribMandatory<xoj::xml_attrs::Domain>(
            xoj::xml_attrs::DOMAIN_STR, attributeMap, xoj::xml_attrs::Domain::ABSOLUTE);

    if (domain != xoj::xml_attrs::Domain::CLONE) {
        const auto filename = XmlParserHelper::getAttribMandatory<fs::path>(xoj::xml_attrs::FILENAME_STR, attributeMap);
        this->builder.setBgPixmap(domain == xoj::xml_attrs::Domain::ATTACH, filename);
    } else {
        // In case of a cloned background image, filename contains the page
        // number from which the image is cloned.
        const auto pageNr = XmlParserHelper::getAttribMandatory<size_t>(xoj::xml_attrs::FILENAME_STR, attributeMap);
        this->builder.setBgPixmapCloned(pageNr);
    }
}

void XmlParser::parseBgPdf(const XmlParserHelper::AttributeMap& attributeMap) {
    if (!this->pdfFilenameParsed) {
        auto domain = XmlParserHelper::getAttribMandatory<xoj::xml_attrs::Domain>(
                xoj::xml_attrs::DOMAIN_STR, attributeMap, xoj::xml_attrs::Domain::ABSOLUTE);
        if (domain == xoj::xml_attrs::Domain::CLONE) {
            g_warning(R"(XML parser: Domain "clone" is invalid for PDF backgrounds. Using "absolute" instead)");
            domain = xoj::xml_attrs::Domain::ABSOLUTE;
        }

        const auto filename = XmlParserHelper::getAttribMandatory<fs::path>(xoj::xml_attrs::FILENAME_STR, attributeMap);

        if (!filename.empty()) {
            this->pdfFilenameParsed = true;
            this->builder.loadBgPdf(domain == xoj::xml_attrs::Domain::ATTACH, filename);
        } else {
            g_warning("XML parser: PDF background filename is empty");
        }
    }

    const auto pageno =
            XmlParserHelper::getAttribMandatory<size_t>(xoj::xml_attrs::PAGE_NUMBER_STR, attributeMap, 1) - 1;

    this->builder.setBgPdf(pageno);
}

void XmlParser::parseLayerTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto name = XmlParserHelper::getAttrib<std::string_view>(xoj::xml_attrs::NAME_STR, attributeMap);

    this->builder.addLayer(name);
}

void XmlParser::parseTimestampTag(const XmlParserHelper::AttributeMap& attributeMap) {
    // Compatibility: timestamps for audio elements are stored in the attributes since 6b43baf

    if (!this->tempFilename.empty()) {
        g_warning("XML parser: Discarding unused audio timestamp element. Filename: %s",
                  char_cast(this->tempFilename.u8string().c_str()));
    }

    this->tempFilename =
            XmlParserHelper::getAttribMandatory<fs::path>(xoj::xml_attrs::AUDIO_FILENAME_STR, attributeMap);
    this->tempTimestamp = XmlParserHelper::getAttribMandatory<size_t>(xoj::xml_attrs::TIMESTAMP_STR, attributeMap);
}

void XmlParser::parseStrokeTag(const XmlParserHelper::AttributeMap& attributeMap) {
    // tool
    const auto tool =
            XmlParserHelper::getAttribMandatory<StrokeTool>(xoj::xml_attrs::TOOL_STR, attributeMap, StrokeTool::PEN);
    // color
    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::black);

    // width
    auto widthSV = XmlParserHelper::getAttribMandatory<std::string_view>(xoj::xml_attrs::WIDTH_STR, attributeMap, "1");
    auto it = widthSV.begin();
    auto end = widthSV.end();
    double width{};
    parseDouble(it, end, width);

    // pressures
    auto pressureSV = XmlParserHelper::getAttrib<std::string_view>(xoj::xml_attrs::PRESSURES_STR, attributeMap);
    if (pressureSV) {
        // MrWriter writes pressures in a separate field
        it = pressureSV->begin();
        end = pressureSV->end();
    }
    // Xournal and Xournal++ use the width field. `it` pointer is already in place.

    double pressure{};
    while (parseDouble(it, end, pressure)) {
        this->pressureBuffer.emplace_back(pressure);
    }

    // fill
    const auto fill = XmlParserHelper::getAttribMandatory<int>(xoj::xml_attrs::FILL_STR, attributeMap, -1, false);

    // cap stype
    const auto capStyle = XmlParserHelper::getAttribMandatory<StrokeCapStyle>(
            xoj::xml_attrs::CAPSTYLE_STR, attributeMap, StrokeCapStyle::ROUND, false);

    // line style
    const auto lineStyle =
            XmlParserHelper::getAttribMandatory<LineStyle>(xoj::xml_attrs::STYLE_STR, attributeMap, {}, false);

    // audio filename and timestamp
    const auto optFilename = XmlParserHelper::getAttrib<fs::path>(xoj::xml_attrs::AUDIO_FILENAME_STR, attributeMap);
    if (optFilename && !optFilename->empty()) {
        if (!this->tempFilename.empty()) {
            g_warning("XML parser: Discarding audio timestamp element, because stroke tag contains \"fn\" attribute");
        }
        this->tempFilename = *optFilename;
        this->tempTimestamp =
                XmlParserHelper::getAttribMandatory<size_t>(xoj::xml_attrs::TIMESTAMP_STR, attributeMap, 0UL);
    }

    // forward data to builder
    this->builder.addStroke(tool, color, width, fill, capStyle, lineStyle, std::move(this->tempFilename),
                            this->tempTimestamp);

    // Reset timestamp, filename was already moved from
    this->tempTimestamp = 0;
}

void XmlParser::parseStrokeText(std::string_view text) {
    std::vector<Point> pointVector;
    pointVector.reserve(this->pressureBuffer.size() + 1);

    auto it = text.begin();
    const auto end = text.end();
    double x{}, y{};
    while (parseDouble(it, end, x)) {
        if (!parseDouble(it, end, y)) {
            g_warning("XML parser: Found stroke that contains an odd number of valid coordinates. "
                      "Discarding the last value");
            break;
        }
        pointVector.emplace_back(x, y);
    }

    this->builder.setStrokePoints(std::move(pointVector), std::move(this->pressureBuffer));
}

void XmlParser::parseTextTag(const XmlParserHelper::AttributeMap& attributeMap) {
    auto font = XmlParserHelper::getAttribMandatory<std::string_view>(xoj::xml_attrs::FONT_STR, attributeMap, "Sans");
    const auto size = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::SIZE_STR, attributeMap, 12);
    const auto x = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::X_COORD_STR, attributeMap);
    const auto y = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::Y_COORD_STR, attributeMap);
    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::black);

    // audio filename and timestamp
    const auto optFilename = XmlParserHelper::getAttrib<fs::path>(xoj::xml_attrs::AUDIO_FILENAME_STR, attributeMap);
    if (optFilename && !optFilename->empty()) {
        if (!this->tempFilename.empty()) {
            g_warning("XML parser: Discarding audio timestamp element, because text tag contains \"fn\" attribute");
        }
        this->tempFilename = *optFilename;
        this->tempTimestamp =
                XmlParserHelper::getAttribMandatory<size_t>(xoj::xml_attrs::TIMESTAMP_STR, attributeMap, 0UL);
    }

    this->builder.addText(std::string{font}, size, x, y, color, std::move(tempFilename), tempTimestamp);

    this->tempTimestamp = 0;
}

void XmlParser::parseTextText(std::string_view text) { this->builder.setTextContents(std::string{text}); }

void XmlParser::parseImageTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto left = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::LEFT_POS_STR, attributeMap);
    const auto top = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::TOP_POS_STR, attributeMap);
    const auto right = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::RIGHT_POS_STR, attributeMap);
    const auto bottom = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::BOTTOM_POS_STR, attributeMap);

    this->builder.addImage(left, top, right, bottom);
}

void XmlParser::parseImageText(std::string_view text) {
    if (!isAllWhitespace(text)) {
        std::string imageData = XmlParserHelper::decodeBase64(text);
        this->builder.setImageData(std::move(imageData));
    }
}

void XmlParser::parseTexImageTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto left = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::LEFT_POS_STR, attributeMap);
    const auto top = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::TOP_POS_STR, attributeMap);
    const auto right = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::RIGHT_POS_STR, attributeMap);
    const auto bottom = XmlParserHelper::getAttribMandatory<double>(xoj::xml_attrs::BOTTOM_POS_STR, attributeMap);

    auto text = XmlParserHelper::getAttribMandatory<std::string_view>(xoj::xml_attrs::TEXT_STR, attributeMap);

    // Attribute "texlength" found in earlier parsers was a workaround from 098a67b to bdd0ec2

    this->builder.addTexImage(left, top, right, bottom, std::string{text});
}

void XmlParser::parseTexImageText(std::string_view text) {
    if (!isAllWhitespace(text)) {
        std::string imageData = XmlParserHelper::decodeBase64(text);
        this->builder.setTexImageData(std::move(imageData));
    }
}

void XmlParser::parseAttachmentTag(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto path = XmlParserHelper::getAttribMandatory<fs::path>(xoj::xml_attrs::PATH_STR, attributeMap);

    xoj_assert(this->lastValidTag);
    switch (*this->lastValidTag) {
        case TagType::IMAGE:
            this->builder.setImageAttachment(path);
            break;
        case TagType::TEXIMAGE:
            this->builder.setTexImageAttachment(path);
            break;
        default:
            g_warning("Ignoring attachment tag under " SV_FMT, U8SV_ARG(TAG_NAMES[*this->lastValidTag]));
            break;
    }
}

auto XmlParser::getTagType(c_string_utf8_view name) const -> TagType {
    using namespace std::literals;

    if (this->hierarchy.empty()) {
        // Parser is at top level
        if (TAG_NAMES[TagType::XOURNAL] == name)
            return TagType::XOURNAL;
        if (TAG_NAMES[TagType::MRWRITER] == name)
            return TagType::MRWRITER;
    } else if (!this->lastValidTag) {
        // Hierarchy contains only unknown tags. Allow parsing of document contents.
        if (TAG_NAMES[TagType::TITLE] == name)
            return TagType::TITLE;
        if (TAG_NAMES[TagType::PREVIEW] == name)
            return TagType::PREVIEW;
        if (TAG_NAMES[TagType::PAGE] == name)
            return TagType::PAGE;
        if (TAG_NAMES[TagType::AUDIO] == name)
            return TagType::AUDIO;
    } else {
        switch (*this->lastValidTag) {
            case TagType::XOURNAL:
            case TagType::MRWRITER:
                if (TAG_NAMES[TagType::TITLE] == name)
                    return TagType::TITLE;
                if (TAG_NAMES[TagType::PREVIEW] == name)
                    return TagType::PREVIEW;
                if (TAG_NAMES[TagType::PAGE] == name)
                    return TagType::PAGE;
                if (TAG_NAMES[TagType::AUDIO] == name)
                    return TagType::AUDIO;
                break;
            case TagType::PAGE:
                if (TAG_NAMES[TagType::BACKGROUND] == name)
                    return TagType::BACKGROUND;
                if (TAG_NAMES[TagType::LAYER] == name)
                    return TagType::LAYER;
                break;
            case TagType::LAYER:
                if (TAG_NAMES[TagType::TIMESTAMP] == name)
                    return TagType::TIMESTAMP;
                if (TAG_NAMES[TagType::STROKE] == name)
                    return TagType::STROKE;
                if (TAG_NAMES[TagType::TEXT] == name)
                    return TagType::TEXT;
                if (TAG_NAMES[TagType::IMAGE] == name)
                    return TagType::IMAGE;
                if (TAG_NAMES[TagType::TEXIMAGE] == name)
                    return TagType::TEXIMAGE;
                break;
            case TagType::IMAGE:
            case TagType::TEXIMAGE:
                if (TAG_NAMES[TagType::ATTACHMENT] == name)
                    return TagType::ATTACHMENT;
                break;
            case TagType::UNKNOWN:
                xoj_assert_message(false, "TagType::UNKNOWN is not a valid tag");
            default:
                xoj_assert_message(false, "Illegal tag in hierarchy");
        }
    }

    // Tag name could not be matched with an expected type
    return TagType::UNKNOWN;
}
