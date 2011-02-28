#include "PdfXRef.h"
#include <glib.h>

PdfXRef::PdfXRef() {
	this->xref = NULL;
	this->xrefLenght = 0;
	this->xrefNr = 0;
}

PdfXRef::~PdfXRef() {
	g_free(xref);
}

void PdfXRef::addXref(int ref) {
	if (this->xrefLenght < this->xrefNr + 1) {
		this->xrefLenght += 100;
		this->xref = (int *) g_realloc(this->xref, this->xrefLenght * sizeof(int));
	}

	this->xref[this->xrefNr++] = ref;
}

void PdfXRef::setXref(int id, int ref) {
	if (id >= this->xrefNr) {
		g_warning("PdfXRef::setXref try to set XREF-ID: %i but there are only %i", id, this->xrefNr);
		return;
	}
	this->xref[id] = ref;
}

int PdfXRef::getXrefCount() {
	return this->xrefNr;
}

int PdfXRef::getXref(int id) {
	return this->xref[id];
}

