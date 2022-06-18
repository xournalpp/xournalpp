#include "XmlTextNode.h"

#include <utility>  // for move

#include "control/xml/XmlAudioNode.h"  // for XmlAudioNode
#include "util/OutputStream.h"         // for OutputStream
#include "util/StringUtils.h"          // for replace_pair, StringUtils

XmlTextNode::XmlTextNode(const char* tag, std::string text): XmlAudioNode(tag), text(std::move(text)) {}

XmlTextNode::XmlTextNode(const char* tag): XmlAudioNode(tag) {}

void XmlTextNode::setText(std::string text) { this->text = std::move(text); }

void XmlTextNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    std::string tmp(this->text);
    StringUtils::replaceAllChars(tmp,
                                 {replace_pair('&', "&amp;"), replace_pair('<', "&lt;"), replace_pair('>', "&gt;")});
    out->write(tmp);

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
