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

#include <iosfwd>  // for ptrdiff_t

#include <gdk/gdk.h>  // for GdkRectangle

#include "util/Color.h"                     // for Color
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

class Element: public Serializable {
protected:
    Element(ElementType type);

public:
    ~Element() override;

    using Index = std::ptrdiff_t;
    static constexpr auto InvalidIndex = static_cast<Index>(-1);

public:
    ElementType getType() const;

    void setX(double x);
    void setY(double y);
    double getX() const;
    double getY() const;

    virtual void move(double dx, double dy);
    virtual void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) = 0;
    virtual void rotate(double x0, double y0, double th) = 0;

    void setColor(Color color);
    Color getColor() const;

    double getElementWidth() const;
    double getElementHeight() const;

    xoj::util::Rectangle<double> getSnappedBounds() const;

    xoj::util::Rectangle<double> boundingRect() const;

    virtual bool intersectsArea(const GdkRectangle* src) const;
    virtual bool intersectsArea(double x, double y, double width, double height) const;

    virtual bool isInSelection(ShapeContainer* container) const;

    virtual bool rescaleOnlyAspectRatio();
    virtual bool rescaleWithMirror();

    /**
     * Take 1:1 copy of this element
     */
    virtual Element* clone() const = 0;

    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

private:
protected:
    virtual void calcSize() const = 0;

protected:
    // If the size has been calculated
    mutable bool sizeCalculated = false;

    mutable double width = 0;
    mutable double height = 0;

    // The position on the screen
    mutable double x = 0;
    mutable double y = 0;

    // The position and dimensions on the screen used for snapping
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
