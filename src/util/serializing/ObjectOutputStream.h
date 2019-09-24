/*
 * Xournal++
 *
 * Serialized output stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class ObjectEncoding;
class Serializeable;

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
	void writeSizeT(size_t st);
	void writeString(const char* str);
	void writeString(const string& s);

	void writeData(const void* data, int len, int width);
	void writeImage(cairo_surface_t* img);

	GString* getStr();

private:
	ObjectEncoding* encoder = nullptr;
};
