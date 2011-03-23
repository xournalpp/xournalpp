#include "XmlTextNode.h"

XmlTextNode::XmlTextNode(const char * tag, const char * text) :
	XmlNode(tag) {
	XOJ_INIT_TYPE(XmlTextNode);

	this->text = g_strdup(text);
}

XmlTextNode::XmlTextNode(const char * tag) :
	XmlNode(tag) {
	XOJ_INIT_TYPE(XmlTextNode);

	this->text = NULL;

	XOJ_RELEASE_TYPE(XmlTextNode);
}

XmlTextNode::~XmlTextNode() {
	XOJ_CHECK_TYPE(XmlTextNode);

	g_free(this->text);
	this->text = NULL;

	XOJ_RELEASE_TYPE(XmlTextNode);
}

void XmlTextNode::setText(const char * text) {
	XOJ_CHECK_TYPE(XmlTextNode);

	g_free(this->text);
	this->text = g_strdup(text);
}

void XmlTextNode::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(XmlTextNode);

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
