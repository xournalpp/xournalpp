#include "Element.h"

//#define MAX(x, y) ( x > y ) ? x : y
//#define MIN(x, y) ( x < y ) ? x : y

Element::Element(ElementType type) {
	this->type = type;
	x = 0;
	y = 0;
	color = 0;
	width = 0;
	height = 0;

	sizeCalculated = false;
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

void Element::finalizeMove() {
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

