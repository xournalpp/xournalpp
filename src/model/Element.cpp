#include "Element.h"
#include "../util/ObjectStream.h"

Element::Element(ElementType type) {
	this->type = type;
	this->x = 0;
	this->y = 0;
	this->color = 0;
	this->width = 0;
	this->height = 0;

	this->sizeCalculated = false;
}

Element::~Element() {
}

ElementType Element::getType() const {
	return type;
}

void Element::setX(double x) {
	this->x = x;
}

void Element::setY(double y) {
	this->y = y;
}

double Element::getX() {
	if (!sizeCalculated) {
		sizeCalculated = true;
		calcSize();
	}
	return x;
}

double Element::getY() {
	if (!sizeCalculated) {
		sizeCalculated = true;
		calcSize();
	}
	return y;
}

void Element::move(double dx, double dy) {
	this->x += dx;
	this->y += dy;
}

double Element::getElementWidth() {
	if (!sizeCalculated) {
		sizeCalculated = true;
		calcSize();
	}
	return width;
}

double Element::getElementHeight() {
	if (!sizeCalculated) {
		sizeCalculated = true;
		calcSize();
	}
	return height;
}

void Element::setColor(int color) {
	this->color = color;
}

int Element::getColor() const {
	return color;
}

bool Element::intersectsArea(const GdkRectangle * src) {
	GdkRectangle rect = { getX(), getY(), getElementWidth(), getElementHeight() };

	return gdk_rectangle_intersect(src, &rect, NULL);
}

bool Element::intersectsArea(double x, double y, double width, double height) {
	double dest_x, dest_y;
	double dest_w, dest_h;

	dest_x = MAX(getX(), x);
	dest_y = MAX(getY(), y);
	dest_w = MIN(getX() + getElementWidth(), x + width) - dest_x;
	dest_h = MIN (getY() + getElementHeight(),y + height) - dest_y;

	return (dest_w > 0 && dest_h > 0);
}

bool Element::isInSelection(ShapeContainer * container) {
	if (!container->contains(getX(), getY())) {
		return false;
	}
	if (!container->contains(getX() + getElementWidth(), getY())) {
		return false;
	}
	if (!container->contains(getX(), getY() + getElementHeight())) {
		return false;
	}
	if (!container->contains(getX() + getElementWidth(), getY() + getElementHeight())) {
		return false;
	}

	return true;
}

bool Element::rescaleOnlyAspectRatio() {
	return false;
}

void Element::serializeElement(ObjectOutputStream & out) {
	out.writeObject("Element");

	out.writeDouble(this->x);
	out.writeDouble(this->y);
	out.writeInt(this->color);

	out.endObject();
}

void Element::readSerializedElement(ObjectInputStream & in) throw (InputStreamException) {
	in.readObject("Element");

	this->x = in.readDouble();
	this->y = in.readDouble();
	this->color = in.readInt();

	in.endObject();
}

