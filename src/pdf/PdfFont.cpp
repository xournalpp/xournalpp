#include "PdfFont.h"

PdfFont::PdfFont(XojPopplerDocument & doc, String originalName, int id, Object * object) {
	this->doc = doc;
	this->originalName = originalName;
	this->id = id;
	this->objectId = -1;
	this->object = object;
}

PdfFont::~PdfFont() {
	if (this->object) {
		delete this->object;
	}
	this->object = NULL;
}
