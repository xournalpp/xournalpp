#include "PdfRefList.h"
#include "PdfRefEntry.h"
#include "PdfXRef.h"

PdfRefList::PdfRefList(PdfXRef * xref, PdfObjectWriter * objectWriter, PdfWriter * writer, const char * type) {
	this->id = 1;
	this->data = NULL;
	this->xref = xref;
	this->objectWriter = objectWriter;
	this->writer = writer;
	this->type = type;
}

PdfRefList::~PdfRefList() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * pattern = (PdfRefEntry *) l->data;
		delete pattern;
	}
	g_list_free(this->data);
}

void PdfRefList::writeObjects() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * ref = (PdfRefEntry *) l->data;

		this->xref->setXref(ref->objectId, this->writer->getDataCount());
		bool res = this->writer->writef("%i 0 obj\n", ref->objectId);

		this->objectWriter->writeObject(ref->object, ref->doc);
		this->writer->write("\nendobj\n");
	}
}

int PdfRefList::lookup(String name, Ref ref, Object * object, XojPopplerDocument doc) {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * p = (PdfRefEntry *) l->data;

		if (p->equalsRef(ref)) {
			object->free();
			delete object;
			return p->objectId;
		}
	}

	int id = this->id++;

	PdfRefEntry * pattern = new PdfRefEntry(this->writer->getNextObjectId(), ref, object, id, doc);
	this->xref->addXref(0);
	this->data = g_list_append(this->data, pattern);

	return id;
}

bool PdfRefList::writeRefList() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * img = (PdfRefEntry *) l->data;
		this->writer->writef("/%s%i %i 0 R\n", this->type, img->imageId, img->objectId);
	}
	return true;
}

