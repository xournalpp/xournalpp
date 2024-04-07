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
#include <functional>
#include <stack>
#include <string>
#include <vector>

#include <libxml/xmlreader.h>

#include "control/xojfile/InputStream.h"
#include "control/xojfile/XmlParserHelper.h"
#include "control/xojfile/XmlTags.h"

#include "config-debug.h"
#include "filesystem.h"

class LoadHandler;


class XmlParser {
public:
    XmlParser(InputStream& input, LoadHandler* handler);
    ~XmlParser();

    /**
     * @brief Parse the XML input and forward data to the handler's appropriate add*,
     * addText* and finalize* functions
     *
     * Loops over all elements at the current depth level and calls processNodeFunction
     * at each node. Returns when the current element is closed or the EOF is reached.
     * If the function returns before EOF is reached, the reader points to a not yet
     * processed closing node.
     *
     * If the first operation does not return a start element node, the function
     * exits immediately.
     *
     * @param processNodeFunction should be able to process any child nodes of
     * the current element. It should call parse() again with an appropriate
     * node processing funcion when expecting grandchildren. Otherwise, it
     * should return xmlTextReaderRead().
     *
     * @return The result of the last read operation.
     */
    int parse(const std::function<int(XmlParser*)>& processNodeFunction = &XmlParser::processRootNode);

private:
    int processRootNode();
    int processDocumentChildNode();
    int processPageChildNode();
    int processLayerChildNode();
    int processAttachment();

    void parseXournalTag();
    void parseMrWriterTag();
    void parsePageTag();
    void parseAudioTag();
    void parseBackgroundTag();
    void parseBgSolid(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBgPixmap(const XmlParserHelper::AttributeMap& attributeMap);
    void parseBgPdf(const XmlParserHelper::AttributeMap& attributeMap);
    void parseLayerTag();
    void parseTimestampTag();
    void parseStrokeTag();
    void parseStrokeText();
    void parseTextTag();
    void parseTextText();
    void parseImageTag();
    void parseImageText();
    void parseTexImageTag();
    void parseTexImageText();
    void parseAttachment();


    XmlParserHelper::AttributeMap getAttributeMap();

    /**
     * Add the current node's tag to the hierarchy stack and return it
     */
    XmlTags::Type openTag();
    /**
     * Remove the specified tag from the hierarchy stack. This function also
     * checks the document integrity together with `openTag()`: each opening
     * tag matches exactly one closing tag of the same name. It may throw an
     * exception if the document structure is not sound.
     */
    void closeTag(XmlTags::Type type);

    XmlTags::Type tagNameToType(const std::string& name) const;
    const char* currentName();
    XmlTags::Type currentTagType();

#ifdef DEBUG_XML_PARSER
    void debugPrintNode();
    void debugPrintAttributes(const XmlParserHelper::AttributeMap& attributes);
#endif


    xmlTextReaderPtr reader;
    LoadHandler* handler;

    std::stack<XmlTags::Type> hierarchy;

    bool pdfFilenameParsed;

    size_t tempTimestamp;
    fs::path tempFilename;

    std::vector<double> pressureBuffer;
};
