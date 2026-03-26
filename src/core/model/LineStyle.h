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

#include <vector>

#include "util/serializing/Serializable.h"  // for Serializable

class ObjectInputStream;
class ObjectOutputStream;


class LineStyle: public Serializable {
public:
    LineStyle();
    ~LineStyle() override;

    bool operator==(const LineStyle& other) const;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    /**
     * Get dash vector
     *
     * @return dashes
     */
    const std::vector<double>& getDashes() const;

    /**
     * @return true if dashed
     */
    bool hasDashes() const;

    /**
     * Set the dash vector and count
     *
     * @param dashes Dash data, will be moved, and continuous use from caller invalid
     */
    void setDashes(std::vector<double>&& dashes);

    /**
     * Get the dash vector with the spaces scaled to the stroke width
     *
     * @param strokeWidth
     *
     * @return dashes scaled to the stroke width
     */
    std::vector<double> getDashesScaledToStrokeWidth(const double strokeWidth) const;

    /**
     * Scale dash vector to stroke width
     *
     * @param strokeWidth
     */
    void scaleDashesToStrokeWidth(const double strokeWidth);

    void setScaleDashes() { scaleDashesToWidth = true; }
    void setScaleDashes(const std::string& line_style) { scaleDashesToWidth = line_style.starts_with("scaled"); }
    bool scaleDashes() const { return scaleDashesToWidth; }

private:
    std::vector<double> dashes;
    bool scaleDashesToWidth = false; // Used for pressure-sensitive dash scaling
};
