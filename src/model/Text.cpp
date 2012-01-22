#include "Text.h"

// Hack: Needed to calculate the view size
#include "../view/TextView.h"

#include <serializing/ObjectOutputStream.h>
#include <serializing/ObjectInputStream.h>
#include <Stacktrace.h>

Text::Text() :
	Element(ELEMENT_TEXT) {
	XOJ_INIT_TYPE(Text);

	this->font.setName("Sans");
	this->font.setSize(12);
	this->inEditing = false;
}

Text::~Text() {
	XOJ_RELEASE_TYPE(Text);
}

Element * Text::clone() {
	XOJ_CHECK_TYPE(Text);

	Text * text = new Text();
	text->font = this->font;
	text->text = this->text;
	text->setColor(this->getColor());
	text->x = this->x;
	text->y = this->y;

	return text;
}


XojFont & Text::getFont() {
	XOJ_CHECK_TYPE(Text);

	return font;
}

void Text::setFont(XojFont & font) {
	XOJ_CHECK_TYPE(Text);

	this->font = font;
}

String Text::getText() {
	XOJ_CHECK_TYPE(Text);

	return this->text;
}

void Text::setText(String text) {
	XOJ_CHECK_TYPE(Text);

	this->text = text;

	calcSize();
}

void Text::calcSize() {
	XOJ_CHECK_TYPE(Text);

	TextView::calcSize(this, this->width, this->height);
}

void Text::setWidth(double width) {
	XOJ_CHECK_TYPE(Text);

	this->width = width;
}

void Text::setHeight(double height) {
	XOJ_CHECK_TYPE(Text);

	this->height = height;
}

void Text::setInEditing(bool inEditing) {
	XOJ_CHECK_TYPE(Text);

	this->inEditing = inEditing;
}

void Text::scale(double x0, double y0, double fx, double fy) {
	XOJ_CHECK_TYPE(Text);

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
	XOJ_CHECK_TYPE(Text);

	return this->inEditing;
}

bool Text::rescaleOnlyAspectRatio() {
	XOJ_CHECK_TYPE(Text);

	return true;
}

void Text::serialize(ObjectOutputStream & out) {
	XOJ_CHECK_TYPE(Text);

	out.writeObject("Text");

	serializeElement(out);

	out.writeString(this->text);

	font.serialize(out);

	out.endObject();
}

void Text::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	XOJ_CHECK_TYPE(Text);

	in.readObject("Text");

	readSerializedElement(in);

	this->text = in.readString();

	font.readSerialized(in);

	in.endObject();
}

