#include "ObjectOutputStream.h"
#include "Serializeable.h"
#include "ObjectEncoding.h"

ObjectOutputStream::ObjectOutputStream(ObjectEncoding* encoder)
{
	XOJ_INIT_TYPE(ObjectOutputStream);

	g_assert(encoder != NULL);
	this->encoder = encoder;

	writeString(XML_VERSION_STR);
}

ObjectOutputStream::~ObjectOutputStream()
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	delete this->encoder;
	this->encoder = NULL;

	XOJ_RELEASE_TYPE(ObjectOutputStream);
}

ObjectOutputStream& ObjectOutputStream::operator<<(Serializeable* s)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	s->serialize(*this);
	return *this;
}

void ObjectOutputStream::writeObject(const char* name)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_{");

	writeString(name);
}

void ObjectOutputStream::endObject()
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_}");
}

void ObjectOutputStream::writeInt(int i)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_i");
	this->encoder->addData(&i, sizeof int);
}

void ObjectOutputStream::writeDouble(double d)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_d");
	this->encoder->addData(&d, sizeof double);
}

void ObjectOutputStream::writeString(const char* str)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	writeString(string(str));
}

void ObjectOutputStream::writeString(const string& s)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_s");
	int len = s.length();
	this->encoder->addData(&len, sizeof int);
	this->encoder->addData(s.c_str(), len);
}

void ObjectOutputStream::writeData(const void* data, int len, int width)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	this->encoder->addStr("_b");
	this->encoder->addData(&len, sizeof int);

	// size of one element
	this->encoder->addData(&width, sizeof int);
	if (data != NULL)
	{
		this->encoder->addData(data, len * width);
	}
}

static cairo_status_t cairoWriteFunction(GString* string,
										 const unsigned char* data, unsigned int length)
{
	g_string_append_len(string, (const gchar*) data, length);
	return CAIRO_STATUS_SUCCESS;
}

void ObjectOutputStream::writeImage(cairo_surface_t* img)
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	GString* imgStr = g_string_sized_new(102400);

	cairo_surface_write_to_png_stream(img, (cairo_write_func_t) &cairoWriteFunction,
									  imgStr);

	this->encoder->addStr("_m");
	this->encoder->addData(&imgStr->len, sizeof gsize);

	this->encoder->addData(imgStr->str, imgStr->len);

	g_string_free(imgStr, true);
}

GString* ObjectOutputStream::getStr()
{
	XOJ_CHECK_TYPE(ObjectOutputStream);

	return this->encoder->getData();
}
