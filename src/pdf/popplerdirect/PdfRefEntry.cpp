#include "PdfRefEntry.h"

PdfRefEntry::PdfRefEntry(PdfRefEntryType type, int objectId, Object* object,
						 int refSourceId, Ref ref, XojPopplerDocument doc)
{
	XOJ_INIT_TYPE(PdfRefEntry);

	this->type = type;
	this->objectId = objectId;
	this->refSourceId = refSourceId;
	this->doc = doc;
	this->object = object;
	this->ref = ref;
	this->used = false;
}

PdfRefEntry::~PdfRefEntry()
{
	XOJ_CHECK_TYPE(PdfRefEntry);

	if (this->object)
	{
		delete this->object;
	}
	this->object = NULL;

	XOJ_RELEASE_TYPE(PdfRefEntry);
}

bool PdfRefEntry::equalsRef(const Ref& ref)
{
	XOJ_CHECK_TYPE(PdfRefEntry);

	return (this->ref.gen == ref.gen && this->ref.num == ref.num);
}

void PdfRefEntry::markAsUsed()
{
	XOJ_CHECK_TYPE(PdfRefEntry);

	this->used = true;
}

bool PdfRefEntry::isUsed()
{
	XOJ_CHECK_TYPE(PdfRefEntry);

	return this->used;
}

