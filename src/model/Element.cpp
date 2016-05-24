#include "Element.h"

#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>

Element::Element(ElementType type)
{
	XOJ_INIT_TYPE(Element);

	this->type = type;
	this->x = 0;
	this->y = 0;
	this->color = 0;
	this->width = 0;
	this->height = 0;

	this->sizeCalculated = false;
}

Element::~Element()
{
	XOJ_RELEASE_TYPE(Element);
}

ElementType Element::getType() const
{
	XOJ_CHECK_TYPE(Element);

	return this->type;
}

void Element::setX(double x)
{
	XOJ_CHECK_TYPE(Element);

	this->x = x;
}

void Element::setY(double y)
{
	XOJ_CHECK_TYPE(Element);

	this->y = y;
}

double Element::getX()
{
	XOJ_CHECK_TYPE(Element);

	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return x;
}

double Element::getY()
{
	XOJ_CHECK_TYPE(Element);

	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return y;
}

void Element::move(double dx, double dy)
{
	XOJ_CHECK_TYPE(Element);

	this->x += dx;
	this->y += dy;
}

double Element::getElementWidth()
{
	XOJ_CHECK_TYPE(Element);

	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->width;
}

double Element::getElementHeight()
{
	XOJ_CHECK_TYPE(Element);

	if (!this->sizeCalculated)
	{
		this->sizeCalculated = true;
		calcSize();
	}
	return this->height;
}

void Element::setColor(int color)
{
	XOJ_CHECK_TYPE(Element);

	this->color = color;
}

int Element::getColor() const
{
	XOJ_CHECK_TYPE(Element);

	return this->color;
}

bool Element::intersectsArea(const GdkRectangle* src)
{
	XOJ_CHECK_TYPE(Element);

	GdkRectangle rect = {
		gint(getX()),
		gint(getY()),
		gint(getElementWidth()),
		gint(getElementHeight())
	};

	return gdk_rectangle_intersect(src, &rect, NULL);
}

bool Element::intersectsArea(double x, double y, double width, double height)
{
	XOJ_CHECK_TYPE(Element);

	double dest_x, dest_y;
	double dest_w, dest_h;

	dest_x = MAX(getX(), x);
	dest_y = MAX(getY(), y);
	dest_w = MIN(getX() + getElementWidth(), x + width) - dest_x;
	dest_h = MIN(getY() + getElementHeight(), y + height) - dest_y;

	return (dest_w > 0 && dest_h > 0);
}

bool Element::isInSelection(ShapeContainer* container)
{
	XOJ_CHECK_TYPE(Element);

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
	XOJ_CHECK_TYPE(Element);

	return false;
}

void Element::serializeElement(ObjectOutputStream& out)
{
	XOJ_CHECK_TYPE(Element);

	out.writeObject("Element");

	out.writeDouble(this->x);
	out.writeDouble(this->y);
	out.writeInt(this->color);

	out.endObject();
}

void Element::readSerializedElement(ObjectInputStream& in) throw (InputStreamException)
{
	XOJ_CHECK_TYPE(Element);

	in.readObject("Element");

	this->x = in.readDouble();
	this->y = in.readDouble();
	this->color = in.readInt();

	in.endObject();
}
