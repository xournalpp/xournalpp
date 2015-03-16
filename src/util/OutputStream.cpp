#include "OutputStream.h"
#include <stdlib.h>
#include <string.h>

OutputStream::OutputStream()
{
}

OutputStream::~OutputStream()
{
}

void OutputStream::write(const String& str)
{
	write(CSTR(str), str.length());
}

void OutputStream::write(const char* str)
{
	write(str, strlen(str));
}

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(String filename)
{
	XOJ_INIT_TYPE(GzOutputStream);

	this->fp = NULL;
	this->filename = filename;
	this->fp = gzopen(CSTR(filename), "w");
	if (this->fp == NULL)
	{
		char* e = g_strdup_printf("error opening file: \"%s\"", CSTR(filename));
		this->error = e;
		g_free(e);
	}
}

GzOutputStream::~GzOutputStream()
{
	XOJ_CHECK_TYPE(GzOutputStream);

	if (this->fp)
	{
		close();
	}

	XOJ_RELEASE_TYPE(GzOutputStream);
}

String& GzOutputStream::getLastError()
{
	XOJ_CHECK_TYPE(GzOutputStream);

	return this->error;
}

void GzOutputStream::write(const char* data, int len)
{
	XOJ_CHECK_TYPE(GzOutputStream);

	gzwrite(this->fp, data, len);
}

void GzOutputStream::close()
{
	XOJ_CHECK_TYPE(GzOutputStream);

	if (this->fp)
	{
		gzclose(this->fp);
		this->fp = NULL;
	}
}

