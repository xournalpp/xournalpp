/*
 * Xournal++
 *
 * Dash definition of a stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "util/serializing/Serializable.h"


class LineStyle: public Serializable {
public:
    LineStyle();
    LineStyle(std::initializer_list<double> dashes, double heavyDownStrokeRatio);
    LineStyle(const LineStyle& other);
    virtual ~LineStyle();

    LineStyle& operator=(const LineStyle& other);

    bool operator==(const LineStyle& other) const;
    bool operator!=(const LineStyle& other) const;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const;
    void readSerialized(ObjectInputStream& in);

public:
    /**
     * @return Returns true, when any effect is active, that distinglishes this style from the plain (or trivial) style.
     */
    bool isNontrivial() const;

    /**
     * Get dash array and count
     *
     * @return true if dashed
     */
    bool getDashes(const double*& dashes, int& dashCount) const;

    /**
     * @return true if dashed
     */
    bool hasDashes() const;

    /**
     * Set the dash array and count
     *
     * @param dashes Dash data, will be copied
     * @param dashCount Count of entries
     */
    void setDashes(const double* dashes, int dashCount);

    /**
     * @brief If this is active, the stroke is rendered with a heavier stroke
     * when heading downwards supposed to upwards.
     *
     * @return true, when this option is active for this line style.
     */
    bool hasHeavyDownstroke() const;

    /**
     * @return The ratio between the upward and downward weight.
     */
    double getHeavyDownstrokeRatio() const;

    /**
     * @brief Set the ratio between the upward and downward weight.
     *
     * @param ratio The ratio. The special value 1.0 deactivates this option
     * completely.
     */
    void setHeavyDownstrokeRatio(double ratio);

private:
    /**
     * Dash definition (nullptr for no Dash)
     */
    double* dashes = nullptr;

    /**
     * Dash count (0 for no dash)
     */
    int dashCount = 0;

    /**
     * Ratio between the upward and downward weight. Should be between 0.0 and 1.0.
     *
     * A value of 1.0 represents this feature beeing completely deactivated.
     */
    double heavyDownstrokeRatio = 1.0;
};
