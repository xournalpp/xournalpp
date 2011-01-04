#include "XFont.h"

XFont::XFont() {
	this->size = 0;
	this->bold = false;
	this->italic = false;
}

XFont::~XFont() {
}

String XFont::getName() {
	return name;
}

void XFont::setName(String name) {
	this->name = name;
}

bool XFont::isItalic() {
	return italic;
}

bool XFont::isBold() {
	return bold;
}

void XFont::setItalic(bool italic) {
	this->italic = italic;
}

void XFont::setBold(bool bold) {
	this->bold = bold;
}

double XFont::getSize() {
	return size;
}

void XFont::setSize(double size) {
	this->size = size;
}

