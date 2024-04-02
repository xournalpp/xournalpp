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

#include "util/Color.h"  // for Color
#include "util/Matrix.h"
#include "util/Point.h"
#include "util/Rectangle.h"                 // for Rectangle
#include "util/serializing/Serializable.h"  // for Serializable

class ObjectInputStream;
class ObjectOutputStream;

enum ElementType { ELEMENT_STROKE = 1, ELEMENT_IMAGE, ELEMENT_TEXIMAGE, ELEMENT_TEXT };

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

    void setX(double x);
    void setY(double y);
    auto getX() const -> double;
    auto getY() const -> double;
    void setWidth(double width);
    void setHeight(double height);

    void move(double dx, double dy);
    virtual void scale(xoj::util::Point<double> base, double fx, double fy, bool restoreLineWidth);
    void rotate(xoj::util::Point<double> base, double th);

    void setColor(Color color);
    auto getColor() const -> Color;

    auto getElementWidth() const -> double;
    auto getElementHeight() const -> double;

    auto getSnappedBounds() const -> xoj::util::Rectangle<double>;
    auto boundingRect() const -> xoj::util::Rectangle<double>;

    auto getTransformation() const -> xoj::util::Matrix;
    void setTransformation(xoj::util::Matrix const& mtx);

    [[deprecated("use xoj::util::Rectangle<double> instead")]]
    virtual auto intersectsArea(const GdkRectangle* src) const -> bool;
    [[deprecated("use xoj::util::Rectangle<double> instead")]]
    virtual auto intersectsArea(double x, double y, double width, double height) const -> bool;
    virtual auto intersectsArea(xoj::util::Rectangle<double> rect) const -> bool;

    virtual auto isInSelection(ShapeContainer* container) const -> bool;

    virtual auto rescaleOnlyAspectRatio() -> bool;
    virtual auto rescaleWithMirror() -> bool;

    /**
     * Take 1:1 copy of this element
     */
    virtual auto clone() const -> ElementPtr = 0;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
    auto updateBounds() const -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>>;

protected:
    [[nodiscard]]
    virtual auto internalUpdateBounds() const
            -> std::pair<xoj::util::Rectangle<double>, xoj::util::Rectangle<double>> = 0;

protected:
private:
    // // === cache ===
    // mutable bool sizeCalculated = false;
    // mutable xoj::util::Rectangle<double> bounds{};
    // mutable xoj::util::Rectangle<double> snappedBounds{}; ///< The position and dimensions on the screen used for
    // snapping

    // === data ===
    xoj::util::Matrix transformation{};
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
