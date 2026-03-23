#include "LineStyle.h"

#include <cstring>  // for memcpy
#include <vector>   // for vector

#include <glib.h>  // for g_free, g_malloc

#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream


LineStyle::LineStyle() = default;

LineStyle::~LineStyle() = default;

auto LineStyle::operator==(const LineStyle& rhs) const -> bool { return dashes == rhs.dashes; }

void LineStyle::serialize(ObjectOutputStream& out) const {
    out.writeObject("LineStyle");

    out.writeData(this->dashes);

    out.endObject();
}

void LineStyle::readSerialized(ObjectInputStream& in) {
    in.readObject("LineStyle");

    in.readData(dashes);

    in.endObject();
}

auto LineStyle::getDashes() const -> const std::vector<double>& { return dashes; }

void LineStyle::setDashes(std::vector<double>&& dashes) { this->dashes = std::move(dashes); }

auto LineStyle::hasDashes() const -> bool { return !dashes.empty(); }

std::vector<double> LineStyle::getDashesScaledToStrokeWidth(const double strokeWidth) const {
    std::vector<double> scaledDashes;
    scaledDashes.reserve(dashes.size());

    for (double dash: dashes) {
        dash *= strokeWidth;
        scaledDashes.push_back(dash);
    }

    return scaledDashes;
}


void LineStyle::scaleDashesToStrokeWidth(const double strokeWidth) {
    for (double& dash: dashes) {
        dash *= strokeWidth;
    }
}
