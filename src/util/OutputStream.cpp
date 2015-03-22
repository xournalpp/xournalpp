#include "OutputStream.h"
#include <stdlib.h>
#include <string.h>

OutputStream::OutputStream()
{
}

OutputStream::~OutputStream()
{
}

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

GzOutputStream::GzOutputStream(path filename)
{
    XOJ_INIT_TYPE(GzOutputStream);

    this->fp = NULL;
    this->filename = filename;
    this->fp = gzopen(filename.c_str(), "w");
    if (this->fp == NULL)
    {
        this->error = (bl::format("error opening file: \"{1}\"") % filename.string()).str();
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

