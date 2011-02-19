#include "XmlNode.h"
#include "TextAttribute.h"
#include "DoubleAttribute.h"
#include "IntAttribute.h"
#include "DoubleArrayAttribute.h"

#include <string.h>

XmlNode::XmlNode(const char * tag) {
	this->tag = g_strdup(tag);
	this->attributes = NULL;
	this->children = NULL;
}

XmlNode::~XmlNode() {
	CHECK_MEMORY(this);

	for (GList * l = this->children; l != NULL; l = l->next) {
		XmlNode * node = (XmlNode *) l->data;
		CHECK_MEMORY(node);
		delete node;
	}
	g_list_free(this->children);
	this->children = NULL;

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;
		CHECK_MEMORY(attrib);
		delete attrib;
	}
	g_list_free(this->attributes);
	this->attributes = NULL;

	g_free(this->tag);
	this->tag = NULL;
}

void XmlNode::setAttrib(const char * attrib, const char * value) {
	CHECK_MEMORY(this);

	if (value == NULL) {
		value = "";
	}
	putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char * attrib, double value) {
	CHECK_MEMORY(this);

	putAttrib(new DoubleAttribute(attrib, value));
}

void XmlNode::setAttrib(const char * attrib, int value) {
	CHECK_MEMORY(this);

	putAttrib(new IntAttribute(attrib, value));
}

/**
 * The double array is now owned by XmlNode and automatically deleted!
 */
void XmlNode::setAttrib(const char * attrib, double * value, int count) {
	CHECK_MEMORY(this);

	putAttrib(new DoubleArrayAttribute(attrib, value, count));
}

void XmlNode::writeOut(OutputStream * out) {
	CHECK_MEMORY(this);

	out->write("<");
	out->write(tag);
	writeAttributes(out);

	if (this->children == NULL) {
		out->write("/>\n");
	} else {
		out->write(">\n");

		for (GList * l = this->children; l != NULL; l = l->next) {
			XmlNode * node = (XmlNode *) l->data;
			CHECK_MEMORY(node);

			node->writeOut(out);
		}

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}
}

void XmlNode::addChild(XmlNode * node) {
	CHECK_MEMORY(this);

	this->children = g_list_append(this->children, node);
}

void XmlNode::putAttrib(Attribute * a) {
	CHECK_MEMORY(this);

	CHECK_MEMORY(a);

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;
		CHECK_MEMORY(attrib);

		if (strcmp(attrib->getName(), a->getName())==0) {
			delete attrib;
			l->data = a;
			return;
		}
	}

	this->attributes = g_list_append(this->attributes, a);
}

void XmlNode::writeAttributes(OutputStream * out) {
	CHECK_MEMORY(this);

	for (GList * l = this->attributes; l != NULL; l = l->next) {
		Attribute * attrib = (Attribute *) l->data;
		CHECK_MEMORY(attrib);
		out->write(" ");
		out->write(attrib->getName());
		out->write("=\"");
		attrib->writeOut(out);
		out->write("\"");
	}
}

