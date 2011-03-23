#include "PdfRefEntry.h"
// TODO: AA: type check

PdfRefEntry::PdfRefEntry(PdfRefEntryType type, int objectId, Object * object, int refSourceId, Ref ref, XojPopplerDocument doc) {
	this->type = type;
	this->objectId = objectId;
	this->refSourceId = refSourceId;
	this->doc = doc;
	this->object = object;
	this->ref = ref;
	this->used = false;
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

void PdfRefEntry::markAsUsed() {
	this->used = true;
}

bool PdfRefEntry::isUsed() {
	return this->used;
}

