#include "XmlNode.h"

#include "DoubleArrayAttribute.h"
#include "DoubleAttribute.h"
#include "IntAttribute.h"
#include "SizeTAttribute.h"
#include "TextAttribute.h"

#include "control/jobs/ProgressListener.h"


XmlNode::XmlNode(const char* tag)
{
	this->tag = g_strdup(tag);
	this->attributes = nullptr;
	this->children = nullptr;
}

XmlNode::~XmlNode()
{
	for (GList* l = this->children; l != nullptr; l = l->next)
	{
		XmlNode* node = (XmlNode*) l->data;
		delete node;
	}
	g_list_free(this->children);
	this->children = nullptr;

	for (GList* l = this->attributes; l != nullptr; l = l->next)
	{
		XMLAttribute* attrib = (XMLAttribute*) l->data;
		delete attrib;
	}
	g_list_free(this->attributes);
	this->attributes = nullptr;

	g_free(this->tag);
	this->tag = nullptr;
}

void XmlNode::setAttrib(const char* attrib, const char* value)
{
	if (value == nullptr)
	{
		value = "";
	}
	putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, string value)
{
	putAttrib(new TextAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, double value)
{
	putAttrib(new DoubleAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, int value)
{
	putAttrib(new IntAttribute(attrib, value));
}

void XmlNode::setAttrib(const char* attrib, size_t  value)
{
	putAttrib(new SizeTAttribute(attrib, value));
}

/**
 * The double array is now owned by XmlNode and automatically deleted!
 */
void XmlNode::setAttrib(const char* attrib, double* value, int count)
{
	putAttrib(new DoubleArrayAttribute(attrib, value, count));
}

void XmlNode::writeOut(OutputStream* out, ProgressListener* listener)
{
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	if (this->children == nullptr)
	{
		out->write("/>\n");
	}
	else
	{
		out->write(">\n");

		if (listener)
		{
			listener->setMaximumState(g_list_length(this->children));
		}

		guint i = 1;

		for (GList* l = this->children; l != nullptr; l = l->next, ++i)
		{
			XmlNode* node = (XmlNode*) l->data;
			node->writeOut(out);
			if (listener)
			{
				listener->setCurrentState(i);
			}
		}

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}
}

void XmlNode::addChild(XmlNode* node)
{
	this->children = g_list_append(this->children, node);
}

void XmlNode::putAttrib(XMLAttribute* a)
{
	for (GList* l = this->attributes; l != nullptr; l = l->next)
	{
		XMLAttribute* attrib = (XMLAttribute*) l->data;

		if (attrib->getName() == a->getName())
		{
			delete attrib;
			l->data = a;
			return;
		}
	}

	this->attributes = g_list_append(this->attributes, a);
}

void XmlNode::writeAttributes(OutputStream* out)
{
	for (GList* l = this->attributes; l != nullptr; l = l->next)
	{
		XMLAttribute* attrib = (XMLAttribute*) l->data;
		out->write(" ");
		out->write(attrib->getName());
		out->write("=\"");
		attrib->writeOut(out);
		out->write("\"");
	}
}

