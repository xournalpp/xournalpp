#include "XmlTexNode.h"

#include <glib.h>  // for g_base64_encode, g_free, gchar, guchar

#include "control/xml/XmlNode.h"  // for XmlNode
#include "util/OutputStream.h"    // for OutputStream

XmlTexNode::XmlTexNode(const char* tag, std::string&& binaryData): XmlNode(tag), binaryData(binaryData) {}

XmlTexNode::~XmlTexNode() = default;

void XmlTexNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    gchar* base64_str =
            g_base64_encode(reinterpret_cast<const guchar*>(this->binaryData.c_str()), this->binaryData.length());
    out->write(base64_str);
    g_free(base64_str);

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
