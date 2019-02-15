#include "XmlTexNode.h"

XmlTexNode::XmlTexNode(const char* tag, string& binaryData)
 : XmlNode(tag),
   binaryData(binaryData)
{
	XOJ_INIT_TYPE(XmlTexNode);
}

XmlTexNode::~XmlTexNode()
{
	XOJ_CHECK_TYPE(XmlTexNode);

	XOJ_RELEASE_TYPE(XmlTexNode);
}

void XmlTexNode::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(XmlTexNode);

	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	gchar* base64_str = g_base64_encode((const guchar*)this->binaryData.c_str(), this->binaryData.length());
	out->write(base64_str);
	g_free(base64_str);

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
