#include "PdfRefEntry.h"

PdfRefEntry::PdfRefEntry(PdfRefEntryType type, int objectId, Object * object, int imageId, XojPopplerDocument doc) {
	this->type = type;
	this->objectId = objectId;
	this->refSourceId = imageId;
	this->doc = doc;
	this->object = object;
}

PdfRefEntry::~PdfRefEntry() {
	if (this->object) {
		delete this->object;
	}
	this->object = NULL;
}
