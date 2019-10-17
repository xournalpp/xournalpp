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
	this->filename = filename;

	this->fp = GzUtil::openPath(filename, "w");
	if (this->fp == nullptr)
	{
		this->error = FS(_F("Error opening file: \"{1}\"") % filename.str());
	}
}

GzOutputStream::~GzOutputStream()
{
	if (this->fp)
	{
		close();
	}
	this->fp = nullptr;
}

string& GzOutputStream::getLastError()
{
	return this->error;
}

void GzOutputStream::write(const char* data, int len)
{
	gzwrite(this->fp, data, len);
}

void GzOutputStream::close()
{
	if (this->fp)
	{
		gzclose(this->fp);
		this->fp = nullptr;
	}
}
