#include "OutputStream.h"

#include "GzUtil.h"
#include <i18n.h>

#include <stdlib.h>

OutputStream::OutputStream() { }

OutputStream::~OutputStream() { }

void OutputStream::write(const string& str)
{
	write(str.c_str(), str.length());
}

void OutputStream::write(const char* str)
{
	write(str, strlen(str));
}

////////////////////////////////////////////////////////
/// GzOutputStream /////////////////////////////////////
////////////////////////////////////////////////////////

GzOutputStream::GzOutputStream(Path filename)
{
	XOJ_INIT_TYPE(GzOutputStream);

	this->fp = NULL;
	this->filename = filename;

	this->fp = GzUtil::openPath(filename, "w");
	if (this->fp == NULL)
	{
		this->error = FS(_F("Error opening file: \"{1}\"") % filename.str());
	}
}

GzOutputStream::~GzOutputStream()
{
	XOJ_CHECK_TYPE(GzOutputStream);

	if (this->fp)
	{
		close();
	}
	this->fp = NULL;

	XOJ_RELEASE_TYPE(GzOutputStream);
}

string& GzOutputStream::getLastError()
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
