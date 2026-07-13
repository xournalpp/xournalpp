/*
 * Xournal++
 *
 * An element on the Document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for ptrdiff_t
#include <memory>   // for unique_ptr
#include <vector>   // for vector

#include <gdk/gdk.h>  // for GdkRectangle

#include "util/Color.h"                     // for Color
#include "util/Rectangle.h"                 // for Rectangle
#include "util/serializing/Serializable.h"  // for Serializable

class ObjectInputStream;
class ObjectOutputStream;

namespace xoj::util {
template <class T>
class Point;
};

enum ElementType { ELEMENT_STROKE = 1, ELEMENT_IMAGE, ELEMENT_TEXIMAGE, ELEMENT_TEXT, ELEMENT_LINK };

class ShapeContainer {
public:
    virtual bool contains(double x, double y) const = 0;

    virtual ~ShapeContainer() = default;
};

class Element;
using ElementPtr = std::unique_ptr<Element>;

class Element: public Serializable {
protected:
    Element(ElementType type);

public:
    ~Element() override = default;

    using Index = std::ptrdiff_t;
    static constexpr auto InvalidIndex = static_cast<Index>(-1);

public:
    ElementType getType() const;

    /// Move the element to (x,y).
    virtual void setOrigin(double x, double y);
    virtual const xoj::util::Point<double>& getOrigin() const;

    virtual void move(double dx, double dy);
    virtual void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) = 0;
    virtual void rotate(double x0, double y0, double th) = 0;

    void setColor(Color color);
    Color getColor() const;

    const xoj::util::Rectangle<double>& getSnappedBounds() const;

    const xoj::util::Rectangle<double>& getBoundingBox() const;

    virtual bool intersectsArea(double x, double y, double width, double height) const;
    /// Returns the distance between the element "as drawn" and the point (x,y)
    virtual double distanceTo(double x, double y) const;
    virtual bool hasBoundingBoxContaining(double x, double y) const;

    virtual bool isInSelection(ShapeContainer* container) const;

    virtual bool rescaleOnlyAspectRatio() const;
    virtual bool rescaleWithMirror() const;

    /**
     * Take 1:1 copy of this element
     */
    virtual auto clone() const -> ElementPtr = 0;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
protected:
    virtual void calcSize() const = 0;

protected:
    /// Whether the size has been calculated or not
    mutable bool sizeCalculated = false;

    /// Rectangular area containing the element as drawn
    mutable xoj::util::Rectangle<double> boundingBox{};

    /// The position and dimensions on the screen used for snapping
    mutable xoj::util::Rectangle<double> snappedBounds{};

private:
    /**
     * Type of this element
     */
    ElementType type;

    /**
     * The color in RGB format
     */
    Color color{0U};
};

namespace xoj {

auto refElementContainer(const std::vector<ElementPtr>& elements) -> std::vector<Element*>;

}  // namespace xoj
