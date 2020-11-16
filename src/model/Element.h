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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "serializing/Serializeable.h"

#include "Color.h"
#include "Rectangle.h"
#include "XournalType.h"

enum ElementType { ELEMENT_STROKE = 1, ELEMENT_IMAGE, ELEMENT_TEXIMAGE, ELEMENT_TEXT };

class ShapeContainer {
public:
    virtual bool contains(double x, double y) = 0;

    virtual ~ShapeContainer() = default;
};

class Element: public Serializeable {
protected:
    Element(ElementType type);

public:
    ~Element() override;

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

    Rectangle<double> getSnappedBounds() const;

    Rectangle<double> boundingRect() const;

    virtual bool intersectsArea(const GdkRectangle* src);
    virtual bool intersectsArea(double x, double y, double width, double height);

    virtual bool isInSelection(ShapeContainer* container);

    virtual bool rescaleOnlyAspectRatio();

    /**
     * Take 1:1 copy of this element
     */
    virtual Element* clone() = 0;

private:
protected:
    virtual void calcSize() const = 0;

    void serializeElement(ObjectOutputStream& out) const;
    void readSerializedElement(ObjectInputStream& in);

protected:
    // If the size has been calculated
    mutable bool sizeCalculated = false;

    mutable double width = 0;
    mutable double height = 0;

    // The position on the screen
    mutable double x = 0;
    mutable double y = 0;

    // The position and dimensions on the screen used for snapping
    mutable Rectangle<double> snappedBounds{};

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
