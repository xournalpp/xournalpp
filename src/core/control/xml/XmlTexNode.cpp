#include "XmlTexNode.h"

#include "control/xml/XmlBase64Writer.h"  // for XmlBase64Writer
#include "control/xml/XmlNode.h"          // for XmlNode
#include "util/OutputStream.h"            // for OutputStream
#include "util/StringUtils.h"             // for StaticStringView

XmlTexNode::XmlTexNode(StringUtils::StaticStringView tag, std::string&& binaryData):
        XmlNode(tag), binaryData(binaryData) {}

XmlTexNode::~XmlTexNode() = default;

void XmlTexNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    XmlBase64Writer b64(out);
    b64.write(reinterpret_cast<const unsigned char*>(this->binaryData.data()), this->binaryData.size());
    b64.finish();

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
