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

#include "util/Matrix.h"
#include "util/Size.h"

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

    inline const xoj::util::Point<double>& getOrigin() const override { return transformationMatrix.shift; }

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    inline const xoj::util::Matrix& getTransformation() const { return transformationMatrix; }
    void setTransformation(const xoj::util::Matrix& m);

    /// Returns the distance between the element "as drawn" and the point (x,y)
    double distanceTo(double x, double y) const override;

    bool isInSelection(ShapeContainer* container) const override;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    const xoj::util::Size<double>& getNaturalSize() const;

protected:
    /**
     * Matrix applied to the canvas before drawing the element - encodes various transformations
     * It always goes from the element's local coordinates to Page coordinates.
     */
    xoj::util::Matrix transformationMatrix;

    /**
     * Size of the rectangle of the element in element coordinates (i.e. before transformation)
     *
     * WARNING: In some cases, the element could go beyond or be smaller than its "natural size" (e.g. text overflowing)
     */
    mutable xoj::util::Size<double> naturalSize;
};
