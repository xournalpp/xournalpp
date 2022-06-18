#include "XmlNode.h"

#include <utility>  // for move

#include <glib.h>  // for g_free

#include "control/jobs/ProgressListener.h"  // for ProgressListener
#include "control/xml/Attribute.h"          // for XMLAttribute
#include "util/OutputStream.h"              // for OutputStream

#include "DoubleArrayAttribute.h"  // for DoubleArrayAttribute
#include "DoubleAttribute.h"       // for DoubleAttribute
#include "IntAttribute.h"          // for IntAttribute
#include "SizeTAttribute.h"        // for SizeTAttribute
#include "TextAttribute.h"         // for TextAttribute


XmlNode::XmlNode(const char* tag): tag(tag) {}

void XmlNode::setAttrib(const char* attrib, const char* value) {
    if (value == nullptr) {
        value = "";
    }
    putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, std::string value) {
    putAttrib(new TextAttribute(attrib, std::move(value)));
}

void XmlNode::setAttrib(const char* attrib, double value) { putAttrib(new DoubleAttribute(attrib, value)); }

void XmlNode::setAttrib(const char* attrib, int value) { putAttrib(new IntAttribute(attrib, value)); }

void XmlNode::setAttrib(const char* attrib, size_t value) { putAttrib(new SizeTAttribute(attrib, value)); }

/**
 * The double array is now owned by XmlNode and automatically deleted!
 */
void XmlNode::setAttrib(const char* attrib, double* value, int count) {
    putAttrib(new DoubleArrayAttribute(attrib, std::vector<double>{value, value + count}));
    g_free(value);
}

void XmlNode::writeOut(OutputStream* out, ProgressListener* listener) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    if (children.empty()) {
        out->write("/>\n");
    } else {
        out->write(">\n");

        if (listener) {
            listener->setMaximumState(static_cast<int>(children.size()));
        }

        int i = 1;

        for (auto& node: children) {
            node->writeOut(out);
            if (listener) {
                listener->setCurrentState(i);
            }
            i++;
        }

        out->write("</");
        out->write(tag);
        out->write(">\n");
    }
}

void XmlNode::addChild(XmlNode* node) { children.emplace_back(node); }

void XmlNode::putAttrib(XMLAttribute* a) {
    for (auto& attrib: attributes) {
        if (attrib->getName() == a->getName()) {
            attrib.reset(a);
            return;
        }
    }

    attributes.emplace_back(a);
}

void XmlNode::writeAttributes(OutputStream* out) {
    for (auto& attrib: attributes) {
        out->write(" ");
        out->write(attrib->getName());
        out->write("=\"");
        attrib->writeOut(out);
        out->write("\"");
    }
}
