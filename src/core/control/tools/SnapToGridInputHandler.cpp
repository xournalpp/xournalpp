#include "SnapToGridInputHandler.h"

#include "control/settings/Settings.h"
#include "model/Snapping.h"

SnapToGridInputHandler::SnapToGridInputHandler(const Settings* settings): settings(settings) {}

double SnapToGridInputHandler::snapVertically(double y, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapVertically(y, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return y;
}

double SnapToGridInputHandler::snapHorizontally(double x, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapHorizontally(x, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return x;
}

Point SnapToGridInputHandler::snapToGrid(Point const& pos, bool alt) const {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapToGrid(pos, settings->getSnapGridSize(), settings->getSnapGridTolerance());
    }
    return pos;
}

double SnapToGridInputHandler::snapAngle(double radian, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapAngle(radian, settings->getSnapRotationTolerance());
    }
    return radian;
}

Point SnapToGridInputHandler::snapRotation(Point const& pos, Point const& center, bool alt) const {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapRotation(pos, center, settings->getSnapRotationTolerance());
    }
    return pos;
}

Point SnapToGridInputHandler::snap(Point const& pos, Point const& center, bool alt) const {
    Point rotationSnappedPoint{snapRotation(pos, center, alt)};
    return snapToGrid(rotationSnappedPoint, alt);
}
