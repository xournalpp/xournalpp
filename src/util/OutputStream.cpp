#include "OutputStream.h"
#include <stdlib.h>
#include <string.h>

OutputStream::OutputStream() {
}

OutputStream::~OutputStream() {
}

void OutputStream::write(const String & str) {
	write(str.c_str(), str.size());
}

void OutputStream::write(const char * str) {
	write(str, strlen(str));
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

GzOutputStream::~GzOutputStream() {
	if (this->fp) {
		close();
	}
}

String & GzOutputStream::getLastError() {
	return this->error;
}

void GzOutputStream::write(const char * data, int len) {
	gzwrite(this->fp, data, len);
}

void GzOutputStream::close() {
	if (this->fp) {
		gzclose(this->fp);
		this->fp = NULL;
	}
}

