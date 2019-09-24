#include "ObjectEncoding.h"

ObjectEncoding::ObjectEncoding()
{
	this->data = g_string_new("");
}

ObjectEncoding::~ObjectEncoding()
{
}

void ObjectEncoding::addStr(const char* str)
{
	g_string_append(this->data, str);
}

GString* ObjectEncoding::getData()
{
	GString* str = this->data;
	this->data = nullptr;
	return str;
}
