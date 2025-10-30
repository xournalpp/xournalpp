#include "XojExportHandler.h"

#include <memory>  // for unique_ptr, __shared_p...
#include <string>  // for string, allocator, ope...

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeHandler
#include "control/xml/XmlNode.h"               // for XmlNode
#include "control/xml/XmlTextNode.h"           // for XmlTextNode
#include "control/xojfile/XmlAttrs.h"          // for xml_attrs
#include "control/xojfile/XmlTags.h"           // for xml_tags
#include "model/PageType.h"                    // for PageTypeFormat, PageType
#include "model/XojPage.h"                     // for XojPage

#include "config.h"  // for PROJECT_STRING, PROJEC...

class AudioElement;
class Stroke;
class XmlAudioNode;
class XmlPointNode;

XojExportHandler::XojExportHandler() = default;

XojExportHandler::~XojExportHandler() = default;

/**
 * Export the fill attributes
 */
void XojExportHandler::visitStrokeExtended(XmlPointNode* stroke, const Stroke* s) {
    // Fill is not exported in .xoj
    // Line style is also not supported
}

void XojExportHandler::writeHeader() {
    this->root->setAttrib(xoj::xml_attrs::CREATOR_STR, PROJECT_STRING);
    // Keep this version on 2, as this is anyway not read by Xournal
    this->root->setAttrib(xoj::xml_attrs::FILEVERSION_STR, "2");
    this->root->addChild(
            new XmlTextNode(xoj::xml_tags::NAMES[xoj::xml_tags::Type::TITLE],
                            std::string{"Xournal document (Compatibility) - see "} + PROJECT_HOMEPAGE_URL));
}

void XojExportHandler::writeSolidBackground(XmlNode* background, ConstPageRef p) {
    using xoj::xml_attrs::BackgroundType;
    background->setAttrib(xoj::xml_attrs::TYPE_STR, BackgroundType::NAMES[BackgroundType::SOLID]);
    background->setAttrib(xoj::xml_attrs::COLOR_STR, getColorStr(p->getBackgroundColor()));

    PageTypeFormat bgFormat = p->getBackgroundType().format;
    std::string format;

    format = PageTypeHandler::getStringForPageTypeFormat(bgFormat);
    if (bgFormat != PageTypeFormat::Plain && bgFormat != PageTypeFormat::Ruled && bgFormat != PageTypeFormat::Lined &&
        bgFormat != PageTypeFormat::Graph) {
        format = "plain";
    }

    background->setAttrib(xoj::xml_attrs::STYLE_STR, format);
}

void XojExportHandler::writeTimestamp(XmlAudioNode* xmlAudioNode, const AudioElement* audioElement) {
    // Do nothing since timestamp are not supported by Xournal
}

void XojExportHandler::writeBackgroundName(XmlNode* background, ConstPageRef p) {
    // Do nothing since background name is not supported by Xournal
}
