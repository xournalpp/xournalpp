#include "LineStyle.h"

#include <cstring>  // for memcpy
#include <vector>   // for vector

#include <glib.h>  // for g_free, g_malloc

#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream


LineStyle::LineStyle() = default;

LineStyle::~LineStyle() = default;

auto LineStyle::operator==(const LineStyle& rhs) const -> bool {
    return dashes == rhs.dashes;
}

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

/**
 * Get dash vector
 *
 * @return dashes
 */
auto LineStyle::getDashes() const -> const std::vector<double>& {
    return dashes;
}

/**
 * Set the dash vector and count
 *
 * @param dashes Dash data, will be moved, and continous use from caller invalid
 */
void LineStyle::setDashes(std::vector<double>&& dashes) {
    this->dashes = std::move(dashes);
}

/**
 * Has LineStyle any dashes
 *
 * @return true if dashed is not empty
 */
auto LineStyle::hasDashes() const -> bool { return !dashes.empty(); }
