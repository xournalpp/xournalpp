#include "XmlTextNode.h"

XmlTextNode::XmlTextNode(const char * tag, const char * text) :
	XmlNode(tag) {
	this->text = g_strdup(text);
}

XmlTextNode::XmlTextNode(const char * tag) :
	XmlNode(tag) {
	this->text = NULL;
}

XmlTextNode::~XmlTextNode() {
	g_free(this->text);
	this->text = NULL;
}

void XmlTextNode::setText(const char * text) {
	CHECK_MEMORY(this);
	g_free(this->text);
	this->text = g_strdup(text);
}

void XmlTextNode::writeOut(OutputStream * out) {
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	String tmp = this->text;
	tmp = tmp.replace("&", "&amp;");
	tmp = tmp.replace("<", "&lt;");
	tmp = tmp.replace(">", "&gt;");
	out->write(tmp);

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
