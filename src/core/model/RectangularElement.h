/*
 * Xournal++
 *
 * An element with a somewhat intrinsic rectangular shape
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"

class RectangularElement: public Element {
protected:
    RectangularElement(ElementType type);

public:
    ~RectangularElement() override = default;
    RectangularElement(const RectangularElement&) = default;
    RectangularElement& operator=(const RectangularElement&) = default;
    RectangularElement(RectangularElement&&) = default;
    RectangularElement& operator=(RectangularElement&&) = default;

    inline const xoj::util::Point<double>& getOrigin() const override { return snappedBounds.getOrigin(); }
    void setOrigin(double x, double y);

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    /// Returns the distance between the element "as drawn" and the point (x,y)
    double distanceTo(double x, double y) const override;

    bool isInSelection(ShapeContainer* container) const override;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;
};
