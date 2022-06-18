#include "XmlPointNode.h"

#include <algorithm>  // for max
#include <utility>    // for move

#include "control/xml/XmlAudioNode.h"  // for XmlAudioNode
#include "util/OutputStream.h"         // for OutputStream
#include "util/Util.h"                 // for writeCoordinateString

XmlPointNode::XmlPointNode(const char* tag): XmlAudioNode(tag) {}

void XmlPointNode::addPoint(Point point) { points.emplace_back(std::move(point)); }

void XmlPointNode::writeOut(OutputStream* out) {
    /** Write stroke and its attributes */
    out->write("<");
    out->write(tag);
    writeAttributes(out);

    out->write(">");

    auto pointIter = points.begin();
    Util::writeCoordinateString(out, pointIter->x, pointIter->y);
    ++pointIter;
    for (; pointIter != points.end(); ++pointIter) {
        out->write(" ");

        Util::writeCoordinateString(out, pointIter->x, pointIter->y);
    }

    out->write("</");
    out->write(tag);
    out->write(">\n");
}
