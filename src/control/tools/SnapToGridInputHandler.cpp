#include "SnapToGridInputHandler.h"

#include "control/settings/Settings.h"
#include "model/Snapping.h"

SnapToGridInputHandler::SnapToGridInputHandler(Settings* settings): settings(settings) {}

Point SnapToGridInputHandler::snapToGrid(Point const& pos, bool alt) {
    if (alt != settings->isSnapGrid()) {
        return Snapping::snapToGrid(pos, settings->getSnapGridSize(),
                                    settings->getSnapGridTolerance());  // grid size is not yet in the settings
    }
    return pos;
}

double SnapToGridInputHandler::snapAngle(double radian, bool alt) {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapAngle(radian, settings->getSnapRotationTolerance());
    }
    return radian;
}

Point SnapToGridInputHandler::snapRotation(Point const& pos, Point const& center, bool alt) {
    if (alt != settings->isSnapRotation()) {
        return Snapping::snapRotation(pos, center, settings->getSnapRotationTolerance());
    }
    return pos;
}

Point SnapToGridInputHandler::snap(Point const& pos, Point const& center, bool alt) {
    Point rotationSnappedPoint{snapRotation(pos, center, alt)};
    return snapToGrid(rotationSnappedPoint, alt);
}
