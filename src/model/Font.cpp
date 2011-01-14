#include "Font.h"

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

