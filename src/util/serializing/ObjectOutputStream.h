/*
 * Xournal++
 *
 * Serialized output stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <StringUtils.h>
#include <gtk/gtk.h>

class Serializeable;
class ObjectEncoding;

class ObjectOutputStream
{
public:
	ObjectOutputStream(ObjectEncoding* encoder);
	virtual ~ObjectOutputStream();

public:
	void writeObject(const char* name);
	void endObject();

	void writeInt(int i);
	void writeDouble(double d);
	void writeString(const char* str);
	void writeString(const string& s);

	void writeData(const void* data, int len, int width);
	void writeImage(cairo_surface_t* img);

	ObjectOutputStream& operator<<(Serializeable* s);

	GString* getStr();

private:
	XOJ_TYPE_ATTRIB;

	ObjectEncoding* encoder;
};
