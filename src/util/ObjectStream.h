/*
 * Xournal++
 *
 * Object stream, this class is used to help serialize objects to a string
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __OBJECTSTREAM_H__
#define __OBJECTSTREAM_H__

#include <gtk/gtk.h>
#include "../util/String.h"
#include "../util/Serializeable.h"

class Serializeable;
class ObjectOutputStream {
public:
	ObjectOutputStream();
	virtual ~ObjectOutputStream();

public:
	void writeObject(const char * name);
	void endObject();

	void writeInt(int i);
	void writeDouble(double d);
	void writeString(const char * str);
	void writeString(const String & s);

	void writeData(const void * data, int len, int width);
	void writeImage(cairo_surface_t * img);

	ObjectOutputStream & operator <<(Serializeable * s);

	GString * getStr();
private:
	GString * data;
};

class ObjectInputStream {
public:
	ObjectInputStream();
	virtual ~ObjectInputStream();

public:
	bool read(const char * data, int len) throw (InputStreamException);

	void readObject(const char * name) throw (InputStreamException);
	String readObject() throw (InputStreamException);
	String getNextObjectName() throw (InputStreamException);
	void endObject() throw (InputStreamException);

	int readInt() throw (InputStreamException);
	double readDouble() throw (InputStreamException);
	String readString() throw (InputStreamException);

	void readData(void ** data, int * len) throw (InputStreamException);
	cairo_surface_t * readImage() throw (InputStreamException);

	ObjectInputStream & operator >>(Serializeable * s) throw (InputStreamException);

private:
	void checkType(char type) throw (InputStreamException);

	String getType(char type);

private:
	GString * str;
	int pos;
};

#endif /* __OBJECTSTREAM_H__ */
