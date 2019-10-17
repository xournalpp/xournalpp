#include "Util.h"
#include "XmlPointNode.h"

XmlPointNode::XmlPointNode(const char* tag)
 : XmlAudioNode(tag),
   points(nullptr)
{
}

XmlPointNode::~XmlPointNode()
{
	for (GList* l = this->points; l != nullptr; l = l->next)
	{
		Point* p = (Point*) l->data;
		delete p;
	}
	g_list_free(this->points);
	this->points = nullptr;
}

void XmlPointNode::addPoint(const Point* point)
{
	this->points = g_list_append(this->points, new Point(*point));
}

void XmlPointNode::writeOut(OutputStream* out)
{
	/** Write stroke and its attributes */
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	for (GList* l = this->points; l != nullptr; l = l->next)
	{
		Point* p = (Point*) l->data;
		if (l != this->points)
		{
			out->write(" ");
		}

		Util::writeCoordinateString(out, p->x, p->y);
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
