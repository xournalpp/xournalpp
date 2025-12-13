/*
 * Xournal++
 *
 * Parses the uncompressed XML of .xoj / .xopp documents
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdlib>
#include <optional>
#include <string_view>
#include <vector>

#include <glib.h>

#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/XmlParserHelper.h"
#include "control/xojfile/XmlTags.h"
#include "util/EnumIndexedArray.h"

#include "filesystem.h"


class XmlParser {
public:
    XmlParser(LoadHandler& handler);

    // GMarkup callbacks
    static void parserStartElement(GMarkupParseContext* context, const gchar* elementName, const gchar** attributeNames,
                                   const gchar** attributeValues, gpointer userdata, GError** error);
    static void parserEndElement(GMarkupParseContext* context, const gchar* elementName, gpointer userdata,
                                 GError** error);
    static void parserText(GMarkupParseContext* context, const gchar* text, gsize textLen, gpointer userdata,
                           GError** error);

    static constexpr GMarkupParser interface = {parserStartElement, parserEndElement, parserText, nullptr, nullptr};

private:
    /**
     * UTF-8 Roadmap
     *
     * Most internals of Xournal++ still use or accept std::string as
     * constructors. When that changes, you should directly retrieve
     * xoj::util::utf8_view types or their aliases below from getAttrib<T>()
     * functions instead of converting it after you already obtained a
     * std::string_view or similar, as shown in parseXournalTag().
     */
    using c_string_utf8_view = XmlParserHelper::c_string_utf8_view;
    using string_utf8_view = XmlParserHelper::string_utf8_view;

    void parseUnknownTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseXournalTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseMrWriterTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parsePageTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseAudioTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBackgroundTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBgSolid(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBgPixmap(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBgPdf(const XmlParserHelper::AttributeMap& attributeMap);
    void parseLayerTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseTimestampTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseStrokeTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseStrokeText(std::string_view text);
    void parseTextTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseTextText(std::string_view text);
    void parseImageTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseImageText(std::string_view text);
    void parseTexImageTag(const XmlParserHelper::AttributeMap& attributeMap);
    void parseTexImageText(std::string_view text);
    void parseAttachmentTag(const XmlParserHelper::AttributeMap& attributeMap);

    /**
     * Get the tag type from a given element name
     *
     * This function returns the corresponding tag type if it is expected under
     * the last valid tag type. Otherwise it returns TagType::UNKNOWN, even if
     * the tag is globally known (e.g. if a page is found under a layer).
     */
    xoj::xml_tags::Type getTagType(c_string_utf8_view name) const;

    using StartElementFunc = void (XmlParser::*)(const XmlParserHelper::AttributeMap&);
    using TextFunc = void (XmlParser::*)(std::string_view);
    using EndElementFunc = void (LoadHandler::*)();
    struct ParsingEntry {
        StartElementFunc start;
        TextFunc text;
        EndElementFunc end;
    };

    // Table to dispatch parsing callbacks
    static constexpr EnumIndexedArray<ParsingEntry, xoj::xml_tags::Type> parsingTable{
            EnumIndexedArray<ParsingEntry, xoj::xml_tags::Type>::underlying_array_type{{
                    {&XmlParser::parseUnknownTag, {}, {}},                               // TagType::UNKNOWN
                    {&XmlParser::parseXournalTag, {}, &LoadHandler::finalizeDocument},   // TagType::XOURNAL
                    {&XmlParser::parseMrWriterTag, {}, &LoadHandler::finalizeDocument},  // TagType::MRWRITER
                    {{}, {}, {}},                                                        // TagType::TITLE (ignored)
                    {{}, {}, {}},                                                        // TagType::PREVIEW (ignored)
                    {&XmlParser::parsePageTag, {}, &LoadHandler::finalizePage},          // TagType::PAGE
                    {&XmlParser::parseAudioTag, {}, {}},                                 // TagType::AUDIO
                    {&XmlParser::parseBackgroundTag, {}, {}},                            // TagType::BACKGROUND
                    {&XmlParser::parseLayerTag, {}, &LoadHandler::finalizeLayer},        // TagType::LAYER
                    {&XmlParser::parseTimestampTag, {}, {}},                             // TagType::TIMESTAMP
                    {&XmlParser::parseStrokeTag, &XmlParser::parseStrokeText,
                     &LoadHandler::finalizeStroke},  // TagType::STROKE
                    {&XmlParser::parseTextTag, &XmlParser::parseTextText, &LoadHandler::finalizeText},  // TagType::TEXT
                    {&XmlParser::parseImageTag, &XmlParser::parseImageText,
                     &LoadHandler::finalizeImage},  // TagType::IMAGE
                    {&XmlParser::parseTexImageTag, &XmlParser::parseTexImageText,
                     &LoadHandler::finalizeTexImage},         // TagType::TEXIMAGE
                    {&XmlParser::parseAttachmentTag, {}, {}}  // TagType::ATTACHMENT
            }}};

    // Handler to which the parsed data is forwarded
    LoadHandler& handler;

    // Stack containing the tag types found at every level in the document
    std::vector<xoj::xml_tags::Type> hierarchy;
    // Variable to track the topmost valid tag type in the hierarchy stack
    std::optional<xoj::xml_tags::Type> lastValidTag;

    bool pdfFilenameParsed = false;

    size_t tempTimestamp = 0;
    fs::path tempFilename;

    std::vector<double> pressureBuffer;
};
