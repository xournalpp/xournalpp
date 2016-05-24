#include "PdfXRef.h"

#include <glib.h>

PdfXRef::PdfXRef()
{
	XOJ_INIT_TYPE(PdfXRef);

	this->xref = NULL;
	this->xrefLenght = 0;
	this->xrefNr = 0;
}

PdfXRef::~PdfXRef()
{
	XOJ_CHECK_TYPE(PdfXRef);

	g_free(this->xref);
	this->xref = NULL;
	this->xrefLenght = 0;

	XOJ_RELEASE_TYPE(PdfXRef);
}

void PdfXRef::addXref(int ref)
{
	XOJ_CHECK_TYPE(PdfXRef);

	if (this->xrefLenght <= this->xrefNr + 1)
	{
		this->xrefLenght += 100;
		this->xref = (int*) g_realloc(this->xref, this->xrefLenght * sizeof(int));
	}

	this->xref[this->xrefNr++] = ref;
}

void PdfXRef::setXref(int id, int ref)
{
	XOJ_CHECK_TYPE(PdfXRef);

	if (id < 1)
	{
		g_warning("PdfXRef::setXref try to set XREF-ID: %i it needs to be at least 1", id);
		return;
	}
	if (id - 1 >= this->xrefNr)
	{
		g_warning("PdfXRef::setXref try to set XREF-ID: %i but there are only %i", id, this->xrefNr);
		return;
	}
	this->xref[id - 1] = ref;
}

int PdfXRef::getXrefCount()
{
	XOJ_CHECK_TYPE(PdfXRef);

	return this->xrefNr;
}

int PdfXRef::getXref(int id)
{
	XOJ_CHECK_TYPE(PdfXRef);

	return this->xref[id];
}
