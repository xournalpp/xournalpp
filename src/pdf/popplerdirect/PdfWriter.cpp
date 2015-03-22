#include "PdfWriter.h"
#include <config.h>
#include <iostream>

#include <GzHelper.h>

using namespace std;

bool PdfWriter::compressPdfOutput = true;

PdfWriter::PdfWriter(PdfXRef* xref)
{
	XOJ_INIT_TYPE(PdfWriter);

	this->inStream = false;
	this->stream = NULL;
	this->out = NULL;
	this->dataCount = 0;
	this->xref = xref;
	this->objectId = 3;
}

PdfWriter::~PdfWriter()
{
	XOJ_CHECK_TYPE(PdfWriter);

	if (this->stream)
	{
		g_string_free(this->stream, true);
	}
	this->stream = NULL;

	this->xref = NULL;

	XOJ_RELEASE_TYPE(PdfWriter);
}

void PdfWriter::setCompressPdfOutput(bool compress)
{
	PdfWriter::compressPdfOutput = compress;
}

void PdfWriter::close()
{
	XOJ_CHECK_TYPE(PdfWriter);

	g_output_stream_close(G_OUTPUT_STREAM(this->out), NULL, NULL);
}

bool PdfWriter::openFile(path filename)
{
	XOJ_CHECK_TYPE(PdfWriter);

	GError* error = NULL;

	GFile* file = g_file_new_for_path(filename.c_str());
	this->out = g_file_replace(file, NULL, false, (GFileCreateFlags) 0, NULL,
							   &error);

	g_object_unref(file);

	if (error)
	{
		lastError = (bl::format("Error opening file for writing: {1}, File: {2}")
					 % error->message % filename.string()).str();
		g_warning("error opening file");
		return false;
	}
	return true;
}

bool PdfWriter::write(string data)
{
	XOJ_CHECK_TYPE(PdfWriter);

	return writeLen(data.c_str(), data.length());
}

bool PdfWriter::writef(const char* format, ...)
{
	XOJ_CHECK_TYPE(PdfWriter);

	va_list args;
	va_start(args, format);
	char* data = g_strdup_vprintf(format, args);
	bool res = writeLen(data, strlen(data));
	g_free(data);
	return res;
}

bool PdfWriter::write(int data)
{
	XOJ_CHECK_TYPE(PdfWriter);

	return writef("%i", data);
	;
}

string PdfWriter::getLastError()
{
	XOJ_CHECK_TYPE(PdfWriter);

	return lastError;
}

int PdfWriter::getObjectId()
{
	XOJ_CHECK_TYPE(PdfWriter);

	return this->objectId;
}

int PdfWriter::getNextObjectId()
{
	XOJ_CHECK_TYPE(PdfWriter);

	return this->objectId++;
}

int PdfWriter::getDataCount()
{
	XOJ_CHECK_TYPE(PdfWriter);

	return this->dataCount;
}

bool PdfWriter::writeTxt(string data)
{
	XOJ_CHECK_TYPE(PdfWriter);

	string str(data);
	StringUtils::replace_all_chars(str, {
								   replace_pair('\\', "\\\\"),
								   replace_pair('(', "\\("),
								   replace_pair(')', "\\)"),
								   replace_pair('\r', "\\\r")
	});
	str = "(" + str + ")";

	return writeLen(str, str.length());
}

bool PdfWriter::writeLen(string data, int len)
{
	XOJ_CHECK_TYPE(PdfWriter);

	if (this->inStream)
	{
		g_string_append_len(this->stream, data.c_str(), len);
		return true;
	}

	GError* err = NULL;

	g_output_stream_write(G_OUTPUT_STREAM(this->out), data.c_str(), len, NULL, &err);

	this->dataCount += len;

	if (err)
	{
		this->lastError = (bl::format("Error writing stream: {1}") % err->message).str();

		cout << bl::format("error writing file: {1}") % err->message << endl;
		g_error_free(err);
		return false;
	}

	return true;
}

bool PdfWriter::writeObj()
{
	XOJ_CHECK_TYPE(PdfWriter);

	this->xref->addXref(this->dataCount);
	bool res = this->writef("%i 0 obj\n", this->objectId++);
	if (!res)
	{
		this->lastError = "Internal PDF error #8";
	}

	return res;
}

bool PdfWriter::writeInfo(string title)
{
	XOJ_CHECK_TYPE(PdfWriter);

	if (!writeObj())
	{
		return false;
	}

	write("<<\n");

	write("/Producer ");
	writeTxt("Xournal++");
	write("\n");

	if (!title.empty())
	{
		write("/Title ");
		if (title.length() > 4 && title.substr(title.length() - 4, 1) == ".")
		{
			title = title.substr(0, title.length() - 4);
		}
		writeTxt(title);
		write("\n");
	}

	const char* username = getenv("USERNAME");
	if (username)
	{
		write("/Author ");
		writeTxt(username);
		write("\n");
	}

	write("/Creator ");
	writeTxt("Cairo / Poppler / Xournal++ v." VERSION);
	write("\n");

	time_t curtime = time(NULL);
	char stime[128] = "D:";
	strftime(stime + 2, sizeof (stime) - 2, "%Y%m%d%H%M%S", localtime(&curtime));

	write("/CreationDate ");
	writeTxt(stime);
	write("\n");

	write(">>\nendobj\n");

	return this->lastError.empty();
}

void PdfWriter::startStream()
{
	XOJ_CHECK_TYPE(PdfWriter);

	this->inStream = true;
}

void PdfWriter::endStream()
{
	XOJ_CHECK_TYPE(PdfWriter);

	this->inStream = false;

	GString* data = NULL;
	GString* compressed = NULL;

	if (PdfWriter::compressPdfOutput)
	{
		compressed = GzHelper::gzcompress(this->stream);
	}

	const char* filter = "";
	if (compressed)
	{
		filter = "/Filter /FlateDecode ";
		data = compressed;
	}
	else
	{
		data = this->stream;
	}

	writef("<<%s/Length %i>>\n", filter, data->len);
	write("\nstream\n");

	writeLen(data->str, data->len);

	write("\nendstream\n");

	if (compressed)
	{
		g_string_free(compressed, true);
	}

	this->stream->len = 0;
}
