#include "Text.h"

// Hack: Needed to calculate the view size
#include "../view/TextView.h"

Text::Text() :
	Element(ELEMENT_TEXT) {
	this->font.setName("Sans");
	this->font.setSize(12);
	this->inEditing = false;
}

Text::~Text() {
}

XFont & Text::getFont() {
	return font;
}

String Text::getText() {
	return text;
}

void Text::setText(String text) {
	this->text = text;

	calcSize();
}

void Text::calcSize() {
	TextView::calcSize(this, this->width, this->height);
}

void Text::setWidth(double width) {
	this->width = width;
}

void Text::setHeight(double height) {
	this->height = height;
}

void Text::setInEditing(bool inEditing) {
	this->inEditing = inEditing;
}

bool Text::isInEditing() {
	return this->inEditing;
}

bool Text::isInSelection(ShapeContainer * container) {
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
