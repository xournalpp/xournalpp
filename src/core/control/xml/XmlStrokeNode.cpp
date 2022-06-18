#include "XmlStrokeNode.h"

#include <glib.h>  // for g_ascii_formatd, G_ASCII_DTOSTR_BUF...

#include "control/xml/XmlNode.h"  // for XmlNode
#include "model/Point.h"          // for Point
#include "util/OutputStream.h"    // for OutputStream
#include "util/Util.h"            // for writeCoordinateString, PRECISION_FO...

XmlStrokeNode::XmlStrokeNode(const char* tag): XmlNode(tag) {
    this->points = nullptr;
    this->pointsLength = 0;
    this->width = 0;
    this->widths = nullptr;
    this->widthsLength = 0;
}

XmlStrokeNode::~XmlStrokeNode() {
    delete[] this->points;
    delete[] this->widths;
}

void XmlStrokeNode::setWidth(double width, const double* widths, int widthsLength) {
    this->width = width;


    delete[] this->widths;

    this->widths = new double[widthsLength];
    for (int i = 0; i < widthsLength; i++) { this->widths[i] = widths[i]; }
    this->widthsLength = widthsLength;
}

void XmlStrokeNode::writeOut(OutputStream* out) {
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(" width=\"");

    char widthStr[G_ASCII_DTOSTR_BUF_SIZE];
    // g_ascii_ version uses C locale always.
    g_ascii_formatd(widthStr, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, width);
    out->write(widthStr);

    for (int i = 0; i < widthsLength; i++) {
        g_ascii_formatd(widthStr, G_ASCII_DTOSTR_BUF_SIZE, Util::PRECISION_FORMAT_STRING, widths[i]);
        out->write(" ");
        out->write(widthStr);
    }

    out->write("\"");

    if (this->pointsLength == 0) {
        out->write("/>");
    } else {
        out->write(">");

        Util::writeCoordinateString(out, points[0].x, points[0].y);

        for (int i = 1; i < this->pointsLength; i++) {
            out->write(" ");
            Util::writeCoordinateString(out, points[i].x, points[i].y);
        }

        out->write("</");
        out->write(tag);
        out->write(">\n");
    }
}
