#include "XmlPointNode.h"

XmlPointNode::XmlPointNode(const char * tag) :
	XmlNode(tag) {
	this->points = NULL;
}

XmlPointNode::~XmlPointNode() {
	delete points;
}

/**
 * The point array is owned by the XML Node and automatically deleted
 */
void XmlPointNode::setPoints(Point * points, int count) {
	// Delete may old data
	delete this->points;

	this->points = points;
	this->count = count;
}

void XmlPointNode::writeOut(OutputStream * out) {
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	for (int i = 0; i < this->count; i++) {
		if (i != 0) {
			out->write(" ");
		}
		Point p = points[i];
		char * tmp = g_strdup_printf("%0.2lf %0.2lf", p.x, p.y);
		out->write(tmp);
		g_free(tmp);
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
