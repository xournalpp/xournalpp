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

#include "util/serializing/Serializable.h"  // for Serializable

#include <vector>

class ObjectInputStream;
class ObjectOutputStream;


class LineStyle: public Serializable {
public:
    LineStyle();
    //LineStyle(const LineStyle& other);
    ~LineStyle() override;

    /*
    void operator=(const LineStyle& other);
    */
    bool operator==(const LineStyle& other) const;

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

public:
    /**
     * Get dash array and count
     *
     * @return true if dashed
     */
    const std::vector<double>& getDashes() const;

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
    void setDashes(std::vector<double>&& dashes);

private:
    std::vector<double> dashes;
};
