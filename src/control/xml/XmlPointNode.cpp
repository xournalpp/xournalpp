#include "XmlPointNode.h"

XmlPointNode::XmlPointNode(const char* tag) : XmlNode(tag)
{
	XOJ_INIT_TYPE(XmlPointNode);

	this->points = NULL;
	this->timestamp = 0;
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
	this->timestamp = 0;

	XOJ_RELEASE_TYPE(XmlPointNode);
}

string XmlPointNode::getAudioFilename()
{
	return this->audioFilename;
}

void XmlPointNode::setAudioFilename(string filename)
{
	this->audioFilename = filename;
}

void XmlPointNode::setTimestamp(int seconds)
{
	this->timestamp = seconds;
}

int XmlPointNode::getTimestamp()
{
	return this->timestamp;
}

void XmlPointNode::addPoint(const Point* point)
{
	XOJ_CHECK_TYPE(XmlPointNode);

	this->points = g_list_append(this->points, new Point(*point));
}

void XmlPointNode::writeOut(OutputStream* out)
{
	XOJ_CHECK_TYPE(XmlPointNode);

	/**
	 * Write timestamp (if present) before each stroke.
	 * This way, when reading, we only need to store 
	 * 1 timestamp value at a time 
	 * and assign it to the consequent stroke.
	 * By adding it this way we don't break 
	 * xournal's fileformat backcompatibility
	 */
	if(this->audioFilename.length() != 0)
	{
		out->write("<timestamp ");
		out->write("ts=\"");
		out->write(std::to_string(this->timestamp));	//must be set via saveHandler
		out->write("\"");
		out->write("fn=\"");
		out->write(this->audioFilename);
		out->write("\"");
		out->write("></timestamp>");
	}

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
		char* tmp = g_strdup_printf("%0.2lf %0.2lf", p->x, p->y);
		out->write(tmp);
		g_free(tmp);
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
