/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XmlNode.h"  // for XmlNode

class OutputStream;
class Point;

class XmlStrokeNode: public XmlNode {
public:
    XmlStrokeNode(const char* tag);
    virtual ~XmlStrokeNode();

public:
    void setWidth(double width, const double* widths, int widthsLength);

    void writeOut(OutputStream* out) override;

private:
    Point* points;
    int pointsLength;

    double width;

    double* widths;
    int widthsLength;
};
