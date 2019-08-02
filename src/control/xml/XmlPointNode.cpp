#include "Util.h"
#include "XmlPointNode.h"

XmlPointNode::XmlPointNode(const char* tag)
 : XmlAudioNode(tag),
   points(NULL)
{
	XOJ_INIT_TYPE(XmlPointNode);
}

XmlPointNode::~XmlPointNode()
{
	XOJ_CHECK_TYPE(XmlPointNode);

	for (GList* l = this->points; l != NULL; l = l->next)
	{
		Point* p = (Point*) l->data;
		delete p;
	}
	g_list_free(this->points);
	this->points = NULL;

	XOJ_RELEASE_TYPE(XmlPointNode);
}

void XmlPointNode::addPoint(const Point* point)
{
	XOJ_CHECK_TYPE(XmlPointNode);

	this->points = g_list_append(this->points, new Point(*point));
}

void XmlPointNode::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(XmlPointNode);

	/** Write stroke and its attributes */
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	for (GList* l = this->points; l != NULL; l = l->next)
	{
		Point* p = (Point*) l->data;
		if (l != this->points)
		{
			out->write(" ");
		}

		gchar coordinateStr[Util::PRECISION_FORMAT_BUFF_LEN];
		Util::getCoordinateString(coordinateStr, Util::PRECISION_FORMAT_BUFF_LEN, p->x, p->y);
		out->write(coordinateStr);
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
