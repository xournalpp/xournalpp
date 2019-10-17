#include "Util.h"
#include "XmlStrokeNode.h"

XmlStrokeNode::XmlStrokeNode(const char* tag) : XmlNode(tag)
{
	this->points = nullptr;
	this->pointsLength = 0;
	this->width = 0;
	this->widths = nullptr;
	this->widthsLength = 0;
}

XmlStrokeNode::~XmlStrokeNode()
{
	delete[] this->points;
	delete[] this->widths;
}

void XmlStrokeNode::setPoints(Point* points, int pointsLength)
{
	if (this->points)
	{
		delete[] this->points;
	}
	this->points = new Point[pointsLength];
	for (int i = 0; i < pointsLength; i++)
	{
		this->points[i] = points[i];
	}
	this->pointsLength = pointsLength;
}

void XmlStrokeNode::setWidth(double width, double* widths, int widthsLength)
{
	this->width = width;

	if (this->widths)
	{
		delete[] this->widths;
	}
	this->widths = new double[widthsLength];
	for (int i = 0; i < widthsLength; i++)
	{
		this->widths[i] = widths[i];
	}
	this->widthsLength = widthsLength;

}

void XmlStrokeNode::writeOut(OutputStream* out)
{
	out->write("<");
	out->write(tag);
	writeAttributes(out);

	out->write(" width=\"");

	char widthStr[G_ASCII_DTOSTR_BUF_SIZE];
	// g_ascii_ version uses C locale always.
	g_ascii_formatd(widthStr, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, width);
	out->write(widthStr);

	for (int i = 0; i < widthsLength; i++)
	{
		g_ascii_formatd(widthStr, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, widths[i]);
		out->write(" ");
		out->write(widthStr);
	}

	out->write("\"");

	if (this->pointsLength == 0)
	{
		out->write("/>");
	}
	else
	{
		out->write(">");

		Util::writeCoordinateString(out, points[0].x, points[0].y);

		for (int i = 1; i < this->pointsLength; i++)
		{
			out->write(" ");
			Util::writeCoordinateString(out, points[i].x, points[i].y);
		}

		out->write("</");
		out->write(tag);
		out->write(">\n");
	}
}
