#include "XmlPointNode.h"

XmlPointNode::XmlPointNode(const char * tag) :
	XmlNode(tag) {
	XOJ_INIT_TYPE(XmlPointNode);

	this->points = NULL;
}

XmlPointNode::~XmlPointNode() {
	XOJ_CHECK_TYPE(XmlPointNode);

	for(GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *)l->data;
		delete p;
	}
	g_list_free(this->points);
	this->points = NULL;

	XOJ_RELEASE_TYPE(XmlPointNode);
}

void XmlPointNode::addPoint(const Point * point) {
	XOJ_CHECK_TYPE(XmlPointNode);

	this->points = g_list_append(this->points, new Point(*point));
}

void XmlPointNode::writeOut(OutputStream * out) {
	XOJ_CHECK_TYPE(XmlPointNode);

	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(">");

	for(GList * l = this->points; l != NULL; l = l->next) {
		Point * p = (Point *)l->data;
		if (l != this->points) {
			out->write(" ");
		}
		char * tmp = g_strdup_printf("%0.2lf %0.2lf", p->x, p->y);
		out->write(tmp);
		g_free(tmp);
	}

	out->write("</");
	out->write(tag);
	out->write(">\n");
}
