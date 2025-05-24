/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

class Inertia;
class PiecewiseLinearPath;
class Spline;

class CircleRecognizer {
private:
    CircleRecognizer();
    virtual ~CircleRecognizer();

public:
    static std::shared_ptr<Spline> recognize(const PiecewiseLinearPath& path);

private:
    static double scoreCircle(const PiecewiseLinearPath& path, Inertia& inertia);
};
