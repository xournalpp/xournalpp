#include "Element.h"

#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

Element::Element(ElementType type)
 : type(type)
{
}

Element::~Element()
{
}

ElementType Element::getType() const
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

double Element::getX()
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return x;
}

double Element::getY()
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

double Element::getElementWidth()
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->width;
}

double Element::getElementHeight()
{
	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->height;
}

Rectangle Element::boundingRect()
{
	return Rectangle(getX(), getY(), getElementWidth(), getElementHeight());
}

void Element::setColor(int color)
{
	this->color = color;
}

int Element::getColor() const
{
	return this->color;
}

bool Element::intersectsArea(const GdkRectangle* src)
{
	GdkRectangle rect = {
		gint(getX()),
		gint(getY()),
		gint(getElementWidth()),
		gint(getElementHeight())
	};

	return gdk_rectangle_intersect(src, &rect, nullptr);
}

bool Element::intersectsArea(double x, double y, double width, double height)
{
	double dest_x, dest_y;
	double dest_w, dest_h;

	dest_x = std::max(getX(), x);
	dest_y = std::max(getY(), y);
	dest_w = std::min(getX() + getElementWidth(), x + width) - dest_x;
	dest_h = std::min(getY() + getElementHeight(), y + height) - dest_y;

	return (dest_w > 0 && dest_h > 0);
}

bool Element::isInSelection(ShapeContainer* container)
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

bool Element::rescaleOnlyAspectRatio()
{
	return false;
}

void Element::serializeElement(ObjectOutputStream& out)
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
