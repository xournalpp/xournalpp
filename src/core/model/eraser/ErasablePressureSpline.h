/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include "model/SplineSegment.h"

#include "ErasableStroke.h"

namespace xoj {
namespace view {
class ErasableStrokeView;
};
};  // namespace xoj

class ErasablePressureSpline: public ErasableStroke {
public:
    ErasablePressureSpline(const Stroke& stroke);
    virtual ~ErasablePressureSpline() = default;

    Type getType() const override { return PRESSURE_SPLINE; }

private:
    std::vector<std::vector<SplineSegment::ParametrizedPoint>> pointCache;
    friend class xoj::view::ErasableStrokeView;
};
