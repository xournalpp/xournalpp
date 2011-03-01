#include "PdfRefEntry.h"

PdfRefEntry::PdfRefEntry(int objectId, Ref ref, Object * object, int imageId, XojPopplerDocument doc) {
	this->objectId = objectId;
	this->ref = ref;
	this->imageId = imageId;
	this->doc = doc;
	this->object = object;
}

PdfRefEntry::~PdfRefEntry() {
	if (this->object) {
		delete this->object;
	}
	this->object = NULL;
}

bool PdfRefEntry::equalsRef(const Ref & ref) {
	return (this->ref.gen == ref.gen && this->ref.num == ref.num);
}
