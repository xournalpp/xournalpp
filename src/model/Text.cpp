#include "Text.h"

// Hack: Needed to calculate the view size
#include "../view/TextView.h"

#include "../util/ObjectStream.h"
#include "../util/Stacktrace.h"
// TODO: AA: type check

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

void Text::scale(double x0, double y0, double fx, double fy) {
	// only proportional scale allowed...
	if (fx != fy) {
		g_warning("rescale font with fx != fy not supported: %lf / %lf", fx, fy);
		Stacktrace::printStracktrace();
	}

	this->x -= x0;
	this->x *= fx;
	this->x += x0;
	this->y -= y0;
	this->y *= fy;
	this->y += y0;

	double size = this->font.getSize() * fx;
	this->font.setSize(size);

	this->sizeCalculated = false;
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

