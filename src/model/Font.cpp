#include "Font.h"
#include "../util/ObjectStream.h"
// TODO: AA: type check

XojFont::XojFont() {
	this->size = 0;
}

XojFont::~XojFont() {
}

String XojFont::getName() {
	return name;
}

void XojFont::setName(String name) {
	this->name = name;
}

double XojFont::getSize() {
	return size;
}

void XojFont::setSize(double size) {
	this->size = size;
}

void XojFont::operator =(const XojFont & font) {
	this->name = font.name;
	this->size = font.size;
}

void XojFont::serialize(ObjectOutputStream & out) {
	out.writeObject("XojFont");

	out.writeString(this->name);
	out.writeDouble(this->size);

	out.endObject();
}

void XojFont::readSerialized(ObjectInputStream & in) throw (InputStreamException) {
	in.readObject("XojFont");

	this->name = in.readString();
	this->size = in.readDouble();

	in.endObject();
}

