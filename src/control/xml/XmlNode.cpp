#include "XmlNode.h"
#include "TextAttribute.h"
#include "DoubleAttribute.h"
#include "IntAttribute.h"
#include "DoubleArrayAttribute.h"

#include <string.h>

XmlNode::XmlNode(const char * tag) {
	XOJ_INIT_TYPE(XmlNode);

	this->tag = g_strdup(tag);
	this->attributes = NULL;
	this->children = NULL;
}

XmlNode::~XmlNode() {
	XOJ_CHECK_TYPE(XmlNode);

	for (GList * l = this->children; l != NULL; l = l->next) {
		XmlNode * node = (XmlNode *) l->data;
		delete node;
	}
	g_list_free(this->children);
	this->children = NULL;

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;
		delete attrib;
	}
	g_list_free(this->attributes);
	this->attributes = NULL;

	g_free(this->tag);
	this->tag = NULL;

	XOJ_RELEASE_TYPE(XmlNode);
}

void XmlNode::setAttrib(const char * attrib, const char * value) {
	XOJ_CHECK_TYPE(XmlNode);

	if (value == NULL) {
		value = "";
	}
	putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char * attrib, double value) {
	XOJ_CHECK_TYPE(XmlNode);

	putAttrib(new DoubleAttribute(attrib, value));
}

void XmlNode::setAttrib(const char * attrib, int value) {
	XOJ_CHECK_TYPE(XmlNode);

	putAttrib(new IntAttribute(attrib, value));
}

/**
 * The double array is now owned by XmlNode and automatically deleted!
 */
void XmlNode::setAttrib(const char * attrib, double * value, int count) {
	XOJ_CHECK_TYPE(XmlNode);

	putAttrib(new DoubleArrayAttribute(attrib, value, count));
}

void XmlNode::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(XmlNode);

	out->write("<");
	out->write(tag);
	writeAttributes(out);

	if (this->children == NULL) {
		out->write("/>\n");
	} else {
		out->write(">\n");

		for (GList * l = this->children; l != NULL; l = l->next) {
			XmlNode * node = (XmlNode *) l->data;
			node->writeOut(out);
		}

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}
}

void XmlNode::addChild(XmlNode * node) {
	XOJ_CHECK_TYPE(XmlNode);

	this->children = g_list_append(this->children, node);
}

void XmlNode::putAttrib(Attribute * a) {
	XOJ_CHECK_TYPE(XmlNode);

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;

		if (strcmp(attrib->getName(), a->getName()) == 0) {
			delete attrib;
			l->data = a;
			return;
		}
	}

	this->attributes = g_list_append(this->attributes, a);
}

void XmlNode::writeAttributes(OutputStream * out) {
	XOJ_CHECK_TYPE(XmlNode);

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;
		out->write(" ");
		out->write(attrib->getName());
		out->write("=\"");
		attrib->writeOut(out);
		out->write("\"");
	}
}

