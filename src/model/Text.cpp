#include "Text.h"

#include "view/TextView.h" // Hack: Needed to calculate the view size

#include <serializing/ObjectInputStream.h>
#include <serializing/ObjectOutputStream.h>
#include <Stacktrace.h>

Text::Text()
 : AudioElement(ELEMENT_TEXT)
{
	this->font.setName("Sans");
	this->font.setSize(12);
}

Text::~Text()
{
}

Element* Text::clone()
{
	Text* text = new Text();
	text->font = this->font;
	text->text = this->text;
	text->setColor(this->getColor());
	text->x = this->x;
	text->y = this->y;
	text->cloneAudioData(this);

	return text;
}

XojFont& Text::getFont()
{
	return font;
}

void Text::setFont(XojFont& font)
{
	this->font = font;
}

string Text::getText()
{
	return this->text;
}

void Text::setText(string text)
{
	this->text = text;

	calcSize();
}

void Text::calcSize()
{
	TextView::calcSize(this, this->width, this->height);
}

void Text::setWidth(double width)
{
	this->width = width;
}

void Text::setHeight(double height)
{
	this->height = height;
}

void Text::setInEditing(bool inEditing)
{
	this->inEditing = inEditing;
}

void Text::scale(double x0, double y0, double fx, double fy)
{
	// only proportional scale allowed...
	if (fx != fy)
	{
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

void Text::rotate(double x0, double y0, double xo, double yo, double th)
{
}

bool Text::isInEditing()
{
	return this->inEditing;
}

bool Text::rescaleOnlyAspectRatio()
{
	return true;
}

bool Text::intersects(double x, double y, double halfEraserSize)
{
	return intersects(x, y, halfEraserSize, nullptr);
}

bool Text::intersects(double x, double y, double halfEraserSize, double* gap)
{
	double x1 = this->x - halfEraserSize;
	double x2 = this->x + this->getElementWidth() + halfEraserSize;
	double y1 = this->y - halfEraserSize;
	double y2 = this->y + this->getElementHeight() + halfEraserSize;

	return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

void Text::serialize(ObjectOutputStream& out)
{
	out.writeObject("Text");

	serializeAudioElement(out);

	out.writeString(this->text);

	font.serialize(out);

	out.endObject();
}

void Text::readSerialized(ObjectInputStream& in)
{
	in.readObject("Text");

	readSerializedAudioElement(in);

	this->text = in.readString();

	font.readSerialized(in);

	in.endObject();
}
