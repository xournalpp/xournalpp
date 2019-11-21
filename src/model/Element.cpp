#include "Element.h"

#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

#include <cmath>

Element::Element(ElementType type)
 : type(type)
{
}

Element::~Element() = default;

auto Element::getType() const -> ElementType
{
	return this->type;
}

void Element::setX(double x)
{
	this->x = x;
}

void Element::setY(double y)
{
	this->y = y;
}

auto Element::getX() -> double
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return x;
}

auto Element::getY() -> double
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return y;
}

void Element::move(double dx, double dy)
{
	this->x += dx;
	this->y += dy;
}

auto Element::getElementWidth() -> double
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->width;
}

auto Element::getElementHeight() -> double
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->height;
}

auto Element::boundingRect() -> Rectangle
{
	return Rectangle(getX(), getY(), getElementWidth(), getElementHeight());
}

void Element::setColor(int color)
{
	this->color = color;
}

auto Element::getColor() const -> int
{
	return this->color;
}

auto Element::intersectsArea(const GdkRectangle* src) -> bool
{
	GdkRectangle rect = {
		gint(getX()),
		gint(getY()),
		gint(getElementWidth()),
		gint(getElementHeight())
	};

	return gdk_rectangle_intersect(src, &rect, nullptr);
}

auto Element::intersectsArea(double x, double y, double width, double height) -> bool
{
	double dest_x = NAN, dest_y = NAN;
	double dest_w = NAN, dest_h = NAN;

	dest_x = std::max(getX(), x);
	dest_y = std::max(getY(), y);
	dest_w = std::min(getX() + getElementWidth(), x + width) - dest_x;
	dest_h = std::min(getY() + getElementHeight(), y + height) - dest_y;

	return (dest_w > 0 && dest_h > 0);
}

auto Element::isInSelection(ShapeContainer* container) -> bool
{
	if (!container->contains(getX(), getY()))
	{
		return false;
	}
	if (!container->contains(getX() + getElementWidth(), getY()))
	{
		return false;
	}
	if (!container->contains(getX(), getY() + getElementHeight()))
	{
		return false;
	}
	if (!container->contains(getX() + getElementWidth(), getY() + getElementHeight()))
	{
		return false;
	}

	return true;
}

auto Element::rescaleOnlyAspectRatio() -> bool
{
	return false;
}

void Element::serializeElement(ObjectOutputStream& out) const
{
	out.writeObject("Element");

	out.writeDouble(this->x);
	out.writeDouble(this->y);
	out.writeInt(this->color);

	out.endObject();
}

void Element::readSerializedElement(ObjectInputStream& in)
{
	in.readObject("Element");

	this->x = in.readDouble();
	this->y = in.readDouble();
	this->color = in.readInt();

	in.endObject();
}
