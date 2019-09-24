#include "XmlTextNode.h"

#include <StringUtils.h>

XmlTextNode::XmlTextNode(const char* tag, const char* text) : XmlAudioNode(tag)
{
	this->text = g_strdup(text);
}

XmlTextNode::XmlTextNode(const char* tag) : XmlAudioNode(tag)
{
	this->text = nullptr;
}

XmlTextNode::~XmlTextNode()
{
	g_free(this->text);
	this->text = nullptr;
}

void XmlTextNode::setText(const char* text)
{
	g_free(this->text);
	this->text = g_strdup(text);
}

void XmlTextNode::writeOut(OutputStream* out)
{
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	string tmp(this->text);
	StringUtils::replaceAllChars(tmp,{
		replace_pair('&', "&amp;"),
		replace_pair('<', "&lt;"),
		replace_pair('>', "&gt;")
	});
	out->write(tmp);

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
