#include "Text.h"

// Hack: Needed to calculate the view size
#include "../view/TextView.h"
#include "../util/ObjectStream.h"

Text::Text() :
	Element(ELEMENT_TEXT) {
	this->font.setName("Sans");
	this->font.setSize(12);
	this->inEditing = false;
}

Text::~Text() {
}

XojFont & Text::getFont() {
	return font;
}

void Text::setFont(XojFont & font) {
	this->font = font;
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

bool Text::rescaleOnlyAspectRatio() {
	return true;
}

void Text::serialize(ObjectOutputStream & out) {
	out.writeObject("Text");

	serializeElement(out);

	out.writeString(this->text);

	font.serialize(out);

	out.endObject();
}

void Text::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	in.readObject("Text");

	readSerialized(in);

	this->text = in.readString();

	font.readSerialized(in);

	in.endObject();
}


