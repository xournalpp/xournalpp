#include "OutputStream.h"
#include <stdlib.h>

OutputStream::OutputStream() {
}

OutputStream::~OutputStream() {
}

void OutputStream::write(const String & str) {
	write(str.c_str());
}

////////////////////////////////////////////////////////
/// StdOutputStream ////////////////////////////////////
////////////////////////////////////////////////////////

void StdOutputStream::write(const char * data) {
	printf("%s", data);
}

void StdOutputStream::close() {
}

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(String filename) {
	this->fp = NULL;
	this->filename = filename;
	this->fp = gzopen(filename.c_str(), "w");
	if (this->fp == NULL) {
		char * e = g_strdup_printf("error opening file: \"%s\"", filename.c_str());
		this->error = e;
		g_free(e);
	}
}

String GzOutputStream::getLastError() {
	return this->error;
}

void GzOutputStream::write(const char * data) {
	gzprintf(this->fp, "%s", data);
}

void GzOutputStream::close() {
	if (this->fp) {
		gzclose(this->fp);
	}
}

