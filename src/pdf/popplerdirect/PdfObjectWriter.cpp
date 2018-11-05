#include "PdfObjectWriter.h"

#include "UpdateRef.h"
#include "UpdateRefKey.h"

#include <iostream>
using std::cout;
using std::endl;

PdfObjectWriter::PdfObjectWriter(PdfWriter* writer, PdfXRef* xref)
{
	XOJ_INIT_TYPE(PdfObjectWriter);

	this->writer = writer;
	this->xref = xref;

	this->updatedReferenced = g_hash_table_new_full((GHashFunc) UpdateRefKey::hashFunction,
												    (GEqualFunc) UpdateRefKey::equalFunction,
													(GDestroyNotify) UpdateRefKey::destroyDelete,
													(GDestroyNotify) UpdateRef::destroyDelete);
}

PdfObjectWriter::~PdfObjectWriter()
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	this->writer = NULL;
	this->xref = NULL;

	g_hash_table_destroy(this->updatedReferenced);
	this->updatedReferenced = NULL;

	XOJ_RELEASE_TYPE(PdfObjectWriter);
}

void PdfObjectWriter::writeCopiedObjects()
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	bool allWritten = false;
	while (!allWritten)
	{
		allWritten = true;
		for (GList* l = g_hash_table_get_values(this->updatedReferenced); l != NULL; l = l->next)
		{
			UpdateRef* uref = (UpdateRef*) l->data;
			if (!uref->wroteOut)
			{
				this->xref->setXref(uref->objectId, this->writer->getDataCount());
				this->writer->write(FORMAT("%i 0 obj\n", uref->objectId));

				writeObject(&uref->object, uref->doc);
				this->writer->write("endobj\n");
				uref->wroteOut = true;
				break;
			}
		}
		for (GList* l = g_hash_table_get_values(this->updatedReferenced); l != NULL; l = l->next)
		{
			UpdateRef* uref = (UpdateRef*) l->data;
			if (!uref->wroteOut)
			{
				allWritten = false;
			}
		}
	}
}

void PdfObjectWriter::writeObject(Object* obj, XojPopplerDocument doc)
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	Array* array;
	Object obj1;

	switch (obj->getType())
	{
	case objBool:
		this->writer->write(FORMAT("%s ", (obj->getBool() ? "true" : "false")));
		break;
	case objInt:
		this->writer->write(FORMAT("%i ", obj->getInt()));
		break;
	case objReal:
		this->writer->write(FORMAT("%g ", obj->getReal()));
		break;
	case objString:
		this->writeString(obj->getString());
		break;
	case objName:
	{
		GooString name(obj->getName());
		GooString* nameToPrint = name.sanitizedName(gFalse /* non ps mode */);
		this->writer->write(FORMAT("/%s ", nameToPrint->getCString()));
		delete nameToPrint;
		break;
	}
	case objNull:
		this->writer->write("null");
		break;
	case objArray:
		array = obj->getArray();
		this->writer->write("[");
		for (int i = 0; i < array->getLength(); i++)
		{
			obj1 = array->getNF(i);
			writeObject(&obj1, doc);
		}
		this->writer->write("] ");
		break;
	case objDict:
		writeDictionnary(obj->getDict(), doc);
		break;
	case objStream:
	{
		// Poppler: We can't modify stream with the current implementation (no write functions in Stream API)
		// Poppler: => the only type of streams which that have been modified are internal streams (=strWeird)
		Stream* stream = obj->getStream();
		if (stream->getKind() == strWeird)
		{
			//we write the stream unencoded
			stream->reset();
			//recalculate stream length
			int tmp = 0;
			for (int c = stream->getChar(); c != EOF; c = stream->getChar())
			{
				tmp++;
			}
			stream->getDict()->set("Length", Object(tmp));

			//Remove Stream encoding
			stream->getDict()->remove("Filter");
			stream->getDict()->remove("DecodeParms");

			writeDictionnary(stream->getDict(), doc);
			writeStream(stream);
		}
		else
		{
			//raw stream copy
			writeDictionnary(stream->getDict(), doc);
			writeRawStream(stream, doc);
		}
		break;
	}
	case objRef:
	{
		UpdateRefKey key(obj->getRef(), doc);
		UpdateRef* uref = (UpdateRef*) g_hash_table_lookup(this->updatedReferenced, &key);
		if (uref)
		{
			this->writer->write(FORMAT("%i %i R ", uref->objectId, 0));
		}
		else
		{
			UpdateRef* uref = new UpdateRef(this->writer->getNextObjectId(), doc);
			this->xref->addXref(0);
			this->writer->write(FORMAT("%i %i R ", uref->objectId, 0));

			uref->object = obj->fetch(doc.getDoc()->getXRef());

			g_hash_table_insert(this->updatedReferenced, new UpdateRefKey(obj->getRef(), doc), uref);
		}
	}
		break;
	case objCmd:
		this->writer->write("cmd\r\n");
		break;
	case objError:
		this->writer->write("error\r\n");
		break;
	case objEOF:
		this->writer->write("eof\r\n");
		break;
	case objNone:
		this->writer->write("none\r\n");
		break;
	default:
		g_error("Unhandled objType : %i, please report a bug with a testcase\r\n", obj->getType());
		break;
	}
}

void PdfObjectWriter::writeRawStream(Stream* str, XojPopplerDocument doc)
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	Object obj1 = str->getDict()->lookup("Length");
	if (!obj1.isInt())
	{
		cout << "PDFDoc::writeRawStream, no Length in stream dict";
		return;
	}

	const int length = obj1.getInt();

	this->writer->write("stream\r\n");
	str->unfilteredReset();

	char buffer[1];

	for (int i = 0; i < length; i++)
	{
		int c = str->getUnfilteredChar();
		buffer[0] = c;
		this->writer->write(string(buffer, 1));
	}
	str->reset();
	this->writer->write("\nendstream\n");
}

void PdfObjectWriter::writeStream(Stream* str)
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	this->writer->write("stream\r\n");
	str->reset();
	for (int c = str->getChar(); c != EOF; c = str->getChar())
	{
		this->writer->write(FORMAT("%c", c));
	}
	this->writer->write("\r\nendstream\r\n");
}

void PdfObjectWriter::writeDictionnary(Dict* dict, XojPopplerDocument doc)
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	Object obj;
	this->writer->write("<<");
	for (int i = 0; i < dict->getLength(); i++)
	{
		GooString keyName(dict->getKey(i));
		GooString* keyNameToPrint = keyName.sanitizedName(gFalse /* non ps mode */);
		this->writer->write(FORMAT("/%s ", keyNameToPrint->getCString()));
		delete keyNameToPrint;
		obj = Object(dict->getValNF(i));
		writeObject(&obj, doc);
	}
	this->writer->write(">> ");
}

void PdfObjectWriter::writeString(const GooString* s)
{
	XOJ_CHECK_TYPE(PdfObjectWriter);

	if (s->hasUnicodeMarker())
	{
		//unicode string don't necessary end with \0
		const char* c = s->getCString();
		this->writer->write("(");
		for (int i = 0; i < s->getLength(); i++)
		{
			char unescaped = *(c + i) & 0x000000ff;
			//escape if needed
			if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
			{
				this->writer->write("\\");
			}
			this->writer->write(FORMAT("%c", unescaped));
		}
		this->writer->write(") ");
	}
	else
	{
		const char* c = s->getCString();
		this->writer->write("(");
		while (*c != '\0')
		{
			char unescaped = (*c) & 0x000000ff;
			//escape if needed
			if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
			{
				this->writer->write("\\");
			}
			this->writer->write(FORMAT("%c", unescaped));
			c++;
		}
		this->writer->write(") ");
	}
}
