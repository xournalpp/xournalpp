#include "control/xojfile/XmlParser.h"

#include <functional>     // for function
#include <stdexcept>      // for runtime_error
#include <string>         // for stod, string
#include <unordered_map>  // for unordered_map
#include <utility>        // for move
#include <vector>         // for vector

#include <glib.h>              // for g_warning
#include <libxml/parser.h>     // for XML_PARSE_RECOVER
#include <libxml/tree.h>       // for XML_ELEMENT_NODE, X...
#include <libxml/xmlerror.h>   // for xmlError, xmlGetLas...
#include <libxml/xmlreader.h>  // for xmlReaderForIO, xml...

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeHandler
#include "control/xojfile/InputStream.h"       // for InputStream
#include "control/xojfile/LoadHandler.h"       // for LoadHandler
#include "control/xojfile/XmlParserHelper.h"   // for getAttrib...
#include "model/PageType.h"                    // for PageType
#include "model/Point.h"                       // for Point
#include "model/Stroke.h"                      // for StrokeTool, StrokeCapStyle
#include "util/Assert.h"                       // for xoj_assert
#include "util/Color.h"                        // for Color
#include "util/i18n.h"                         // for FS, _F
#include "util/safe_casts.h"                   // for as_unsigned

#include "config-debug.h"  // for DEBUG_XML_PARSER
#include "filesystem.h"    // for path

#ifdef DEBUG_XML_PARSER
#include <iostream>  // for cout

#define DEBUG_PARSER(f) f
#else
#define DEBUG_PARSER(f)
#endif


static auto readCallback(void* context, char* buffer, int len) -> int {
    auto* input = reinterpret_cast<InputStream*>(context);
    return input->read(buffer, as_unsigned(len));
}

// called by xmlFreeTextReader
static auto closeCallback(void* context) -> int {
    auto* input = reinterpret_cast<InputStream*>(context);
    input->close();
    return 0;
}


XmlParser::XmlParser(InputStream& input, LoadHandler* handler):
        reader(xmlReaderForIO(readCallback, closeCallback, &input, nullptr, nullptr,
                              XML_PARSE_RECOVER | XML_PARSE_NOBLANKS)),
        handler(handler),
        pdfFilenameParsed(false),
        tempTimestamp(0) {
    if (!reader) {
        const xmlError* error = xmlGetLastError();
        std::string message = FS(_F("Error setting up an XML reader: {1}") % error->message);
        message += "\n" + FS(_F("Error code {1}") % error->code);
        xmlResetLastError();
        throw std::runtime_error(message);
    }
}

XmlParser::~XmlParser() {
    if (this->reader) {
        xmlFreeTextReader(this->reader);
    }
}

auto XmlParser::parse(const std::function<int(XmlParser*)>& processNodeFunction) -> int {
    int res = xmlTextReaderRead(reader);
    int startDepth{};
    if (res == 1) {
        if (xmlTextReaderNodeType(this->reader) != XML_ELEMENT_NODE) {
            // The first node isn't an opening node.
            return res;
        } else {
            startDepth = xmlTextReaderDepth(this->reader);
        }
    }

    while (res == 1) {
        if (xmlTextReaderDepth(this->reader) >= startDepth) {
            DEBUG_PARSER(debugPrintNode());
            // The node processing functions always perform a read operation at
            // the end. Some do because they call parse(), so all must comply.
            res = processNodeFunction(this);
        } else {
            // We reached a node at a lower depth as our start depth.
            return res;
        }
    }

    if (res < 0) {
        const xmlError* error = xmlGetLastError();
        std::string message = FS(_F("Error parsing XML file: {1}") % error->message);
        message += FS(_F("Error code {1}, line {2}") % error->code % error->line);
        xmlResetLastError();
        throw std::runtime_error(message);
    }

    return res;
}


auto XmlParser::processRootNode() -> int {
    const int nodeType = xmlTextReaderNodeType(this->reader);
    switch (nodeType) {
        case XML_ELEMENT_NODE: {
            xoj_assert(this->hierarchy.empty());

            const TagType tagType = openTag();

            switch (tagType) {
                case TagType::XOURNAL:
                    parseXournalTag();
                    break;
                case TagType::MRWRITER:
                    parseMrWriterTag();
                    break;
                default:
                    // Print a warning, but attempt parsing the document anyway
                    g_warning("XML parser: Unexpected root tag: \"%s\"", currentName());
                    break;
            }

            // The root tag should not be empty
            if (xmlTextReaderIsEmptyElement(this->reader)) {
                throw std::runtime_error(_("Error parsing XML file: the document root tag is empty"));
            }

            return parse(&XmlParser::processDocumentChildNode);
        }
        case XML_ELEMENT_DECL: {
            // Parsing is done: we have arrived at the closing node
            this->handler->finalizeDocument();
            closeTag(currentTagType());
            return xmlTextReaderRead(this->reader);
        }
        default:
            g_warning("XML parser: Ignoring unexpected node type %d at document root", nodeType);
            return xmlTextReaderRead(this->reader);
    }
}

auto XmlParser::processDocumentChildNode() -> int {
    xoj_assert(!this->hierarchy.empty());

    const int nodeType = xmlTextReaderNodeType(this->reader);
    switch (nodeType) {
        case XML_ELEMENT_NODE: {
            xoj_assert(this->hierarchy.top() == TagType::XOURNAL || this->hierarchy.top() == TagType::MRWRITER ||
                       this->hierarchy.top() == TagType::UNKNOWN);

            const TagType tagType = openTag();

            switch (tagType) {
                case TagType::TITLE:
                case TagType::PREVIEW:
                    // Ignore these tags, we don't need them.
                    break;
                case TagType::PAGE:
                    parsePageTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        g_warning("XML parser: Found empty page");
                        this->handler->finalizePage();
                        break;
                    }
                    return parse(&XmlParser::processPageChildNode);
                case TagType::AUDIO:
                    parseAudioTag();
                    break;
                default:
                    g_warning("XML parser: Ignoring unexpected tag in document: \"%s\"", currentName());
                    break;
            }

            return xmlTextReaderRead(this->reader);
        }
        case XML_TEXT_NODE: {
            // ignore text from tags above (title or preview), print a warning otherwise
            if (this->hierarchy.top() != TagType::TITLE && this->hierarchy.top() != TagType::PREVIEW) {
                g_warning("XML parser: Ignoring unexpected text under tag \"%s\"",
                          tagTypeToName(this->hierarchy.top()).c_str());
            }
            return xmlTextReaderRead(this->reader);
        }
        case XML_ELEMENT_DECL: {
            if (this->hierarchy.top() == TagType::PAGE) {
                this->handler->finalizePage();
            }
            closeTag(currentTagType());
            return xmlTextReaderRead(this->reader);
        }
        default:
            g_warning("XML parser: Ignoring unexpected node type %d in document", nodeType);
            return xmlTextReaderRead(this->reader);
    }
}

auto XmlParser::processPageChildNode() -> int {
    xoj_assert(!this->hierarchy.empty());

    const int nodeType = xmlTextReaderNodeType(this->reader);
    switch (nodeType) {
        case XML_ELEMENT_NODE: {
            xoj_assert(this->hierarchy.top() == TagType::PAGE || this->hierarchy.top() == TagType::UNKNOWN);

            const TagType tagType = openTag();

            switch (tagType) {
                case TagType::BACKGROUND:
                    parseBackgroundTag();
                    break;
                case TagType::LAYER:
                    parseLayerTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        // Don't warn: it's normal to have an empty layer in an empty page
                        this->handler->finalizeLayer();
                        break;
                    }
                    return parse(&XmlParser::processLayerChildNode);
                default:
                    g_warning("XML parser: Ignoring unexpected tag in page: \"%s\"", currentName());
                    break;
            }
            return xmlTextReaderRead(this->reader);
        }
        case XML_ELEMENT_DECL:
            if (this->hierarchy.top() == TagType::LAYER) {
                this->handler->finalizeLayer();
            }
            closeTag(currentTagType());
            return xmlTextReaderRead(this->reader);
        default:
            g_warning("XML parser: Ignoring unexpected node type %d in page", nodeType);
            return xmlTextReaderRead(this->reader);
    }
}

auto XmlParser::processLayerChildNode() -> int {
    xoj_assert(!this->hierarchy.empty());

    const int nodeType = xmlTextReaderNodeType(this->reader);
    switch (nodeType) {
        case XML_ELEMENT_NODE: {
            xoj_assert(this->hierarchy.top() == TagType::LAYER || this->hierarchy.top() == TagType::UNKNOWN);

            const TagType tagType = openTag();

            switch (tagType) {
                case TagType::TIMESTAMP:
                    parseTimestampTag();
                    break;
                case TagType::STROKE:
                    parseStrokeTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        g_warning("XML parser: Found empty stroke");
                        this->handler->finalizeStroke();
                    }
                    break;
                case TagType::TEXT:
                    parseTextTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        g_warning("XML parser: Found empty text");
                        this->handler->finalizeText();
                    }
                    break;
                case TagType::IMAGE:
                    parseImageTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        g_warning("XML parser: Found empty image");
                        this->handler->finalizeImage();
                    }
                    // An image may have an attachment. If it doesn't, parse()
                    // will return right away
                    return parse(&XmlParser::processAttachment);
                case TagType::TEXIMAGE:
                    parseTexImageTag();
                    if (xmlTextReaderIsEmptyElement(this->reader)) {
                        g_warning("XML parser: Found empty TEX image");
                        this->handler->finalizeTexImage();
                    }
                    // An image may have an attachment. If it doesn't, parse()
                    // will return right away
                    return parse(&XmlParser::processAttachment);
                default:
                    g_warning("XML parser: Ignoring unexpected tag in layer: \"%s\"", currentName());
                    break;
            }
            return xmlTextReaderRead(this->reader);
        }
        case XML_TEXT_NODE: {
            switch (this->hierarchy.top()) {
                case TagType::STROKE:
                    parseStrokeText();
                    break;
                case TagType::TEXT:
                    parseTextText();
                    break;
                case TagType::IMAGE:
                    parseImageText();
                    break;
                case TagType::TEXIMAGE:
                    parseTexImageText();
                    break;
                default:
                    g_warning("XML parser: Ignoring unexpected text under tag \"%s\"",
                              tagTypeToName(this->hierarchy.top()).c_str());
                    break;
            }
            return xmlTextReaderRead(this->reader);
        }
        case XML_ELEMENT_DECL: {
            switch (this->hierarchy.top()) {
                case TagType::STROKE:
                    this->handler->finalizeStroke();
                    break;
                case TagType::TEXT:
                    this->handler->finalizeText();
                    break;
                case TagType::IMAGE:
                    this->handler->finalizeImage();
                    break;
                case TagType::TEXIMAGE:
                    this->handler->finalizeTexImage();
                    break;
                default:
                    break;
            }
            closeTag(currentTagType());
            return xmlTextReaderRead(this->reader);
        }
        default:
            g_warning("XML parser: Ignoring unexpected node type %d in layer", nodeType);
            return xmlTextReaderRead(this->reader);
    }
}

auto XmlParser::processAttachment() -> int {
    xoj_assert(!this->hierarchy.empty());

    const int nodeType = xmlTextReaderNodeType(this->reader);
    switch (nodeType) {
        case XML_ELEMENT_NODE: {
            xoj_assert(this->hierarchy.top() == TagType::IMAGE || this->hierarchy.top() == TagType::TEXIMAGE ||
                       this->hierarchy.top() == TagType::UNKNOWN);

            const TagType tagType = openTag();

            switch (tagType) {
                case TagType::ATTACHMENT:
                    parseAttachment();
                    break;
                default:
                    g_warning("XML parser: Ignoring unexpected tag in image or TEX image: \"%s\"", currentName());
                    break;
            }
            return xmlTextReaderRead(this->reader);
        }
        case XML_ELEMENT_DECL:
            closeTag(currentTagType());
            return xmlTextReaderRead(this->reader);
        default:
            g_warning("XML parser: Ignoring unexpected node type %d in image or TEX image", nodeType);
            return xmlTextReaderRead(this->reader);
    }
}


void XmlParser::parseXournalTag() {
    const auto attributeMap = getAttributeMap();

    std::string creator;
    const auto optCreator = XmlParserHelper::getAttrib<std::string>("creator", attributeMap);
    if (optCreator) {
        creator = *optCreator;
    } else {
        // Compatibility: the creator attribute exists since 7017b71. Before that, only a version string was written
        const auto optVersion = XmlParserHelper::getAttrib<std::string>("version", attributeMap);
        if (optVersion) {
            creator = "Xournal " + *optVersion;
        } else {
            creator = "Unknown";
        }
    }

    const auto fileversion = XmlParserHelper::getAttribMandatory<int>("fileversion", attributeMap, 1);

    this->handler->addXournal(creator, fileversion);
}

void XmlParser::parseMrWriterTag() {
    const auto attributeMap = getAttributeMap();

    std::string creator;
    auto optVersion = XmlParserHelper::getAttrib<std::string>("verison", attributeMap);
    if (optVersion) {
        creator = "MrWriter " + *optVersion;
    } else {
        creator = "Unknown";
    }

    this->handler->addMrWriter(creator);
}

void XmlParser::parsePageTag() {
    const auto attributeMap = getAttributeMap();

    const auto width = XmlParserHelper::getAttribMandatory<double>("width", attributeMap);
    const auto height = XmlParserHelper::getAttribMandatory<double>("height", attributeMap);

    this->handler->addPage(width, height);
}

void XmlParser::parseAudioTag() {
    const auto attributeMap = getAttributeMap();

    const auto filename = XmlParserHelper::getAttribMandatory<fs::path>("fn", attributeMap);

    this->handler->addAudioAttachment(filename);
}

void XmlParser::parseBackgroundTag() {
    const auto attributeMap = getAttributeMap();

    const auto name = XmlParserHelper::getAttrib<std::string>("name", attributeMap);
    const auto optType = XmlParserHelper::getAttrib<std::string>("type", attributeMap);

    this->handler->addBackground(name);
    if (optType) {
        if (*optType == "solid") {
            parseBgSolid(attributeMap);
        } else if (*optType == "pixmap") {
            parseBgPixmap(attributeMap);
        } else if (*optType == "pdf") {
            parseBgPdf(attributeMap);
        } else {
            g_warning("XML parser: Ignoring unknown background type \"%s\"", optType->c_str());
        }
    } else {
        // It's not possible to assume a default type as other attributes have to be set in fuction of this. Not setting
        // a background will leave the default-constructed one.
        g_warning("XML parser: Attribute \"type\" not found in background tag. Ignoring tag.");
    }
}

void XmlParser::parseBgSolid(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto optStyle = XmlParserHelper::getAttrib<std::string>("style", attributeMap);
    const auto config = XmlParserHelper::getAttribMandatory<std::string>("config", attributeMap, "", false);
    PageType bg;
    if (optStyle) {
        bg.format = PageTypeHandler::getPageTypeFormatForString(*optStyle);
    }
    bg.config = config;

    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::white, true);

    this->handler->setBgSolid(bg, color);
}

void XmlParser::parseBgPixmap(const XmlParserHelper::AttributeMap& attributeMap) {
    const auto domain = XmlParserHelper::getAttribMandatory<XmlParserHelper::Domain>("domain", attributeMap,
                                                                                     XmlParserHelper::Domain::ABOSLUTE);

    if (domain != XmlParserHelper::Domain::CLONE) {
        const fs::path filename = XmlParserHelper::getAttribMandatory<std::string>("filename", attributeMap);
        this->handler->setBgPixmap(domain == XmlParserHelper::Domain::ATTACH, filename);
    } else {
        // In case of a cloned background image, filename contains the page
        // number from which the image is cloned.
        const auto pageNr = XmlParserHelper::getAttribMandatory<size_t>("filename", attributeMap);
        this->handler->setBgPixmapCloned(pageNr);
    }
}

void XmlParser::parseBgPdf(const XmlParserHelper::AttributeMap& attributeMap) {
    if (!this->pdfFilenameParsed) {
        auto domain = XmlParserHelper::getAttribMandatory<XmlParserHelper::Domain>("domain", attributeMap,
                                                                                   XmlParserHelper::Domain::ABOSLUTE);
        if (domain == XmlParserHelper::Domain::CLONE) {
            g_warning("XML parser: Domain \"clone\" is invalid for PDF backgrounds. Using \"absolute\" instead");
            domain = XmlParserHelper::Domain::ABOSLUTE;
        }

        const fs::path filename = XmlParserHelper::getAttribMandatory<std::string>("filename", attributeMap);

        if (!filename.empty()) {
            this->pdfFilenameParsed = true;
            this->handler->loadBgPdf(domain == XmlParserHelper::Domain::ATTACH, filename);
        } else {
            g_warning("XML parser: PDF background filename is empty");
        }
    }

    const auto pageno = XmlParserHelper::getAttribMandatory<size_t>("pageno", attributeMap, 1) - 1;

    this->handler->setBgPdf(pageno);
}

void XmlParser::parseLayerTag() {
    const auto attributeMap = getAttributeMap();

    const auto name = XmlParserHelper::getAttrib<std::string>("name", attributeMap);

    this->handler->addLayer(name);
}

void XmlParser::parseTimestampTag() {
    // Compatibility: timestamps for audio elements are stored in the attributes since 6b43baf

    const auto attributeMap = getAttributeMap();

    if (!this->tempFilename.empty()) {
        g_warning("XML parser: Discarding unused audio timestamp element. Filename: %s",
                  this->tempFilename.u8string().c_str());
    }

    this->tempFilename = XmlParserHelper::getAttribMandatory<std::string>("fn", attributeMap);
    this->tempTimestamp = XmlParserHelper::getAttribMandatory<size_t>("ts", attributeMap);
}

void XmlParser::parseStrokeTag() {
    const auto attributeMap = getAttributeMap();

    // tool
    const auto tool = XmlParserHelper::getAttribMandatory<StrokeTool>("tool", attributeMap, StrokeTool::PEN);
    // color
    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::black);

    // width
    auto widthStr = XmlParserHelper::getAttribMandatory<std::string>("width", attributeMap, "1");
    // Use g_ascii_strtod instead of streams beacuse it is about twice as fast
    const char* itPtr = widthStr.c_str();
    char* endPtr = nullptr;
    const double width = g_ascii_strtod(itPtr, &endPtr);

    // pressures
    auto pressureStr = XmlParserHelper::getAttrib<std::string>("pressures", attributeMap);
    if (pressureStr) {
        // MrWriter writes pressures in a separate field
        itPtr = pressureStr->c_str();
    } else {
        // Xournal and Xournal++ use the width field
        itPtr = endPtr;
    }
    while (*itPtr != 0) {
        const double pressure = g_ascii_strtod(itPtr, &endPtr);
        if (endPtr == itPtr) {
            // Parsing failed
            g_warning("XML parser: A pressure point could not be parsed as double. Remaining points: \"%s\"", itPtr);
            break;
        }
        this->pressureBuffer.emplace_back(pressure);
        itPtr = endPtr;
    }

    // fill
    const auto fill = XmlParserHelper::getAttribMandatory<int>("fill", attributeMap, -1, false);

    // cap stype
    const auto capStyle =
            XmlParserHelper::getAttribMandatory<StrokeCapStyle>("capStyle", attributeMap, StrokeCapStyle::ROUND, false);

    // line style
    const auto lineStyle = XmlParserHelper::getAttrib<LineStyle>("style", attributeMap);

    // audio filename and timestamp
    const auto optFilename = XmlParserHelper::getAttrib<std::string>("fn", attributeMap);
    if (optFilename && !optFilename->empty()) {
        if (!this->tempFilename.empty()) {
            g_warning("XML parser: Discarding audio timestamp element, because stroke tag contains \"fn\" attribute");
        }
        this->tempFilename = *optFilename;
        this->tempTimestamp = XmlParserHelper::getAttribMandatory("ts", attributeMap, 0UL);
    }

    // forward data to handler
    this->handler->addStroke(tool, color, width, fill, capStyle, lineStyle, this->tempFilename, this->tempTimestamp);

    // Reset saved audio attributes
    this->tempFilename.clear();
    this->tempTimestamp = 0;
}

void XmlParser::parseStrokeText() {
    std::vector<Point> pointVector;
    pointVector.reserve(this->pressureBuffer.size());

    // Use g_ascii_strtod instead of streams beacuse it is about twice as fast
    const char* itPtr = reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader));
    char* endPtr = nullptr;
    while (*itPtr != 0) {
        const double x = g_ascii_strtod(itPtr, &endPtr);
        itPtr = endPtr;
        // Note: should the first call to g_ascii_strtod have failed, the second one will be given the same input
        //       and fail in the same way. We only need to check for an error once.
        const double y = g_ascii_strtod(itPtr, &endPtr);
        if (endPtr == itPtr) {
            // Parsing failed
            g_warning("XML parser: A stroke coordinate could not be parsed as double. Remaining data: \"%s\"", itPtr);
            break;
        }
        pointVector.emplace_back(x, y);
        itPtr = endPtr;
    }

    this->handler->setStrokePoints(std::move(pointVector), std::move(this->pressureBuffer));
}

void XmlParser::parseTextTag() {
    const auto attributeMap = getAttributeMap();

    const auto font = XmlParserHelper::getAttribMandatory<std::string>("font", attributeMap, "Sans");
    const auto size = XmlParserHelper::getAttribMandatory<double>("size", attributeMap, 12);
    const auto x = XmlParserHelper::getAttribMandatory<double>("x", attributeMap);
    const auto y = XmlParserHelper::getAttribMandatory<double>("y", attributeMap);
    const auto color = XmlParserHelper::getAttribColorMandatory(attributeMap, Colors::black);

    // audio filename and timestamp
    const auto optFilename = XmlParserHelper::getAttrib<std::string>("fn", attributeMap);
    if (optFilename && !optFilename->empty()) {
        if (!this->tempFilename.empty()) {
            g_warning("XML parser: Discarding audio timestamp element, because text tag contains \"fn\" attribute");
        }
        this->tempFilename = *optFilename;
        this->tempTimestamp = XmlParserHelper::getAttribMandatory("ts", attributeMap, 0UL);
    }

    this->handler->addText(font, size, x, y, color, tempFilename, tempTimestamp);

    this->tempFilename.clear();
    this->tempTimestamp = 0;
}

void XmlParser::parseTextText() {
    const auto text = std::string(reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader)));
    this->handler->setTextContents(text);
}

void XmlParser::parseImageTag() {
    const auto attributeMap = getAttributeMap();

    const auto left = XmlParserHelper::getAttribMandatory<double>("left", attributeMap);
    const auto top = XmlParserHelper::getAttribMandatory<double>("top", attributeMap);
    const auto right = XmlParserHelper::getAttribMandatory<double>("right", attributeMap);
    const auto bottom = XmlParserHelper::getAttribMandatory<double>("bottom", attributeMap);

    this->handler->addImage(left, top, right, bottom);
}

void XmlParser::parseImageText() {
    std::string imageData =
            XmlParserHelper::decodeBase64(reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader)));
    this->handler->setImageData(std::move(imageData));
}

void XmlParser::parseTexImageTag() {
    const auto attributeMap = getAttributeMap();

    const auto left = XmlParserHelper::getAttribMandatory<double>("left", attributeMap);
    const auto top = XmlParserHelper::getAttribMandatory<double>("top", attributeMap);
    const auto right = XmlParserHelper::getAttribMandatory<double>("right", attributeMap);
    const auto bottom = XmlParserHelper::getAttribMandatory<double>("bottom", attributeMap);

    auto text = XmlParserHelper::getAttribMandatory<std::string>("text", attributeMap);

    // Attribute "texlength" found in eralier parsers was a workaround from 098a67b to bdd0ec2

    this->handler->addTexImage(left, top, right, bottom, text);
}

void XmlParser::parseTexImageText() {
    std::string imageData =
            XmlParserHelper::decodeBase64(reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader)));
    this->handler->setTexImageData(std::move(imageData));
}

void XmlParser::parseAttachment() {
    const auto attributeMap = getAttributeMap();

    const auto path = XmlParserHelper::getAttribMandatory<fs::path>("path", attributeMap);

    switch (this->hierarchy.top()) {
        case TagType::IMAGE:
            this->handler->setImageAttachment(path);
            break;
        case TagType::TEXIMAGE:
            this->handler->setTexImageAttachment(path);
            break;
        default:
            break;
    }
}


auto XmlParser::getAttributeMap() -> XmlParserHelper::AttributeMap {
    xoj_assert(xmlTextReaderNodeType(this->reader) == XML_ELEMENT_NODE);

    XmlParserHelper::AttributeMap attributeMap;
    while (xmlTextReaderMoveToNextAttribute(this->reader)) {
        attributeMap[currentName()] = reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader));
    }

    DEBUG_PARSER(debugPrintAttributes(attributeMap));

    return attributeMap;
}


auto XmlParser::openTag() -> TagType {
    const TagType type = currentTagType();
    // Add a level to the hierarchy only if the element isn't "empty" (which
    // means there is no closing element)
    if (!xmlTextReaderIsEmptyElement(this->reader)) {
        this->hierarchy.push(type);
    }
    return type;
}

void XmlParser::closeTag(TagType type) {
    // Check that the document structure is not messed up
    if (this->hierarchy.empty()) {
        throw std::runtime_error(
                FS(_F("Error parsing XML file: found closing tag \"{1}\" at document root") % tagTypeToName(type)));
    }
    if (this->hierarchy.top() != type) {
        throw std::runtime_error(
                FS(_F("Error parsing XML file: closing tag \"{1}\" does not correspond to last open element \"{2}\"") %
                   tagTypeToName(type) % tagTypeToName(this->hierarchy.top())));
    }

    // Go up one level in the hierarchy
    this->hierarchy.pop();
}

auto XmlParser::tagTypeToName(TagType type) const -> const std::string& {
    static const std::unordered_map<TagType, std::string> nameMap = {
            {TagType::UNKNOWN, "[unknown]"},    {TagType::XOURNAL, "xournal"},
            {TagType::MRWRITER, "MrWriter"},    {TagType::TITLE, "title"},
            {TagType::PREVIEW, "preview"},      {TagType::PAGE, "page"},
            {TagType::AUDIO, "audio"},          {TagType::BACKGROUND, "background"},
            {TagType::LAYER, "layer"},          {TagType::TIMESTAMP, "timestamp"},
            {TagType::STROKE, "stroke"},        {TagType::TEXT, "text"},
            {TagType::IMAGE, "image"},          {TagType::TEXIMAGE, "teximage"},
            {TagType::ATTACHMENT, "attachment"}};

    return nameMap.at(type);
}

auto XmlParser::tagNameToType(const std::string& name) const -> TagType {
    static const std::unordered_map<std::string, TagType> typeMap = {{"xournal", TagType::XOURNAL},
                                                                     {"MrWriter", TagType::MRWRITER},
                                                                     {"title", TagType::TITLE},
                                                                     {"preview", TagType::PREVIEW},
                                                                     {"page", TagType::PAGE},
                                                                     {"audio", TagType::AUDIO},
                                                                     {"background", TagType::BACKGROUND},
                                                                     {"layer", TagType::LAYER},
                                                                     {"timestamp", TagType::TIMESTAMP},
                                                                     {"stroke", TagType::STROKE},
                                                                     {"text", TagType::TEXT},
                                                                     {"image", TagType::IMAGE},
                                                                     {"teximage", TagType::TEXIMAGE},
                                                                     {"attachment", TagType::ATTACHMENT}};

    auto it = typeMap.find(name);
    if (it == typeMap.end()) {
        g_warning("XML parser: unknown tag name \"%s\"", name.c_str());
        return TagType::UNKNOWN;
    } else {
        return it->second;
    }
}
auto XmlParser::currentName() -> const char* {
    return reinterpret_cast<const char*>(xmlTextReaderConstName(this->reader));
}
auto XmlParser::currentTagType() -> TagType { return tagNameToType(currentName()); }

#ifdef DEBUG_XML_PARSER
void XmlParser::debugPrintNode() {
    const char *name = nullptr, *value = nullptr;

    name = currentName();
    if (name == nullptr) {
        name = "--";
    }

    std::cout << std::dec << std::boolalpha << "Depth: " << xmlTextReaderDepth(this->reader)
              << "  Type: " << xmlTextReaderNodeType(this->reader) << "  Name: " << name
              << "  Empty: " << xmlTextReaderIsEmptyElement(this->reader);

    value = reinterpret_cast<const char*>(xmlTextReaderConstValue(this->reader));
    if (value == nullptr) {
        std::cout << '\n';
    } else {
        std::cout << "  Value: \"" << value << "\"\n";
    }
}

void XmlParser::debugPrintAttributes(const XmlParserHelper::AttributeMap& attributes) {
    if (!attributes.empty()) {
        std::cout << "Attributes:";
        for (const auto& [key, value]: attributes) {
            std::cout << " [" << key << "] = " << value << ";";
        }
        std::cout << '\n';
    }
}
#endif
