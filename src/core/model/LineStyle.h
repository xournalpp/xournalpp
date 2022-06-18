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

class ObjectInputStream;
class ObjectOutputStream;


class LineStyle: public Serializable {
public:
    LineStyle();
    LineStyle(const LineStyle& other);
    ~LineStyle() override;

    void operator=(const LineStyle& other);

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

private:
    /**
     * Dash definition (nullptr for no Dash)
     */
    double* dashes = nullptr;

    /**
     * Dash count (0 for no dash)
     */
    int dashCount = 0;
};
