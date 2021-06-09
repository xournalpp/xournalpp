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

#include "EraseableStroke.h"

class EraseablePressureSpline: public EraseableStroke {
public:
    EraseablePressureSpline(Stroke* stroke);
    virtual ~EraseablePressureSpline() = default;

    virtual void draw(cairo_t* cr);

private:
    std::vector<std::vector<ParametrizedPoint>> pointCache;
};
