/*
 * Xournal++
 *
 * Serialized input stream
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "InputStreamException.h"

#include <gtk/gtk.h>

class Serializeable;

class ObjectInputStream
{
public:
	ObjectInputStream();
	virtual ~ObjectInputStream();

public:
	bool read(const char* data, int len) throw (InputStreamException);

	void readObject(const char* name) throw (InputStreamException);
	string readObject() throw (InputStreamException);
	string getNextObjectName() throw (InputStreamException);
	void endObject() throw (InputStreamException);

	int readInt() throw (InputStreamException);
	double readDouble() throw (InputStreamException);
	string readString() throw (InputStreamException);

	void readData(void** data, int* len) throw (InputStreamException);
	cairo_surface_t* readImage() throw (InputStreamException);

	ObjectInputStream& operator>>(Serializeable* s) throw (InputStreamException);

private:
	void checkType(char type) throw (InputStreamException);

	string getType(char type);

private:
	XOJ_TYPE_ATTRIB;

	GString* str;
	gsize pos;
};
