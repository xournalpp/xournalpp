#include "LineStyle.h"

#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"


LineStyle::LineStyle(): dashCount(0), dashes(nullptr), heavyDownstrokeRatio(1.0){};

LineStyle::LineStyle(std::vector<double> dashes, double heavyDownstrokeRatio):
        dashCount(dashes.size()), heavyDownstrokeRatio(heavyDownstrokeRatio) {
    this->dashes = static_cast<double*>(g_malloc(dashCount * sizeof(double)));
    std::copy(dashes.begin(), dashes.end(), this->dashes);
}

LineStyle::LineStyle(std::initializer_list<double> dashes, double heavyDownstrokeRatio):
        dashCount(dashes.size()), heavyDownstrokeRatio(heavyDownstrokeRatio) {
    this->dashes = static_cast<double*>(g_malloc(dashCount * sizeof(double)));
    std::copy(dashes.begin(), dashes.end(), this->dashes);
}

LineStyle::LineStyle(const LineStyle& other) {
    const double* other_dashes;
    int other_dashCount;
    other.getDashes(other_dashes, other_dashCount);
    this->setDashes(other_dashes, other_dashCount);
    this->setHeavyDownstrokeRatio(other.getHeavyDownstrokeRatio());
}

LineStyle::~LineStyle() {
    g_free(this->dashes);
    this->dashes = nullptr;
    this->dashCount = 0;
    this->heavyDownstrokeRatio = 0.0;
}

auto LineStyle::operator=(const LineStyle& other) -> LineStyle& {
    if (this == &other)
        return *this;

    const double* dashes = nullptr;
    int dashCount = 0;

    other.getDashes(dashes, dashCount);
    setDashes(dashes, dashCount);

    setHeavyDownstrokeRatio(other.getHeavyDownstrokeRatio());

    return *this;
}

auto LineStyle::operator==(const LineStyle& other) const -> bool {
    const double *dashes, *other_dashes;
    int dashCount, other_dashCount;
    this->getDashes(dashes, dashCount);
    other.getDashes(other_dashes, other_dashCount);
    if (dashCount != other_dashCount)
        return false;
    if (memcmp(dashes, other_dashes, dashCount * sizeof(double)) != 0)
        return false;
    if (this->getHeavyDownstrokeRatio() != other.getHeavyDownstrokeRatio())
        return false;
    return true;
}

void LineStyle::serialize(ObjectOutputStream& out) const {
    out.writeObject("LineStyle");

    out.writeData(this->dashes, this->dashCount, sizeof(double));
    out.writeDouble(this->heavyDownstrokeRatio);

    out.endObject();
}

void LineStyle::readSerialized(ObjectInputStream& in) {
    in.readObject("LineStyle");

    g_free(this->dashes);
    this->dashes = nullptr;
    this->dashCount = 0;
    in.readData(reinterpret_cast<void**>(&this->dashes), &this->dashCount);

    this->heavyDownstrokeRatio = in.readDouble();

    in.endObject();
}

/**
 * Get dash array and count
 *
 * @return true if dashed
 */
auto LineStyle::getDashes(const double*& dashes, int& dashCount) const -> bool {
    dashes = this->dashes;
    dashCount = this->dashCount;

    return this->dashCount > 0;
}

/**
 * Set the dash array and count
 *
 * @param dashes Dash data, will be copied
 * @param dashCount Count of entries
 */
// Todo(fabian): memory use after free
void LineStyle::setDashes(const double* dashes, int dashCount) {
    g_free(this->dashes);
    if (dashCount == 0 || dashes == nullptr) {
        this->dashCount = 0;
        this->dashes = nullptr;
        return;
    }

    this->dashes = static_cast<double*>(g_malloc(dashCount * sizeof(double)));
    this->dashCount = dashCount;

    memcpy(this->dashes, dashes, this->dashCount * sizeof(double));
}

/**
 * Get dash array and count
 *
 * @return true if dashed
 */
auto LineStyle::hasDashes() const -> bool { return this->dashCount > 0; }

// TODO: copy the comments from the header file?

void LineStyle::setHeavyDownstrokeRatio(double ratio) {
    // TODO: the ratio should be non-negative.
    this->heavyDownstrokeRatio = ratio;
}

auto LineStyle::hasHeavyDownstroke() const -> bool { return this->heavyDownstrokeRatio != 1.0; }

auto LineStyle::getHeavyDownstrokeRatio() const -> double { return this->heavyDownstrokeRatio; }

auto LineStyle::isNontrivial() const -> bool { return this->hasDashes() or this->hasHeavyDownstroke(); }
