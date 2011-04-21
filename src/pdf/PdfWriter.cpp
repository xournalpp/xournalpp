#include "PdfWriter.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <config.h>

#include "../util/GzHelper.h"

bool PdfWriter::compressPdfOutput = true;

PdfWriter::PdfWriter(PdfXRef * xref) {
	XOJ_INIT_TYPE(PdfWriter);

	this->inStream = false;
	this->stream = NULL;
	this->out = NULL;
	this->dataCount = 0;
	this->xref = xref;
	this->objectId = 3;
}

PdfWriter::~PdfWriter() {
	XOJ_CHECK_TYPE(PdfWriter);

	if (this->stream) {
		g_string_free(this->stream, true);
	}
	this->stream = NULL;

	this->xref = NULL;

	XOJ_RELEASE_TYPE(PdfWriter);
}

void PdfWriter::setCompressPdfOutput(bool compress) {
	PdfWriter::compressPdfOutput = compress;
}

void PdfWriter::close() {
	XOJ_CHECK_TYPE(PdfWriter);

	g_output_stream_close(G_OUTPUT_STREAM(this->out), NULL, NULL);
}

bool PdfWriter::openFile(const char * uri) {
	XOJ_CHECK_TYPE(PdfWriter);

	GError * error = NULL;

	GFile * file = g_file_new_for_uri(uri);
	this->out = g_file_replace(file, NULL, false, (GFileCreateFlags) 0, NULL, &error);

	g_object_unref(file);

	if (error) {
		lastError = "Error opening file for writing: ";
		lastError += error->message;
		lastError += ", File: ";
		lastError += uri;
		g_warning("error opening file");
		return false;
	}
	return true;
}

bool PdfWriter::write(const char * data) {
	XOJ_CHECK_TYPE(PdfWriter);

	return writeLen(data, strlen(data));
}

bool PdfWriter::writef(const char * format, ...) {
	XOJ_CHECK_TYPE(PdfWriter);

	va_list args;
	va_start(args, format);
	char * data = g_strdup_vprintf(format, args);
	bool res = writeLen(data, strlen(data));
	g_free(data);
	return res;
}

bool PdfWriter::write(int data) {
	XOJ_CHECK_TYPE(PdfWriter);

	return writef("%i", data);;
}

String PdfWriter::getLastError() {
	XOJ_CHECK_TYPE(PdfWriter);

	return lastError;
}

int PdfWriter::getObjectId() {
	XOJ_CHECK_TYPE(PdfWriter);

	return this->objectId;
}

int PdfWriter::getNextObjectId() {
	XOJ_CHECK_TYPE(PdfWriter);

	return this->objectId++;
}

int PdfWriter::getDataCount() {
	XOJ_CHECK_TYPE(PdfWriter);

	return this->dataCount;
}

bool PdfWriter::writeTxt(const char * data) {
	XOJ_CHECK_TYPE(PdfWriter);

	GString * str = g_string_sized_new(strlen(data) + 100);
	g_string_append(str, "(");

	while (*data) {
		if (*data == '\\' || *data == '(' || *data == ')' || *data == '\r') {
			g_string_append_c(str, '\\');
		}
		g_string_append_c(str, *data);
		data++;
	}

	g_string_append_c(str, ')');
	return writeLen(str->str, str->len);

	g_string_free(str, true);
}

bool PdfWriter::writeLen(const char * data, int len) {
	XOJ_CHECK_TYPE(PdfWriter);

	if (this->inStream) {
		g_string_append_len(this->stream, data, len);
		return true;
	}

	GError * err = NULL;

	g_output_stream_write(G_OUTPUT_STREAM(this->out), data, len, NULL, &err);

	this->dataCount += len;

	if (err) {
		this->lastError = "Error writing stream: ";
		this->lastError += err->message;

		printf("error writing file: %s\n", err->message);
		g_error_free(err);
		return false;
	}

	return true;
}

bool PdfWriter::writeObj() {
	XOJ_CHECK_TYPE(PdfWriter);

	this->xref->addXref(this->dataCount);
	bool res = this->writef("%i 0 obj\n", this->objectId++);
	if (!res) {
		this->lastError = "Internal PDF error #8";
	}

	return res;
}

bool PdfWriter::writeInfo(String title) {
	XOJ_CHECK_TYPE(PdfWriter);

	if (!writeObj()) {
		return false;
	}

	write("<<\n");

	write("/Producer ");
	writeTxt("Xournal++");
	write("\n");

	if (!title.isEmpty()) {
		write("/Title ");
		if (title.length() > 4 && title.substring(-4, 1) == ".") {
			title = title.substring(0, -4);
		}
		writeTxt(title.c_str());
		write("\n");
	}

	const char * username = getenv("USERNAME");
	if(username) {
		write("/Author ");
		writeTxt();
		write("\n");
	}

	write("/Creator ");
	writeTxt("Cairo / Poppler " VERSION);
	write("\n");

	time_t curtime = time(NULL);
	char stime[128] = "D:";
	strftime(stime + 2, sizeof(stime) - 2, "%Y%m%d%H%M%S", localtime(&curtime));

	write("/CreationDate ");
	writeTxt(stime);
	write("\n");

	write(">>\nendobj\n");

	return this->lastError.isEmpty();
}

void PdfWriter::startStream() {
	XOJ_CHECK_TYPE(PdfWriter);

	this->inStream = true;
	if (this->stream == NULL) {
		this->stream = g_string_new("");
	}
}

void PdfWriter::endStream() {
	XOJ_CHECK_TYPE(PdfWriter);

	this->inStream = false;

	GString * data = NULL;
	GString * compressed = NULL;

	if (PdfWriter::compressPdfOutput) {
		compressed = GzHelper::gzcompress(this->stream);
	}

	const char * filter = "";
	if (compressed) {
		filter = "/Filter /FlateDecode ";
		data = compressed;
	} else {
		data = this->stream;
	}

	writef("<<%s/Length %i>>\n", filter, data->len);
	write("\nstream\n");

	writeLen(data->str, data->len);

	write("\nendstream\n");

	if (compressed) {
		g_string_free(compressed, true);
	}

	this->stream->len = 0;
}
