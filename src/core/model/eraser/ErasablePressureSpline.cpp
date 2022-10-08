#include "ErasablePressureSpline.h"

#include "model/Stroke.h"
#include "model/path/Spline.h"

ErasablePressureSpline::ErasablePressureSpline(const Stroke& stroke): ErasableStroke(stroke) {
    if (this->stroke.path->getType() != Path::SPLINE) {
        g_warning("ErasablePressureSpline created from splineless stroke");
        return;
    }
    const Spline& spline = dynamic_cast<const Spline&>(this->stroke.getPath());
    this->pointCache.reserve(spline.nbSegments());
    for (auto&& seg: spline.segments()) {
        std::vector<SplineSegment::ParametrizedPoint> pts;
        seg.toParametrizedPoints(pts);
        this->pointCache.push_back(std::move(pts));
    }
}
