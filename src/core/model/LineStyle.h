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

private:
    std::vector<double> dashes;
};
