#include "BinObjectEncoding.h"

BinObjectEncoding::BinObjectEncoding()
{
	XOJ_INIT_TYPE(BinObjectEncoding);
}

BinObjectEncoding::~BinObjectEncoding()
{
	XOJ_RELEASE_TYPE(BinObjectEncoding);
}

void BinObjectEncoding::addData(const void* data, int len)
{
	XOJ_CHECK_TYPE(ObjectEncoding);

	g_string_append_len(this->data, (const char*)data, len);
}
