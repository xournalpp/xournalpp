#include "XmlTextNode.h"

#include "StringUtils.h"

XmlTextNode::XmlTextNode(const char* tag, std::string text): XmlAudioNode(tag), text(std::move(text)) {}

XmlTextNode::XmlTextNode(const char* tag): XmlAudioNode(tag) {}

void XmlTextNode::setText(std::string text) { this->text = std::move(text); }

void XmlTextNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    string tmp(this->text);
    StringUtils::replaceAllChars(tmp,
                                 {replace_pair('&', "&amp;"), replace_pair('<', "&lt;"), replace_pair('>', "&gt;")});
    out->write(tmp);

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
