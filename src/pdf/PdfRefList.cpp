#include "PdfRefList.h"
#include "PdfRefEntry.h"
#include "PdfXRef.h"

PdfRefList::PdfRefList(PdfXRef * xref, PdfObjectWriter * objectWriter, PdfWriter * writer, char * type) {
	this->id = 1;
	this->data = NULL;
	this->xref = xref;
	this->objectWriter = objectWriter;
	this->writer = writer;
	this->type = type;
}

PdfRefList::~PdfRefList() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * entry = (PdfRefEntry *) l->data;
		delete entry;
	}
	g_list_free(this->data);
	g_free(this->type);
	this->type = NULL;
}

void PdfRefList::writeObjects() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * ref = (PdfRefEntry *) l->data;

		if (ref->type == PDF_REF_ENTRY_TYPE_REF) {
			this->xref->setXref(ref->objectId, this->writer->getDataCount());
			this->writer->writef("%i 0 obj\n", ref->objectId);
			this->objectWriter->writeObject(ref->object, ref->doc);
			this->writer->write("\nendobj\n");
		} else if (ref->type == PDF_REF_ENTRY_TYPE_DICT) {
			// nothing to do...
		} else {
			g_warning("PdfRefList::writeObjects: object type unknown: %i", ref->object->getType());
		}
	}
}

void PdfRefList::deletePdfRefList(PdfRefList * ref) {
	delete ref;
}

void PdfRefList::parse(Dict * dict, int index, XojPopplerDocument doc, GList * &replacementList) {
	Object o;
	dict->getVal(index, &o);

	if (o.isArray() && strcmp("ProcSet", dict->getKey(index)) == 0) {
		return;
	}

	if (!o.isDict()) {
		g_warning("PdfRefList::parse \"%s\" has type: %i\n", dict->getKey(index), o.getType());
		return;
	}

	Dict * dataDict = o.getDict();
	for (int u = 0; u < dataDict->getLength(); u++) {
		Object contentsObjectRef;
		Object * contentsObject = new Object();

		dataDict->getVal(u, contentsObject);
		dataDict->getValNF(u, &contentsObjectRef);

		PdfRefEntry * refEntry = NULL;
		int id = -1;
		if (contentsObjectRef.isRef()) {
			id = this->id++;
			refEntry = new PdfRefEntry(PDF_REF_ENTRY_TYPE_REF, this->writer->getNextObjectId(), contentsObject, id, doc);
			this->xref->addXref(0);
		} else if (contentsObjectRef.isDict()) {
			id = this->id++;
			refEntry = new PdfRefEntry(PDF_REF_ENTRY_TYPE_DICT, 0, contentsObject, id, doc);
		} else {
			g_warning("PdfRefList::parse type not handled, type ID = %i\n", contentsObjectRef.getType());

			contentsObject->free();
			delete contentsObject;
		}

		if (refEntry) {
			this->data = g_list_append(this->data, refEntry);

			RefReplacement * replacement = new RefReplacement(dataDict->getKey(u), id, this->type);
			replacementList = g_list_append(replacementList, replacement);
		}

		contentsObjectRef.free();
	}

	o.free();
}

void PdfRefList::writeRefList(const char * type) {
	if (!this->data) {
		return;
	}

	this->writer->writef("/%s <<\n", type);

	for (GList * l = this->data; l != NULL; l = l->next) {
		PdfRefEntry * ref = (PdfRefEntry *) l->data;

		if (ref->type == PDF_REF_ENTRY_TYPE_REF) {
			this->writer->writef("/%s%i %i 0 R\n", this->type, ref->refSourceId, ref->objectId);
		} else if (ref->type == PDF_REF_ENTRY_TYPE_DICT) {
			this->writer->writef("/%s%i ", this->type, ref->refSourceId);
			this->objectWriter->writeDictionnary(ref->object->getDict(), ref->doc);
		} else {
			g_warning("PdfRefList::writeObjects: object type unknown: %i", ref->object->getType());
		}
	}

	this->writer->write(">>\n");
}

RefReplacement::RefReplacement(String name, int newId, const char * type) {
	this->name = name;
	this->newId = newId;
	this->type = g_strdup(type);
}

RefReplacement::~RefReplacement() {
	g_free(this->type);
}

