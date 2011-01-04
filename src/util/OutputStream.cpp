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
	if (filename.startsWith("file://")) {
		this->filename = filename;
		String localName = filename.substring(7);
		this->fp = gzopen(localName.c_str(), "w");
		if (this->fp == NULL) {
			char * e = g_strdup_printf("error opening file: \"%s\"", localName.c_str());
			this->error = e;
			g_free(e);
		}
	} else {
		char buffer[L_tmpnam];

		// We cannot use mkstemp because we need the name for gzopen as string
		char * tmpFile = tmpnam(buffer);
		if (!tmpFile) {
			char * e = g_strdup_printf("coult not get filename for tmp file");
			this->error = e;
			g_free(e);
			return;
		}

		this->fp = gzopen(tmpFile, "w");
		if (this->fp == NULL) {
			char * e = g_strdup_printf("error opening tmpfile: \"%s\"", tmpFile);
			this->error = e;
			g_free(e);
		}

		printf("->%s\n", tmpFile);

		this->filename = tmpFile;
		this->target = filename;
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

		if (!this->target.isEmpty()) {
			printf("should copy \"%s\" to \"%s\"\n", this->filename.c_str(), this->target.c_str());
		}
	}
}

