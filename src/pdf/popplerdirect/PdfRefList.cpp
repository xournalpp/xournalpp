#include "PdfRefList.h"

#include "PdfRefEntry.h"
#include "PdfXRef.h"

PdfRefList::PdfRefList(PdfXRef* xref, PdfObjectWriter* objectWriter, PdfWriter* writer, char* type)
{
	XOJ_INIT_TYPE(PdfRefList);

	this->id = 1;
	this->data = NULL;
	this->xref = xref;
	this->objectWriter = objectWriter;
	this->writer = writer;
	this->type = type;
}

PdfRefList::~PdfRefList()
{
	XOJ_CHECK_TYPE(PdfRefList);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfRefEntry* entry = (PdfRefEntry*) l->data;
		delete entry;
	}
	g_list_free(this->data);
	this->data = NULL;
	g_free(this->type);
	this->type = NULL;

	XOJ_RELEASE_TYPE(PdfRefList);
}

void PdfRefList::writeObjects()
{
	XOJ_CHECK_TYPE(PdfRefList);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfRefEntry* ref = (PdfRefEntry*) l->data;

		if (!ref->isUsed())
		{
			continue;
		}

		if (ref->type == PDF_REF_ENTRY_TYPE_REF)
		{
			this->xref->setXref(ref->objectId, this->writer->getDataCount());
			this->writer->write(FORMAT("%i 0 obj\n", ref->objectId));
			this->objectWriter->writeObject(ref->object, ref->doc);
			this->writer->write("\nendobj\n");
		}
		else if (ref->type == PDF_REF_ENTRY_TYPE_DICT)
		{
			// nothing to do...
		}
		else
		{
			g_warning("PdfRefList::writeObjects: object type unknown: %i", ref->object->getType());
		}
	}
}

void PdfRefList::deletePdfRefList(PdfRefList* ref)
{
	delete ref;
	ref = NULL;
}

int PdfRefList::lookup(Ref ref, Object* object, XojPopplerDocument doc, PdfRefEntry*& refEntryRet)
{
	XOJ_CHECK_TYPE(PdfRefList);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfRefEntry* p = (PdfRefEntry*) l->data;

		if (p->equalsRef(ref))
		{
			object->free();
			delete object;

			refEntryRet = p;
			return p->refSourceId;
		}
	}

	int id = this->id++;

	PdfRefEntry* refEntry = new PdfRefEntry(PDF_REF_ENTRY_TYPE_REF, this->writer->getNextObjectId(), object, id, ref, doc);
	refEntryRet = refEntry;
	this->xref->addXref(0);
	this->data = g_list_append(this->data, refEntry);

	return id;
}

void PdfRefList::parse(Dict* dict, int index, XojPopplerDocument doc, GList*& replacementList)
{
	XOJ_CHECK_TYPE(PdfRefList);

	Object o;
	dict->getVal(index, &o);

	if (o.isArray() && strcmp("ProcSet", dict->getKey(index)) == 0)
	{
		return;
	}

	if (!o.isDict())
	{
		g_warning("PdfRefList::parse \"%s\" has type: %i\n", dict->getKey(index), o.getType());
		return;
	}

	Dict* dataDict = o.getDict();
	for (int u = 0; u < dataDict->getLength(); u++)
	{
		Object contentsObjectRef;
		Object* contentsObject = new Object();

		dataDict->getVal(u, contentsObject);
		dataDict->getValNF(u, &contentsObjectRef);

		if (contentsObjectRef.isRef())
		{
			PdfRefEntry* refEntry = NULL;
			int id = lookup(contentsObjectRef.getRef(), contentsObject, doc, refEntry);

			RefReplacement* replacement = new RefReplacement(dataDict->getKey(u), id, this->type, refEntry);
			replacementList = g_list_append(replacementList, replacement);
		}
		else if (contentsObjectRef.isDict())
		{
			Ref ref = { -1, -1 };
			int id = this->id++;
			PdfRefEntry* refEntry = new PdfRefEntry(PDF_REF_ENTRY_TYPE_DICT, 0, contentsObject, id, ref, doc);
			this->data = g_list_append(this->data, refEntry);

			RefReplacement* replacement = new RefReplacement(dataDict->getKey(u), id, this->type, refEntry);
			replacementList = g_list_append(replacementList, replacement);
		}
		else
		{
			g_warning("PdfRefList::parse type not handled, type ID = %i\n", contentsObjectRef.getType());

			contentsObject->free();
			delete contentsObject;
		}

		contentsObjectRef.free();
	}

	o.free();
}

void PdfRefList::writeRefList(const char* type)
{
	XOJ_CHECK_TYPE(PdfRefList);

	if (!this->data)
	{
		return;
	}

	this->writer->write(FORMAT("/%s <<\n", type));

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		PdfRefEntry* ref = (PdfRefEntry*) l->data;

		if (!ref->isUsed())
		{
			continue;
		}

		if (ref->type == PDF_REF_ENTRY_TYPE_REF)
		{
			this->writer->write(FORMAT("/%s%i %i 0 R\n", this->type, ref->refSourceId, ref->objectId));
		}
		else if (ref->type == PDF_REF_ENTRY_TYPE_DICT)
		{
			this->writer->write(FORMAT("/%s%i ", this->type, ref->refSourceId));
			this->objectWriter->writeDictionnary(ref->object->getDict(), ref->doc);
		}
		else
		{
			g_warning("PdfRefList::writeObjects: object type unknown: %i", ref->object->getType());
		}
	}

	this->writer->write(">>\n");
}

RefReplacement::RefReplacement(string name, int newId, const char* type, PdfRefEntry* refEntry)
{
	XOJ_INIT_TYPE(RefReplacement);

	this->name = name;
	this->newId = newId;
	this->type = g_strdup(type);
	this->used = false;
	this->refEntry = refEntry;
}

RefReplacement::~RefReplacement()
{
	XOJ_CHECK_TYPE(RefReplacement);

	g_free(this->type);
	this->type = NULL;
	if (this->refEntry == NULL)
	{
		g_warning("RefReplacement::~RefReplacement(): this->refEntry == NULL");
		return;
	}
	if (this->used)
	{
		this->refEntry->markAsUsed();
	}

	XOJ_RELEASE_TYPE(RefReplacement);
}

void RefReplacement::markAsUsed()
{
	XOJ_CHECK_TYPE(RefReplacement);

	this->used = true;
}
