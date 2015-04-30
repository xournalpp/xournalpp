#include "ObjectEncoding.h"

ObjectEncoding::ObjectEncoding()
{
	XOJ_INIT_TYPE(ObjectEncoding);

	this->data = g_string_new("");
}

ObjectEncoding::~ObjectEncoding()
{
	XOJ_RELEASE_TYPE(ObjectEncoding);
}

void ObjectEncoding::addStr(const char* str)
{
	XOJ_CHECK_TYPE(ObjectEncoding);

	g_string_append(this->data, str);
}

GString* ObjectEncoding::getData()
{
	XOJ_CHECK_TYPE(ObjectEncoding);

	GString* str = this->data;
	this->data = NULL;
	return str;
}
